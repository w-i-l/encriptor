#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include "headers/random_permutation.h"
#include "headers/coder.h"
#include "headers/coder_multiprocess.h"

#define MAX_PROCESS 10
#define ADDITIONAL_SAFETY_SIZE 100
#define PERMUTATION_FILE_SCALE_FACTOR 3
#define BUFFER_SIZE 5000

size_t round_to_page_size(size_t size) {
    size_t page_size = getpagesize();
    return (size + page_size - 1) & ~(page_size - 1);
}

void print_process_segment_info(process_segment_info* segments) {
    for(int i = 0; i < MAX_PROCESS; i++) {
        printf("Segment %d:\n", i);
        printf("No of words: %d\n", segments[i].no_of_words);
        printf("No of chars: %d\n", segments[i].no_of_chars);
        printf("Begin offset: %ld\n", segments[i].begin_offset);
        printf("Segment length: %ld\n", segments[i].segment_length);
        printf("\n");
    }
}

process_segment_info* initialise_processes_segments(const char* input_file) {
    
    FILE* input = fopen(input_file, "r");
    if(input == NULL) {
        printf("Error opening input file\n");
        printf("Error code: %s\n", strerror(errno));
        return NULL;
    }

    process_segment_info* segments = (process_segment_info*) malloc(MAX_PROCESS * sizeof(process_segment_info));
    if(segments == NULL) {
        printf("Error allocating memory for process segments\n");
        printf("Error code: %s\n", strerror(errno));
        return NULL;
    }
    
    struct stat st;
    stat(input_file, &st);
    size_t file_size = st.st_size;

    size_t process_segment_aproximate_length = (file_size + MAX_PROCESS - 1) / MAX_PROCESS;
    char buffer[BUFFER_SIZE] = {0};

    int current_segment_index = 0;

    size_t current_segment_size = 0;
    size_t current_segment_no_of_words = 0;
    size_t current_segment_no_of_chars = 0;
    size_t current_segment_begin_offset = 0;

    printf("Begins to read the file\n");

    do {
        memset(buffer, 0, BUFFER_SIZE);
        fscanf(input, "%s\n", buffer);

        size_t buffer_size = 0;
        for(; buffer[buffer_size] != '\0'; buffer_size++);
        buffer_size++;

        if(current_segment_size + buffer_size > process_segment_aproximate_length) {
            segments[current_segment_index].no_of_words = current_segment_no_of_words;
            segments[current_segment_index].no_of_chars = current_segment_no_of_chars;
            segments[current_segment_index].begin_offset = current_segment_begin_offset;
            segments[current_segment_index].segment_length = current_segment_size;
            segments[current_segment_index].permutations = (permutation*) malloc(current_segment_no_of_words * sizeof(permutation));

            current_segment_index++;
            current_segment_begin_offset += current_segment_size;
            current_segment_size = buffer_size;
            current_segment_no_of_words = 1;
            current_segment_no_of_chars = buffer_size;
        } else {
            current_segment_size += buffer_size;
            current_segment_no_of_words++;
            current_segment_no_of_chars += buffer_size;
        }
    } while(!feof(input));

    printf("Remaining segment size: %ld\n", current_segment_size);
    printf("Remaining segment no of words: %ld\n", current_segment_no_of_words);
    printf("Remaining segment no of chars: %ld\n", current_segment_no_of_chars);

    current_segment_index = MAX_PROCESS - 1;
    // add to the last segment
    segments[current_segment_index].no_of_words += current_segment_no_of_words;
    segments[current_segment_index].no_of_chars += current_segment_no_of_chars;
    segments[current_segment_index].segment_length += current_segment_size;

    fclose(input);

    return segments;
}

char* get_mapped_file(const char* file) {

    int input_fd = open(file, O_RDONLY);
    if(input_fd == -1) {
        printf("Error opening input file\n");
        printf("Error code: %s\n", strerror(errno));
        return NULL;
    }

    struct stat st;
    stat(file, &st);
    size_t file_size = st.st_size;

    size_t page_size = getpagesize();
    size_t mapped_file_size = round_to_page_size(file_size + ADDITIONAL_SAFETY_SIZE);
    char* mapped_file = (char*) mmap(
        NULL, 
        mapped_file_size,
        PROT_READ,
        MAP_SHARED,
        input_fd,
        0
    );
    if(mapped_file == MAP_FAILED) {
        printf("Error mapping file\n");
        printf("Error code: %s\n", strerror(errno));
        return NULL;
    }

    close(input_fd);
    return mapped_file;
}

void encode_multiprocess(const char* input_file, const char* output_file, const char* permutation_file) {
    
    process_segment_info* segments = initialise_processes_segments(input_file);
    if(segments == NULL) {
        printf("Error initialising process segments\n");
        return;
    }
    
    char* mapped_file = get_mapped_file(input_file);
    if(mapped_file == NULL) {
        printf("Error getting mapped file\n");
        return;
    }

    pid_t pids[MAX_PROCESS];
    for(int i = 0; i < MAX_PROCESS; i++) {
        
        int pid = fork();
        if(pid == -1) {
            printf("Error forking process\n");
            printf("Error code: %s\n", strerror(errno));
            return;
        }
        pids[i] = pid;

        if(pid == 0) {

            printf("Child process %d with PID: %d has started.\n", i, getpid());

            char* segment = mapped_file + segments[i].begin_offset;

            for(int j = 0;j < segments[i].no_of_words;j++) {
                permutation aux_permutation;

                size_t size = 0;
                for(; segment[size] != ' '; size++);
                size++;

                aux_permutation = random_permutation(segment, size);

                segments[i].permutations[j] = aux_permutation;
                print_permutation(segments[i].permutations[j]);
                segment += size;
            }
            _exit(0);
        }
    }

    print_process_segment_info(segments);

    FILE* encoded = fopen(output_file, "w");
    FILE* permutations = fopen(permutation_file, "w");

    for(int i = 0; i < MAX_PROCESS; i++) {
        pid_t pid = wait(NULL);

        // free the memory for the specific process
        for(int j = 0; j < MAX_PROCESS; j++) {
            if(pids[j] == pid) {

                printf("Starting to write for process %d with PID: %d\n", i, pid);

                for(int k = 0; k < segments[j].no_of_words; k++) {
                    printf("Writing permutation %d for process %d with PID: %d - word: %s\n", k, i, pid, segments[j].permutations[k].char_permutation);
                    if(fprintf(encoded, "%s ", segments[j].permutations[k].char_permutation) == 0) {
                        printf("Error writing to file\n");
                        return;
                    }
                }


                for(int k = 0; k < segments[j].no_of_words; k++) {
                    free_permutation(segments[j].permutations[k]);
                }
                free(segments[j].permutations);
            }
        }
        printf("Freed memory for process %d\n", i);

        printf("Child process %d with PID: %d has finished.\n\n", i, pid);
    }

    fclose(encoded);
    fclose(permutations);
}