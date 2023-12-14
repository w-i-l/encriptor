#ifndef RANDOM_PERMUTATION_H
#define RANDOM_PERMUTATION_H

typedef struct {
    char* char_permutation;
    int* int_permutation;
} permutation;

permutation random_permutation(char*, size_t);
char* decode_permutation(permutation, size_t);

void free_permutation(permutation);

void print_permutation(permutation);

#endif

