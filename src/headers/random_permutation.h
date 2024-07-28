#ifndef RANDOM_PERMUTATION_H
#define RANDOM_PERMUTATION_H

#include <stdio.h>

#define MAX_PERMUTATION_LENGTH 500

typedef struct {
    char char_permutation[MAX_PERMUTATION_LENGTH];
    int int_permutation[MAX_PERMUTATION_LENGTH];
    size_t length;
} permutation;

permutation create_permutation(char*, int*);
permutation create_empty_permutation(char*);

permutation random_permutation(char*);
char* decode_permutation(permutation);

void free_permutation(permutation);

void print_permutation(permutation);

#endif

