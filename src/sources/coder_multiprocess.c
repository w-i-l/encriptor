#include "../headers/coder_multiprocess.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../headers/random_permutation.h"

extern bool IS_DEBUG_ENABLED;

void write_permutations_to_file(
    const char *permutations_filename,
    const char *encoded_filename,
    permutation *permutations, size_t length
);
char *encoding_permutation_to_string(permutation p);

int is_space(char c) {
    return c == ' ' || 
            c == '\t' ||
            c == '\n' ||
            c == '\r' ||
            c == '\v' ||
            c == '\f';
}

typedef struct {
    size_t total_words;
    permutation permutations[];
} shared_data;

size_t get_size_of_permutation(size_t length) {
    size_t size = 0;
    for (size_t i = 0; i < length; i++) {
        int n = i;

        if (n < 10) {
            size += 2;
        } else if (n < 100) {
            size += 3;
        } else {
            size += 4;
        }
    }

    return size;
}

size_t get_permutation_segment_size(
    const char *encoded_filename,
    process_segment_info *segments,
    int segment
) {
    FILE *encoded_file = fopen(encoded_filename, "r");
    if (encoded_file == NULL) {
        perror("Error opening file");
        return -1;
    }

    if (segment < 0) {
        return 0;
    }

    size_t segment_size = 0;
    fseek(encoded_file, segments[segment].begin_offset, SEEK_SET);

    char c;
    size_t word_size = 0;
    while ((size_t)ftell(encoded_file) < segments[segment].end_offset) {
        c = fgetc(encoded_file);
        if (is_space(c) || c == '\n') {
            if (word_size > 0) {
                segment_size += get_size_of_permutation(word_size) + 1;    // +1 for space
                word_size = 0;
            }
        } else {
            word_size++;
        }
    }

    if (word_size > 0) {
        segment_size += get_size_of_permutation(word_size);
    }

    fclose(encoded_file);
    return segment_size;
}

