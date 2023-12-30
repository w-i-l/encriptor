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

#define MAX_PROCESS 6
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

void encode_multiprocess(const char* input_file, const char* output_file) {
    
    process_segment_info* segments = initialise_processes_segments(input_file);
    if(segments == NULL) {
        printf("Error initialising process segments\n");
        return;
    }

    size_t total_size = 0;
        for(int j = 0; j < MAX_PROCESS; j++) {
            total_size += segments[j].no_of_words;
    }
    total_size += ADDITIONAL_SAFETY_SIZE;
    printf("Total size: %ld\n", total_size);
    permutation* shared_permutations = (permutation*) mmap(
        NULL,
        total_size * sizeof(permutation),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        -1,
        0
    );
    if(shared_permutations == MAP_FAILED) {
        printf("Error mapping shared memory\n");
        printf("Error code: %s\n", strerror(errno));
        return;
    }
    
    char* mapped_file = get_mapped_file(input_file);
    if(mapped_file == NULL) {
        printf("Error getting mapped file\n");
        free(segments);
        return;
    }

    // print_process_segment_info(segments);

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

            size_t offset = 0;
            for(int j = 0; j < i; j++) {
                offset += segments[j].no_of_words;
            }

            printf("Child process %d with PID: %d has started.\n", i, getpid());

            char* segment = mapped_file + segments[i].begin_offset;

            // printf("The limit offset for the curremt process is: %ld\n", offset + segments[i].no_of_words);
            for(int j = 0; j < segments[i].no_of_words; j++) {
                permutation aux_permutation;

                size_t size = 0;
                for(; segment[size] != ' '; size++);
                size++;

                aux_permutation = random_permutation(segment, size);

                // if(printf("Process %d with PID: %d has generated permutation with offset: %ld\n", i, getpid(), offset + j) < 0) {
                //     printf("Error printing to stdout\n");
                //     _exit(0);
                // }

                if(offset + j >= total_size) {
                    printf("Error offset is bigger than total size\n");
                    _exit(0);
                }
                shared_permutations[offset + j] = aux_permutation;
                // print_permutation(shared_permutations[offset + j]);

                segment += size;
                if((size_t)(segment - mapped_file) > segments[i].segment_length + segments[i].begin_offset) {
                    printf("Error segment is bigger than segment length\n");
                    _exit(0);
                }
            }
            _exit(0);
        }
    }
    printf("Back to parent process\n");

    for(int i = 0; i < MAX_PROCESS; i++) {
        pid_t pid = wait(NULL);
        if(pid == -1) {
            printf("Error waiting for child process\n");
            printf("Error code: %s\n", strerror(errno));
            return;
        }

        printf("Child process %d with PID: %d has finished.\n\n", i, pid);
    }

    FILE* encoded = fopen(output_file, "w");
    for(size_t i = 0; i < total_size; i++) {
        if(fprintf(encoded, "%s ", shared_permutations[i].char_permutation) == 0) {
            printf("Error writing to file\n");
            return;
        }
    }

    fclose(encoded);
    munmap(mapped_file, total_size);
    munmap(shared_permutations, total_size * sizeof(permutation));
    free(segments);
}
