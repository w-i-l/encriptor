#include "../headers/coder_multiprocess.h"
#include "../headers/random_permutation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

size_t round_to_page_size(size_t size) {
    size_t page_size = getpagesize();
    printf("Page size: %ld\n", page_size);
    return ROUND_UP(size, page_size);
}

int encode_multiprocess(const char* input_file, const char* output_file, process_segment_info* segments, int no_of_processes) {
    FILE* input = fopen(input_file, "r");
    if(input == NULL) {
        perror("Error opening input file");
        return -1;
    }

    FILE* output = fopen(output_file, "w");
    if(output == NULL) {
        perror("Error opening output file");
        fclose(input);
        return -1;
    }

    // create shared memory
    int fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    if(fd == -1) {
        perror("Error creating shared memory");
        fclose(input);
        fclose(output);
        return -1;
    }

    size_t total_words = 0;
    size_t total_lenght = 0;
    for(int i = 0; i < no_of_processes; i++) {
        total_words += segments[i].no_of_words;
        size_t size = segments[i].no_of_words * sizeof(permutation);
        size = round_to_page_size(size);
        total_lenght += size;
    }
    printf("Total words: %ld\n", total_words);

    size_t shared_memory_size = round_to_page_size(total_lenght);

    if(ftruncate(fd, shared_memory_size) == -1) {
        perror("Error truncating shared memory");
        fclose(input);
        fclose(output);
        shm_unlink("/shared_memory");
        return -1;
    }

    printf("Shared memory size: %ld\n", shared_memory_size);

    // flush the output before forking
    fflush(stdout);

    for(int process_no = 0; process_no < no_of_processes; process_no++) {
        int pid = fork();
        if(pid == -1) {
            perror("Error forking");
            return -1;
        }

        if(pid == 0) {
            // printf("Child process %d\n", process_no);

            size_t words_offset = 0;
            for(int i = 0; i < process_no; i++) {
                words_offset += segments[i].no_of_words;
            }
            words_offset = words_offset * sizeof(permutation);
            words_offset = round_to_page_size(words_offset);

            size_t words_to_read = segments[process_no].no_of_words * sizeof(permutation);
            words_to_read = round_to_page_size(words_to_read);

            printf("words to read: %d words offset: %d\n", words_to_read, words_offset);

            permutation* shared_memory = (permutation*) mmap(
                NULL, 
                words_to_read, 
                PROT_READ | PROT_WRITE, 
                MAP_SHARED, 
                fd, 
                words_offset
            );

            if(shared_memory == MAP_FAILED) {
                perror("Error mapping shared memory");
                perror(strerror(errno));
                printf("words_to_read: %d words_offset: %d\n", words_to_read, words_offset);
                printf("[ERROR] Process %d\n", process_no);
                return -1;
            }

            char buffer[1024];
            int index = 0;

            fseek(input, segments[process_no].begin_offset, SEEK_SET);
            while(fscanf(input, "%1023s", buffer) != EOF && ftell(input) < segments[process_no].end_offset) {
                permutation perm = random_permutation(buffer);
                shared_memory[index] = perm;
                index++;
            }

            print_permutation(shared_memory[0]);
            munmap(shared_memory, words_to_read);
            exit(0);
        } else {
            printf("Parent process %d\n", process_no);
        }
    }

    for (int i = 0; i < no_of_processes; i++) {
        wait(NULL);
    }

    printf("Back to Parent process\n");

    permutation* shared_memory = (permutation*) mmap(
        NULL, 
        shared_memory_size, 
        PROT_READ, 
        MAP_SHARED, 
        fd, 
        0
    );

    if(shared_memory == MAP_FAILED) {
        perror("Error mapping shared memory");
        perror(strerror(errno));
        return -1;
    }

    printf("Printing shared memory\n");
    print_permutation(shared_memory[0]);
    // for(int i = 0; i < 10; i++) {
    //     print_permutation(shared_memory[i]);
    // }

    munmap(shared_memory, shared_memory_size);


    fclose(input);
    fclose(output);

    return fd;
}
