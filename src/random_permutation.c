#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "headers/random_permutation.h"

permutation random_permutation(char* array, size_t array_size) {

    char* char_permutation = (char*) malloc(array_size * sizeof(char) + 1);
    int* int_permutation = (int*) malloc(array_size * sizeof(int));

    for(size_t i = 0; i < array_size; i++) {
        char_permutation[i] = array[i];
        int_permutation[i] = i;
    }

    srand(time(NULL)); 

    for(size_t i = 0; i < array_size - 2; i++) {
        size_t random_index = (size_t) (i + rand() % (array_size - i - 1));
        

        int temp = int_permutation[i];
        int_permutation[i] = int_permutation[random_index];
        int_permutation[random_index] = temp;

        char aux = char_permutation[i];
        char_permutation[i] = char_permutation[random_index];
        char_permutation[random_index] = aux;
    }

    
    char_permutation[array_size] = '\0';
    
    permutation to_return;
    to_return.char_permutation = char_permutation;
    to_return.int_permutation = int_permutation;
    return to_return;
}

void free_permutation(permutation p){
    free(p.char_permutation);
    free(p.int_permutation);
}

void print_permutation(permutation p){
    printf("Permutation: ");
    for(size_t i = 0; p.char_permutation[i] != '\0'; i++)
        printf("%d ", p.int_permutation[i]);
    printf("\n");
    printf("Word: %s\n", p.char_permutation);
}
