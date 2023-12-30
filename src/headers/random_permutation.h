#ifndef RANDOM_PERMUTATION_H
#define RANDOM_PERMUTATION_H

#define PERMUTATION_ARRAY_MAX_SIZE 500

typedef struct {
    char char_permutation[PERMUTATION_ARRAY_MAX_SIZE];
    int int_permutation[PERMUTATION_ARRAY_MAX_SIZE];
} permutation;

permutation random_permutation(char*, size_t);
char* decode_permutation(permutation, size_t);

void free_permutation(permutation);

void print_permutation(permutation);

#endif

