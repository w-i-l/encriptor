#include "../headers/segmentation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void print_segment_info(process_segment_info info) {
    printf("Segment info:\n");
    printf("Number of words: %d\n", info.no_of_words);
    printf("Number of characters: %d\n", info.no_of_chars);
    printf("Begin offset: %lu\n", info.begin_offset);
    printf("End offset: %lu\n", info.end_offset);
    printf("Segment length: %lu\n", info.segment_length);
}

process_segment_info* get_segments_for_file(const char* filenmae, int no_of_processes) {
    FILE* file = fopen(filenmae, "r");
    if(file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    process_segment_info* segments = (process_segment_info*) malloc(no_of_processes * sizeof(process_segment_info));
    if(segments == NULL) {
        perror("Error allocating memory for segments");
        return NULL;
    }

    size_t max_segment_size = file_size / no_of_processes;
    size_t current_offset = 0;

    char buffer[1024] = {0};

    for(int i = 0; i < no_of_processes; i++) {
        segments[i].begin_offset = current_offset;
        segments[i].no_of_words = 0;
        segments[i].no_of_chars = 0;

        if(buffer[0] != '\0') {
            segments[i].begin_offset -= strlen(buffer);
            segments[i].no_of_words++;
            segments[i].no_of_chars += strlen(buffer);
        }

        while(fscanf(file, "%1023s", buffer) == 1) {
            size_t word_size = strlen(buffer);
            // printf("Word size: %ld\n", word_size);

            size_t current_chars = segments[i].no_of_chars + segments[i].no_of_words + word_size;
            if(i != no_of_processes - 1 && current_chars > max_segment_size) {
                segments[i].end_offset = ftell(file) - word_size;
                break;
            } else if(ftell(file) == max_segment_size) {
                buffer[0] = '\0';
                segments->end_offset = ftell(file);
            } else if(i == no_of_processes - 1 && feof(file)) {
                segments[i].end_offset = ftell(file);
            }

            segments[i].no_of_words++;
            segments[i].no_of_chars += word_size;

        }
        
        segments[i].segment_length = segments[i].no_of_chars + segments[i].no_of_words;
        current_offset = ftell(file);
    }

    fclose(file);
    return segments;
}

void copy_file_segment(const char* input_filename, const char* output_filename, process_segment_info* segments, int no_of_processes) {
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

bool is_file_segmented_correctly(const char* filename, process_segment_info* segments, int no_of_processes) {
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