int encode_multiprocess(
    const char *input_file,
    const char *permutations_filename,
    const char *encoded_filename,
    process_segment_info *segments,
    int no_of_processes
) {
    // Open input file
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        perror("Error opening input file");
        return -1;
    }

    // Get file size
    struct stat sb;
    if (fstat(input_fd, &sb) == -1) {
        perror("Error getting file size");
        close(input_fd);
        return -1;
    }

    // Map file to memory
    char *input_map = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, input_fd, 0);
    if (input_map == MAP_FAILED) {
        perror("Error mapping input file");
        close(input_fd);
        return -1;
    }

    // Count total words across all segments
    size_t total_words = 0;
    for (int i = 0; i < no_of_processes; i++) {
        total_words += segments[i].no_of_words;
    }

    if (IS_DEBUG_ENABLED) {
        printf("Total words to process: %zu\n", total_words);
    }

    // Create shared memory for permutations
    size_t shm_size =
            sizeof(shared_data) + (total_words + 100) * sizeof(permutation);
    int fd = open("/tmp/encoded_file", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Error creating shared memory");
        munmap(input_map, sb.st_size);
        close(input_fd);
        return -1;
    }

    // Truncate file to required size
    if (ftruncate(fd, shm_size) == -1) {
        perror("Error truncating shared memory");
        munmap(input_map, sb.st_size);
        close(input_fd);
        close(fd);
        return -1;
    }

    // Map shared memory
    shared_data *shared_memory = (shared_data *)mmap(
        NULL,
        shm_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED, 
        fd,
        0
    );

    if (shared_memory == MAP_FAILED) {
        perror("Error mapping shared memory");
        munmap(input_map, sb.st_size);
        close(input_fd);
        close(fd);
        return -1;
    }

    // Initialize shared memory
    shared_memory->total_words = total_words;
    memset(shared_memory->permutations, 0, total_words * sizeof(permutation));

    // Track child processes
    pid_t child_pids[no_of_processes];

    // Fork child processes
    for (int process_no = 0; process_no < no_of_processes; process_no++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error forking");

            // Kill any already-started children
            for (int j = 0; j < process_no; j++) {
                kill(child_pids[j], SIGTERM);
            }

            munmap(shared_memory, shm_size);
            munmap(input_map, sb.st_size);
            close(input_fd);
            close(fd);
            return -1;
        }

        if (pid == 0) {
            // Child process
            shared_data *child_shared_memory = (shared_data *)mmap(
                    NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (child_shared_memory == MAP_FAILED) {
                perror("Child: Error mapping shared memory");
                exit(1);
            }

            // Calculate starting index for this segment in the permutations array
            size_t start_index = 0;
            for (int i = 0; i < process_no; i++) {
                start_index += segments[i].no_of_words;
            }

            char *segment_start = input_map + segments[process_no].begin_offset;
            char *segment_end = input_map + segments[process_no].end_offset;

            if (IS_DEBUG_ENABLED) {
                printf("Child %d: Starting at offset %ld, expected to process %d words\n",
                            process_no, segments[process_no].begin_offset,
                            segments[process_no].no_of_words);
            }

            // Process words in this segment
            char buffer[MAX_PERMUTATION_LENGTH];
            size_t index = 0;
            size_t buffer_index = 0;
            char *current = segment_start;
            int in_word = 0;

            while (current < segment_end &&
                         index < (size_t)segments[process_no].no_of_words) {
                char c = *current++;

                // Handle word boundary
                if (c == ' ' || c == '\n' || c == '\t') {
                    if (in_word) {
                        // We have a complete word
                        buffer[buffer_index] = '\0';

                        // Ensure array bounds
                        if (start_index + index >= total_words) {
                            fprintf(stderr,
                                            "Child %d: Array index out of bounds: %zu >= %zu\n",
                                            process_no, start_index + index, total_words);
                            break;
                        }

                        // Generate random permutation
                        child_shared_memory->permutations[start_index + index] =
                                random_permutation(buffer);

                        if (IS_DEBUG_ENABLED && (index < 5 || index % 1000 == 0)) {
                            printf("Child %d: Processed word %zu: '%s'\n", process_no, index,
                                         buffer);
                        }

                        index++;
                        buffer_index = 0;
                        in_word = 0;
                    }
                } else {
                    // Add character to current word
                    if (buffer_index < MAX_PERMUTATION_LENGTH - 1) {
                        buffer[buffer_index++] = c;
                        in_word = 1;
                    }
                }
            }

            // Handle final word if any
            if (in_word && buffer_index > 0) {
                buffer[buffer_index] = '\0';

                if (start_index + index < total_words) {
                    child_shared_memory->permutations[start_index + index] =
                            random_permutation(buffer);
                    index++;
                }
            }

            if (IS_DEBUG_ENABLED) {
            printf("Child %d: Finished at offset %ld, processed %zu words\n",
                         process_no, current - input_map, index);
            }

            munmap(child_shared_memory, shm_size);
            exit(0);
        } else {
            // Parent - store child PID
            child_pids[process_no] = pid;
        }
    }

    if (IS_DEBUG_ENABLED) {
        // Wait for child processes
        printf("Parent: Waiting for child processes to complete\n");
    }

    for (int i = 0; i < no_of_processes; i++) {
        int status;
        pid_t pid = waitpid(child_pids[i], &status, 0);

        if (pid == -1) {
            perror("Error waiting for child process");
            continue;
        }

        if (IS_DEBUG_ENABLED) {
            if (WIFEXITED(status)) {
                printf("Child process %d (PID %d) exited with status %d\n", i,
                            child_pids[i], WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child process %d (PID %d) terminated by signal %d\n", i,
                            child_pids[i], WTERMSIG(status));
            }
        }
    }

    if (IS_DEBUG_ENABLED) {
        printf("Parent: All child processes completed, writing output\n");
    }

    // Write results to file
    write_permutations_to_file(permutations_filename, encoded_filename,
                                                         shared_memory->permutations, total_words);

    // Clean up resources
    munmap(shared_memory, shm_size);
    munmap(input_map, sb.st_size);
    close(input_fd);
    close(fd);

    return 0;
}

