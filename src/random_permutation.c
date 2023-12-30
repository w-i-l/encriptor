#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "headers/random_permutation.h"
#include <errno.h>
#include <string.h>

permutation random_permutation(char* array, size_t array_size) {

    permutation to_return;

    for(size_t i = 0; i < array_size; i++) {
        to_return.char_permutation[i] = array[i];
        to_return.int_permutation[i] = i;
    }

    srand(time(NULL)); 

    for(size_t i = 0; i < array_size - 2; i++) {
        size_t random_index = (size_t) (i + rand() % (array_size - i - 1));
    
        int temp = to_return.int_permutation[i];
        to_return.int_permutation[i] = to_return.int_permutation[random_index];
        to_return.int_permutation[random_index] = temp;

        char aux = to_return.char_permutation[i];
        to_return.char_permutation[i] = to_return.char_permutation[random_index];
        to_return.char_permutation[random_index] = aux;
    }
    
    to_return.char_permutation[array_size - 1] = '\0';

    return to_return;
}

char* decode_permutation(permutation p, size_t array_size) {
    char* to_return = (char*) malloc(array_size * sizeof(char) + 1);
    
    for(size_t i = 0; i < array_size; i++) {
        to_return[p.int_permutation[i]] = p.char_permutation[i];
    }

    to_return[array_size] = '\0';
    return to_return;
}

void free_permutation(permutation p){
    free(p.char_permutation);
    free(p.int_permutation);
}

void print_permutation(permutation p){
    printf("Permutation: ");
    size_t i;
    for(i = 0; p.char_permutation[i] != '\0'; i++)
        printf("%d ", p.int_permutation[i]);
    printf(" length: %ld\n", i);
    printf("Word: %s\n", p.char_permutation);
}
