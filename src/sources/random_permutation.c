#include "../headers/random_permutation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

permutation create_permutation(char* word, int* sequence) {
    permutation p;

    size_t input_size = strlen(word);
    p.char_permutation = (char*) malloc(input_size * sizeof(char) + 1);
    p.int_permutation = (int*) malloc(input_size * sizeof(int));

    strcpy(p.char_permutation, word);
    memcpy(p.int_permutation, sequence, input_size * sizeof(int));
    p.length = input_size;

    return p;
}

permutation create_empty_permutation(char* word) {
    permutation p;

    size_t input_size = strlen(word);
    p.char_permutation = (char*) malloc(input_size * sizeof(char) + 1);
    p.int_permutation = (int*) malloc(input_size * sizeof(int));
    p.length = input_size;

    strcpy(p.char_permutation, word);
    for(size_t i = 0; i < input_size; i++) {
        p.int_permutation[i] = i;
    }

    return p;
}

void free_permutation(permutation p) {
    free(p.char_permutation);
    free(p.int_permutation);
}

permutation random_permutation(char* word) {
    size_t input_size = strlen(word) + 1;
    permutation to_return = create_empty_permutation(word);

    for(size_t i = 0; i < input_size - 2; i++) {
        size_t random_index = (size_t) (i + rand() % (input_size - i - 1));
    
        int temp = to_return.int_permutation[i];
        to_return.int_permutation[i] = to_return.int_permutation[random_index];
        to_return.int_permutation[random_index] = temp;

        char aux = to_return.char_permutation[i];
        to_return.char_permutation[i] = to_return.char_permutation[random_index];
        to_return.char_permutation[random_index] = aux;
    }

    to_return.char_permutation[input_size - 1] = '\0';

    return to_return;
}

char* decode_permutation(permutation p) {
    char* to_return = (char*) malloc(p.length * sizeof(char) + 1);
    
    for(size_t i = 0; i < p.length; i++) {
        to_return[p.int_permutation[i]] = p.char_permutation[i];
    }

    to_return[p.length] = '\0';
    return to_return;
}

void print_permutation(permutation p) {
    printf("Permutation: ");
    
    size_t i;
    for(i = 0; p.char_permutation[i] != '\0'; i++)
        printf("%d ", p.int_permutation[i]);

    printf(" length: %ld\n", i);
    printf("Word: %s\n", p.char_permutation);
}

