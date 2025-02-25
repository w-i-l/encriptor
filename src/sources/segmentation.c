#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../headers/segmentation.h"


void print_segment_info(process_segment_info info) {
    printf("Segment info:\n");
    printf("Number of words: %d\n", info.no_of_words);
    printf("Number of characters: %d\n", info.no_of_chars);
    printf("Begin offset: %elu\n", info.begin_offset);
    printf("End offset: %lu\n", info.end_offset);
    printf("Segment length: %lu\n", info.segment_length);
}


process_segment_info *get_segments_for_file(const char *filename, int no_of_processes)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file for segmentation");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate segments with proper initialization
    process_segment_info *segments = (process_segment_info *)calloc(no_of_processes, sizeof(process_segment_info));
    if (segments == NULL)
    {
        perror("Error allocating memory for segments");
        fclose(file);
        return NULL;
    }

    // Calculate approximate segment size
    size_t approx_segment_size = file_size / no_of_processes;
    printf("File size: %zu, Approximate segment size: %zu\n", file_size, approx_segment_size);

    // Process each segment
    size_t current_offset = 0;
    for (int i = 0; i < no_of_processes; i++)
    {
        segments[i].begin_offset = current_offset;

        // Determine where this segment should approximately end
        size_t target_end = (i == no_of_processes - 1) ? file_size : // Last segment goes to end of file
                                segments[i].begin_offset + approx_segment_size;

        // Find a word boundary near the target end
        // First seek to target position
        fseek(file, target_end, SEEK_SET);

        // If not at end of file, find the next word boundary
        if (target_end < file_size)
        {
            // Search forward for next complete word boundary (space or newline)
            int c;
            while ((c = fgetc(file)) != EOF && c != ' ' && c != '\n' && c != '\t')
            {
                target_end++;
            }

            // Include the boundary character in this segment
            if (c != EOF)
            {
                target_end++;
            }
        }

        segments[i].end_offset = target_end;
        segments[i].segment_length = segments[i].end_offset - segments[i].begin_offset;

        // Now count words and characters in this segment
        fseek(file, segments[i].begin_offset, SEEK_SET);

        size_t word_count = 0;
        size_t char_count = 0;
        int in_word = 0;

        for (size_t pos = segments[i].begin_offset; pos < segments[i].end_offset; pos++)
        {
            int c = fgetc(file);
            if (c == EOF)
                break;

            char_count++;

            // Word boundary detection
            if (c == ' ' || c == '\n' || c == '\t')
            {
                if (in_word)
                {
                    word_count++;
                    in_word = 0;
                }
            }
            else
            {
                in_word = 1;
            }
        }

        // Count the last word if we were in one
        if (in_word)
        {
            word_count++;
        }

        segments[i].no_of_words = word_count;
        segments[i].no_of_chars = char_count;

        printf("Segment %d: begin=%zu, end=%zu, length=%zu, words=%d, chars=%d\n",
               i, segments[i].begin_offset, segments[i].end_offset,
               segments[i].segment_length, segments[i].no_of_words, segments[i].no_of_chars);

        // Prepare for next segment
        current_offset = segments[i].end_offset;
    }

    // Verify segments are created correctly
    for (int i = 0; i < no_of_processes - 1; i++)
    {
        if (segments[i].end_offset != segments[i + 1].begin_offset)
        {
            fprintf(stderr, "Warning: Segment boundary mismatch between segments %d and %d\n", i, i + 1);
            fprintf(stderr, "Segment %d ends at %zu but segment %d begins at %zu\n",
                    i, segments[i].end_offset, i + 1, segments[i + 1].begin_offset);
        }
    }

    fclose(file);
    return segments;
}


void copy_file_segment(
    const char* input_filename, 
    const char* output_filename, 
    process_segment_info* segments, 
    int no_of_processes
) {
    FILE* input = fopen(input_filename, "r");
    if(input == NULL) {
        perror("Error opening input file");
        return;
    }

    FILE* output = fopen(output_filename, "w");
    if(output == NULL) {
        perror("Error opening output file");
        return;
    }

    fseek(input, 0, SEEK_SET);
    char c;
    for(int segment_index = 0; segment_index < no_of_processes; segment_index++) {
        fseek(input, segments[segment_index].begin_offset, SEEK_SET);
        for(size_t i = 0; i < segments[segment_index].no_of_words; i++) {
            c = fgetc(input);
            while(c != EOF && c != ' ' && c != '\n') {
                fputc(c, output);
                c = fgetc(input);
            }

            if (c == EOF) {
                break;
            }
            fputc(c, output);
        }
    }

    fclose(input);
    fclose(output);
}


bool is_file_segmented_correctly(
    const char* filename, 
    process_segment_info* segments, 
    int no_of_processes
) {
    FILE* file = fopen(filename, "r");
    if(file == NULL) {
        perror("Error opening file");
        return false;
    }

    // skip first segment
    for(int i = 1; i < no_of_processes; i++) {
        fseek(file, segments[i].begin_offset - 1, SEEK_SET);
        char space, next_char;
        fscanf(file, "%c %c", &space, &next_char);
        if(space != ' ' || next_char == ' ') {
            printf("Segment %d is not correct\n", i);
            return false;
        }
    }

    fclose(file);
    return true;
}