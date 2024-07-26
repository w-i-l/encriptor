#include "headers/random_permutation.h"

permutation random_permutation(const char* input, size_t length) {
    permutation p;
    
    // Allocate memory for char_permutation
    p.char_permutation = malloc(length + 1);
    strcpy(p.char_permutation, "random");
    
    // Initialize int_permutation
    for (int i = 0; i < 10; i++) {
        p.int_permutation[i] = i;
    }
    
    return p;
}