int decode_multiprocess(
    const char *encoded_filename,
    const char *permutations_filename,
    const char *output_filename,
    process_segment_info *segments,
    int no_of_processes
) {
    // Open input files
    FILE *encoded_file = fopen(encoded_filename, "r");
    FILE *permutations_file = fopen(permutations_filename, "r");

    if (encoded_file == NULL || permutations_file == NULL) {
        if (encoded_file) fclose(encoded_file);
        if (permutations_file) fclose(permutations_file);
        perror("Error opening input files");
        return -1;
    }

    // Open output file
    FILE *output = fopen(output_filename, "w");
    if (output == NULL) {
        perror("Error opening output file");
        fclose(encoded_file);
        fclose(permutations_file);
        return -1;
    }

    // First, count words in both files to verify they match
    size_t encoded_words = 0;
    size_t permutations_words = 0;

    // Count words in encoded file
    char c;
    int in_word = 0;
    rewind(encoded_file);

    while ((c = fgetc(encoded_file)) != EOF) {
        if (c == ' ' || c == '\n' || c == '\t') {
            if (in_word) {
                encoded_words++;
                in_word = 0;
            }
        } else {
            in_word = 1;
        }
    }

    if (in_word) {
        encoded_words++;
    }

    // Count words in permutations file
    in_word = 0;
    rewind(permutations_file);

    while ((c = fgetc(permutations_file)) != EOF) {
        if (c == ' ' || c == '\n' || c == '\t') {
            if (in_word) {
                permutations_words++;
                in_word = 0;
            }
        } else {
            in_word = 1;
        }
    }

    if (in_word) {
        permutations_words++;
    }

    if (IS_DEBUG_ENABLED) {
        printf("Found %zu words in encoded file and %zu in permutations file\n",
                    encoded_words, permutations_words);
    }

    if (encoded_words != permutations_words) {
        fprintf(stderr,
                        "Warning: Word count mismatch between encoded and permutations "
                        "files\n");
    }

    // Number of words to process
    size_t total_words = encoded_words;

    // Allocate arrays to store words and their positions
    char **encoded_words_array = (char **)calloc(total_words, sizeof(char *));
    char **permutation_strings_array =
            (char **)calloc(total_words, sizeof(char *));

    if (!encoded_words_array || !permutation_strings_array) {
        perror("Memory allocation failed");
        fclose(encoded_file);
        fclose(permutations_file);
        fclose(output);
        return -1;
    }

    // Read all words from encoded file
    rewind(encoded_file);
    char buffer[MAX_PERMUTATION_LENGTH * 2];
    size_t word_index = 0;
    size_t buffer_index = 0;
    in_word = 0;

    while ((c = fgetc(encoded_file)) != EOF && word_index < total_words) {
        if (c == ' ' || c == '\n' || c == '\t') {
            if (in_word) {
                buffer[buffer_index] = '\0';
                encoded_words_array[word_index] = strdup(buffer);

                if (!encoded_words_array[word_index]) {
                    perror("Memory allocation failed");
                    break;
                }

                word_index++;
                buffer_index = 0;
                in_word = 0;
            }
        } else {
            buffer[buffer_index++] = c;
            in_word = 1;

            if (buffer_index >= MAX_PERMUTATION_LENGTH * 2 - 1) {
                fprintf(stderr, "Word too long in encoded file\n");
                buffer[buffer_index] = '\0';
                break;
            }
        }
    }

    if (in_word && word_index < total_words) {
        buffer[buffer_index] = '\0';
        encoded_words_array[word_index] = strdup(buffer);
        word_index++;
    }

    // Read all permutation strings
    rewind(permutations_file);
    word_index = 0;
    buffer_index = 0;
    in_word = 0;

    while ((c = fgetc(permutations_file)) != EOF && word_index < total_words) {
        if (c == ' ' || c == '\n' || c == '\t') {
            if (in_word) {
                buffer[buffer_index] = '\0';
                permutation_strings_array[word_index] = strdup(buffer);

                if (!permutation_strings_array[word_index]) {
                    perror("Memory allocation failed");
                    break;
                }

                word_index++;
                buffer_index = 0;
                in_word = 0;
            }
        } else {
            buffer[buffer_index++] = c;
            in_word = 1;

            if (buffer_index >= MAX_PERMUTATION_LENGTH * 2 - 1) {
                fprintf(stderr, "Permutation string too long\n");
                buffer[buffer_index] = '\0';
                break;
            }
        }
    }

    if (in_word && word_index < total_words) {
        buffer[buffer_index] = '\0';
        permutation_strings_array[word_index] = strdup(buffer);
        word_index++;
    }

    if (IS_DEBUG_ENABLED) {
        printf("Read %zu encoded words and %zu permutation strings\n", word_index,
                    word_index);
    }

    // Now decode the words sequentially
    int error_count = 0;
    int success_count = 0;

    for (size_t i = 0; i < total_words; i++) {
        if (!encoded_words_array[i] || !permutation_strings_array[i]) {
            fprintf(stderr, "NULL pointer for word %zu\n", i);
            error_count++;
            continue;
        }

        // Decode permutation
        int_permutation ip = decode_int_permutation(permutation_strings_array[i]);

        if (ip.int_permutation == NULL || ip.length == 0) {
            fprintf(stderr, "Failed to decode permutation for word %zu\n", i);
            error_count++;
            // Output placeholder but not after the last word
            if (i < total_words - 1) {
                fprintf(output, " ");
            }
            continue;
        }

        // Create permutation
        permutation p;
        memset(&p, 0, sizeof(permutation));

        strncpy(p.char_permutation, encoded_words_array[i],
                        MAX_PERMUTATION_LENGTH - 1);
        p.length = ip.length;

        for (size_t j = 0; j < ip.length && j < MAX_PERMUTATION_LENGTH; j++) {
            p.int_permutation[j] = ip.int_permutation[j];
        }

        // Decode word
        char *decoded = decode_permutation(p);

        if (decoded == NULL || decoded[0] == '\0') {
            fprintf(stderr, "Failed to decode word %zu\n", i);
            error_count++;
            // Output placeholder but not after the last word
            if (i < total_words - 1) {
                fprintf(output, " ");
            }
        } else {
            fprintf(output, "%s",
                            decoded);    // Write the word without a trailing space

            // Only add space if this is not the last word
            if (i < total_words - 1) {
                fprintf(output, " ");
            }

            success_count++;

            if (IS_DEBUG_ENABLED && i % 1000 == 0) {
                printf("Decoded word %zu: '%s' -> '%s'\n", i, encoded_words_array[i],
                             decoded);
            }
        }

        // Clean up
        free(ip.int_permutation);
        free(decoded);

        // Periodically flush the output
        if (i % 10000 == 0) {
            fflush(output);
        }
    }

    if (IS_DEBUG_ENABLED) {
        printf("Decoding completed: %d successes, %d errors\n", success_count,
                    error_count);
    }

    // Clean up
    for (size_t i = 0; i < total_words; i++) {
        if (encoded_words_array[i]) free(encoded_words_array[i]);
        if (permutation_strings_array[i]) free(permutation_strings_array[i]);
    }

    free(encoded_words_array);
    free(permutation_strings_array);

    fclose(encoded_file);
    fclose(permutations_file);
    fclose(output);

    return error_count;
}

