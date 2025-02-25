#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../headers/random_permutation.h"


permutation create_permutation(char *word, int *sequence) {
    permutation p;

    // Initialize the permutation
    memset(p.char_permutation, 0, MAX_PERMUTATION_LENGTH);
    memset(p.int_permutation, 0, MAX_PERMUTATION_LENGTH * sizeof(int));

    if (word == NULL || sequence == NULL) {
        fprintf(stderr, "Warning: NULL pointers passed to create_permutation\n");
        p.length = 0;
        return p;
    }

    size_t input_size = strlen(word);
    if (input_size >= MAX_PERMUTATION_LENGTH) {
        fprintf(
            stderr,
            "Warning: Word is too long for permutation (%zu characters)\n",
            input_size
        );
        input_size = MAX_PERMUTATION_LENGTH - 1;
    }

    strncpy(p.char_permutation, word, input_size);
    p.char_permutation[input_size] = '\0';

    memcpy(p.int_permutation, sequence, input_size * sizeof(int));
    p.length = input_size;

    return p;
}


permutation random_permutation(char *word) {
    permutation p;
    memset(&p, 0, sizeof(permutation));

    // Get word length, handle empty strings
    size_t len = (word != NULL) ? strlen(word) : 0;
    if (len == 0) {
        p.length = 0;
        return p;
    }

    // Handle oversized words
    if (len >= MAX_PERMUTATION_LENGTH - 1) {
        len = MAX_PERMUTATION_LENGTH - 1;
    }

    // Copy the word
    strncpy(p.char_permutation, word, len);
    p.char_permutation[len] = '\0';

    // Initialize permutation indices
    for (size_t i = 0; i < len; i++) {
        p.int_permutation[i] = i;
    }

    // Set the length
    p.length = len;

    // Fisher-Yates shuffle (for words with more than one character)
    if (len > 1) {
        // Seed random if not already seeded
        static int seeded = 0;
        if (!seeded) {
            srand(time(NULL));
            seeded = 1;
        }

        for (size_t i = 0; i < len - 1; i++) {
            // Get random index between i and len-1
            size_t j = i + rand() % (len - i);

            // Swap characters
            char temp_char = p.char_permutation[i];
            p.char_permutation[i] = p.char_permutation[j];
            p.char_permutation[j] = temp_char;

            // Swap indices
            int temp_idx = p.int_permutation[i];
            p.int_permutation[i] = p.int_permutation[j];
            p.int_permutation[j] = temp_idx;
        }
    }

    return p;
}


char *decode_permutation(permutation p) {
    // Handle empty permutation
    if (p.length == 0) {
        char *empty = (char *)malloc(1);
        if (empty) {
            empty[0] = '\0';
        }
        return empty;
    }

    // Allocate memory for decoded string
    char *decoded = (char *)calloc(p.length + 1, sizeof(char));
    if (decoded == NULL) {
        perror("Memory allocation failed in decode_permutation");
        return NULL;
    }

    // Apply permutation indices
    for (size_t i = 0; i < p.length; i++) {
        // Validate index is within bounds
        if (p.int_permutation[i] >= 0 && p.int_permutation[i] < (int)p.length) {
            decoded[p.int_permutation[i]] = p.char_permutation[i];
        } else {
            // Use fallback for invalid index - place char at current position
            fprintf(stderr, "Invalid permutation index %d (max %zu)\n",
                            p.int_permutation[i], p.length - 1);
            decoded[i] = p.char_permutation[i];
        }
    }

    // Ensure null termination
    decoded[p.length] = '\0';

    // Validate for embedded nulls
    for (size_t i = 0; i < p.length; i++) {
        if (decoded[i] == '\0') {
            // Replace nulls with spaces
            decoded[i] = ' ';
        }
    }

    return decoded;
}


void print_permutation(permutation p) {
    printf("Permutation: ");

    size_t i;
    for (i = 0; p.char_permutation[i] != '\0'; i++)
        printf("%d ", p.int_permutation[i]);

    char *decoded = decode_permutation(p);
    printf(" length: %ld\n", i);
    printf("Word: %s -> %s\n", decoded, p.char_permutation);
    printf("-------------\n");
    free(decoded);
}


int_permutation decode_int_permutation(char *encoded) {
    int_permutation to_return;
    to_return.int_permutation = NULL;
    to_return.length = 0;

    // Handle NULL or empty input
    if (encoded == NULL || encoded[0] == '\0') {
        // Return an identity permutation (index 0)
        to_return.int_permutation = (int *)calloc(1, sizeof(int));
        if (to_return.int_permutation == NULL) {
            perror("Memory allocation failed in decode_int_permutation");
            return to_return;
        }
        to_return.int_permutation[0] = 0;
        to_return.length = 1;
        return to_return;
    }

    // Special handling for the empty permutation marker "0-"
    if (strcmp(encoded, "0-") == 0 ||
            (encoded[0] == '0' && encoded[1] == '-' &&
             (encoded[2] == '\0' || encoded[2] == ' '))) {
        to_return.int_permutation = (int *)calloc(1, sizeof(int));
        if (to_return.int_permutation == NULL) {
            perror("Memory allocation failed in decode_int_permutation");
            return to_return;
        }
        to_return.int_permutation[0] = 0;
        to_return.length = 1;
        return to_return;
    }

    // Count the number of indices by counting the dashes + 1
    size_t num_indices = 1;
    for (size_t i = 0; encoded[i] != '\0'; i++) {
        if (encoded[i] == '-') {
            num_indices++;
        }
    }

    // If we couldn't determine indices, use identity permutation
    if (num_indices == 0) {
        to_return.int_permutation = (int *)calloc(1, sizeof(int));
        if (to_return.int_permutation) {
            to_return.int_permutation[0] = 0;
            to_return.length = 1;
        }
        return to_return;
    }

    // Allocate memory for the permutation
    to_return.int_permutation = (int *)calloc(num_indices, sizeof(int));
    if (to_return.int_permutation == NULL) {
        perror("Memory allocation failed for permutation indices");
        return to_return;
    }

    // Parse the indices from the string
    size_t index = 0;
    char *token = strtok(strdup(encoded), "-");

    while (token != NULL && index < num_indices) {
        // Convert token to integer
        char *endptr;
        long value = strtol(token, &endptr, 10);

        // Check for conversion errors
        if (*endptr != '\0' || value < 0 || value > INT_MAX) {
            fprintf(stderr, "Invalid index in permutation: %s\n", token);
        } else {
            to_return.int_permutation[index++] = (int)value;
        }

        token = strtok(NULL, "-");
    }

    to_return.length = index;

    // If we couldn't parse any indices, use identity permutation
    if (to_return.length == 0 && to_return.int_permutation) {
        to_return.int_permutation[0] = 0;
        to_return.length = 1;
    }

    return to_return;
}


void debug_permutation_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for debugging");
        return;
    }

    printf("Content of %s:\n", filename);

    char buffer[4096];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytes_read] = '\0';

    printf("%s\n", buffer);
    printf("Total bytes: %zu\n", bytes_read);

    fclose(file);
}