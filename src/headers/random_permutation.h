#ifndef RANDOM_PERMUTATION_H
#define RANDOM_PERMUTATION_H

#include <stdio.h>

typedef struct {
    char* char_permutation;
    int* int_permutation;
    size_t length;
} permutation;

permutation create_permutation(char*, int*);
permutation create_empty_permutation(char*);

permutation random_permutation(char*);
char* decode_permutation(permutation);

void free_permutation(permutation);

void print_permutation(permutation);

#endif