void write_permutations_to_file(
    const char *permutations_filename,
    const char *encoded_filename,
    permutation *permutations,
    size_t length
) {
    FILE *permutations_file = fopen(permutations_filename, "w");
    if (permutations_file == NULL) {
        perror("Error opening permutations file");
        return;
    }

    FILE *encoded_file = fopen(encoded_filename, "w");
    if (encoded_file == NULL) {
        perror("Error opening encoded file");
        fclose(permutations_file);
        return;
    }

    printf("Writing %zu permutations to files\n", length);

    size_t valid_perms = 0;
    size_t empty_perms = 0;
    size_t invalid_perms = 0;

    for (size_t i = 0; i < length; i++) {
        // Handle empty permutations with a special marker
        if (permutations[i].length == 0 ||
                permutations[i].char_permutation[0] == '\0') {
            // For the last word, don't add a trailing space
            if (i < length - 1) {
                fprintf(permutations_file, "0- ");
                fprintf(encoded_file, "_ ");
            } else {
                fprintf(permutations_file, "0-");
                fprintf(encoded_file, "_");
            }

            if (i % 1000 == 0) {
                printf("Empty permutation at index %zu, writing placeholder\n", i);
            }

            empty_perms++;
            continue;
        }

        // Verify permutation data
        int valid = 1;
        for (size_t j = 0; j < permutations[i].length; j++) {
            if (permutations[i].char_permutation[j] == '\0') {
                // Fix null character by replacing with space
                permutations[i].char_permutation[j] = ' ';
            }
        }

        // Ensure the string is null-terminated
        permutations[i].char_permutation[permutations[i].length] = '\0';

        // Create permutation string
        char *encoding = encoding_permutation_to_string(permutations[i]);
        if (encoding == NULL) {
            fprintf(stderr, "Failed to encode permutation at index %zu\n", i);
            invalid_perms++;

            // Use empty permutation as fallback, handling last word case
            if (i < length - 1) {
                fprintf(permutations_file, "0- ");
                fprintf(encoded_file, "_ ");
            } else {
                fprintf(permutations_file, "0-");
                fprintf(encoded_file, "_");
            }
            continue;
        }

        // Write both permutation and encoded word, with special handling for last
        // word
        if (i < length - 1) {
            fprintf(permutations_file, "%s ", encoding);
            fprintf(encoded_file, "%s ", permutations[i].char_permutation);
        } else {
            // No trailing space for the last word
            fprintf(permutations_file, "%s", encoding);
            fprintf(encoded_file, "%s", permutations[i].char_permutation);
        }

        free(encoding);
        valid_perms++;

        // Periodically flush files to avoid buffer issues
        if (i % 10000 == 0) {
            fflush(permutations_file);
            fflush(encoded_file);
        }
    }

    // No trailing newlines - this was removed to avoid affecting MD5 checksums
    fflush(permutations_file);
    fflush(encoded_file);

    printf("Successfully wrote %zu valid permutations, %zu empty, %zu invalid\n",
                 valid_perms, empty_perms, invalid_perms);

    fclose(permutations_file);
    fclose(encoded_file);
}

char *encoding_permutation_to_string(permutation p) {
    // Handle empty permutation
    if (p.length == 0) {
        char *result = (char *)malloc(3);
        if (result == NULL) {
            perror("Failed to allocate memory for empty permutation string");
            return NULL;
        }
        strcpy(result, "0-");
        return result;
    }

    // Allocate enough memory for the string (5 chars per index plus null
    // terminator)
    char *result = (char *)malloc(p.length * 5 + 1);
    if (result == NULL) {
        perror("Failed to allocate memory for permutation string");
        return NULL;
    }

    // Build the string with indices separated by dashes
    int pos = 0;
    for (size_t i = 0; i < p.length; i++) {
        // Add the index value
        int written = snprintf(result + pos, 5, "%d", p.int_permutation[i]);
        if (written < 0) {
            free(result);
            return NULL;
        }
        pos += written;

        // Add dash separator if not the last number
        if (i < p.length - 1) {
            result[pos++] = '-';
        }
    }

    // Null terminate the string
    result[pos] = '\0';

    return result;
}
