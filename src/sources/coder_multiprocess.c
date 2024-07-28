#include "../headers/coder_multiprocess.h"
#include "../headers/random_permutation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

void write_permutations_to_file(const char* permutations_filename, const char* encoded_filename, permutation* permutations, size_t length);
char* encoding_permutation_to_string(permutation p);

int isspace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

typedef struct {
    size_t total_words;
    permutation permutations[];
} shared_data;

int encode_multiprocess(const char* input_file, const char* permutations_filename, const char* encoded_filename, process_segment_info* segments, int no_of_processes) {
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        perror("Error opening input file");
        return -1;
    }

    struct stat sb;
    if (fstat(input_fd, &sb) == -1) {
        perror("Error getting file size");
        close(input_fd);
        return -1;
    }

    char* input_map = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, input_fd, 0);
    if (input_map == MAP_FAILED) {
        perror("Error mapping input file");
        close(input_fd);
        return -1;
    }

    FILE* output = fopen(output_file, "w");
    if (output == NULL) {
        perror("Error opening output file");
        munmap(input_map, sb.st_size);
        close(input_fd);
        return -1;
    }

    size_t total_words = 0;
    for (int i = 0; i < no_of_processes; i++) {
        total_words += segments[i].no_of_words;
    }

    size_t shm_size = sizeof(shared_data) + total_words * sizeof(permutation);

    int fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error creating shared memory");
        fclose(output);
        munmap(input_map, sb.st_size);
        close(input_fd);
        return -1;
    }

    if (ftruncate(fd, shm_size) == -1) {
        perror("Error truncating shared memory");
        fclose(output);
        munmap(input_map, sb.st_size);
        close(input_fd);
        shm_unlink("/shared_memory");
        return -1;
    }

    shared_data* shared_memory = (shared_data*)mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Error mapping shared memory");
        fclose(output);
        munmap(input_map, sb.st_size);
        close(input_fd);
        shm_unlink("/shared_memory");
        return -1;
    }

    shared_memory->total_words = total_words;

    fflush(stdout);
    for (int process_no = 0; process_no < no_of_processes; process_no++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error forking");
            return -1;
        }
        if (pid == 0) {
            // Child process
            shared_data* child_shared_memory = (shared_data*)mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (child_shared_memory == MAP_FAILED) {
                perror("Child: Error mapping shared memory");
                exit(1);
            }

            size_t start_index = 0;
            for (int i = 0; i < process_no; i++) {
                start_index += segments[i].no_of_words;
            }

            char* segment_start = input_map + segments[process_no].begin_offset;
            char* segment_end = input_map + segments[process_no].end_offset;
            char* current = segment_start;

            printf("Child %d: Starting at offset %ld\n", process_no, segments[process_no].begin_offset);

            char buffer[1024];
            size_t index = 0;
            size_t buffer_index = 0;

            while (current < segment_end && index < segments[process_no].no_of_words) {
                char c = *current++;
                if (isspace(c) || c == '\n') {
                    if (buffer_index > 0) {
                        buffer[buffer_index] = '\0';
                        child_shared_memory->permutations[start_index + index] = random_permutation(buffer);
                        // printf("Child %d: Read word %zu: '%s'\n", process_no, index, buffer);
                        index++;
                        buffer_index = 0;
                    }
                } else {
                    if (buffer_index < 1023) {
                        buffer[buffer_index++] = c;
                    }
                }
            }

            // Handle last word if buffer is not empty
            if (buffer_index > 0) {
                buffer[buffer_index] = '\0';
                child_shared_memory->permutations[start_index + index] = random_permutation(buffer);
                printf("Child %d: Read word %zu: '%s'\n", process_no, index, buffer);
                index++;
            }

            printf("Child %d: Finished at offset %ld, read %zu words\n", process_no, current - input_map, index);

            munmap(child_shared_memory, shm_size);
            exit(0);
        }
    }

    for (int i = 0; i < no_of_processes; i++) {
        wait(NULL);
    }

    printf("Back to Parent process\n");
    printf("Writing output\n");

    write_permutations_to_file(output_file, "files/encoded.txt", shared_memory->permutations, shared_memory->total_words);

    // for (size_t i = 0; i < shared_memory->total_words - 1; i++) {
    //     char* decoded = decode_permutation(shared_memory->permutations[i]);
    //     printf("Decoded word %zu: '%s'\n", i, decoded);
    //     if(decoded[0] != '\0')
    //         fprintf(output, "%s ", decoded);
    //     free(decoded);
    // }

    // char* decoded = decode_permutation(shared_memory->permutations[shared_memory->total_words - 1]);
    // printf("Decoded word %zu: '%s'\n", shared_memory->total_words - 1, decoded);
    // if(decoded[0] != '\0')
    //     fprintf(output, "%s", decoded);
    // free(decoded);

    munmap(shared_memory, shm_size);
    munmap(input_map, sb.st_size);
    close(input_fd);
    fclose(output);
    shm_unlink("/shared_memory");

    return 0;
}

void write_permutations_to_file(const char* permutations_filename, const char* encoded_filename, permutation* permutations, size_t length) {
    FILE* permutations_file = fopen(permutations_filename, "w");
    if (permutations_file == NULL) {
        perror("Error opening file");
        return;
    }

    FILE* encoded_file = fopen(encoded_filename, "w");
    if (encoded_file == NULL) {
        perror("Error opening file");
        return;
    }

    for(size_t i = 0; i < length; i++) {
        char* encoding = encoding_permutation_to_string(permutations[i]);
        printf("Encoding: %s\n", encoding);
        fprintf(permutations_file, "%s\n", encoding);
        fprintf(encoded_file, "%s\n", permutations[i].char_permutation);
        free(encoding);
    }

    fclose(permutations_file);
    fclose(encoded_file);
}

char* encoding_permutation_to_string(permutation p) {
    // 0-9 - 10 characters + 9 spaces + 1 to link with next
    // 10-99 - 2 * 90 = 180 characters + 90 spaces + 1 to link with next
    // 100-500 - 3 * 401 = 1203 characters + 400 spaces
    // Total: 10 + 180 + 1203 = 1393 characters + 499 spaces = 1892 characters
    const int MAX_LENGTH = MAX_PERMUTATION_LENGTH * 4;
    char* result = (char*) malloc(MAX_LENGTH * sizeof(char));

    int index = 0;
    for (size_t i = 0; i < p.length; i++) {
        int n = p.int_permutation[i];
        if (n < 10) {
            result[index++] = n + '0';
            result[index++] = '-';
        } else if (n < 100) {
            result[index++] = n / 10 + '0';
            result[index++] = n % 10 + '0';
            result[index++] = '-';
        } else {
            result[index++] = n / 100 + '0';
            result[index++] = (n / 10) % 10 + '0';
            result[index++] = n % 10 + '0';
            result[index++] = '-';
        }
    }

    result[index - 1] = '\0';
    return result;
}