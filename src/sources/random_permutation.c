#include "../headers/random_permutation.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

permutation create_permutation(char* word, int* sequence) {
    permutation p;

    size_t input_size = strlen(word);

    strcpy(p.char_permutation, word);
    memcpy(p.int_permutation, sequence, input_size * sizeof(int));
    p.length = input_size;
    // printf("Creating permutation: %s\n", p.char_permutation);
    // print_permutation(p);
    return p;
}

permutation create_empty_permutation(char* word) {
    permutation p;

    size_t input_size = strlen(word);
    p.length = input_size;

    strcpy(p.char_permutation, word);
    for(size_t i = 0; i < input_size; i++) {
        p.int_permutation[i] = i;
    }

    return p;
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

        // print_permutation(to_return);


    return to_return;
}

char* decode_permutation(permutation p) {
    char* to_return = (char*) malloc(p.length * sizeof(char) + 1);
    
    if(p.char_permutation[0] == '\0') {
        to_return[0] = '\0';
        return to_return;
    }




    for(size_t i = 0; i < p.length; i++) {
        to_return[p.int_permutation[i]] = p.char_permutation[i];
    }

    to_return[p.length] = '\0';
    // printf("Decoding: %s -> %s\n", p.char_permutation, to_return);
    return to_return;
}

void print_permutation(permutation p) {
    printf("Permutation: ");
    
    size_t i;
    for(i = 0; p.char_permutation[i] != '\0'; i++)
        printf("%d ", p.int_permutation[i]);

    char* decoded = decode_permutation(p);
    printf(" length: %ld\n", i);
    printf("Word: %s -> %s\n", decoded, p.char_permutation);
    printf("-------------\n");
    free(decoded);
}

int_permutation decode_int_permutation(char* encoded) {
    size_t length = strlen(encoded);
    int_permutation to_return;

    to_return.int_permutation = (int*) malloc(length * sizeof(int));
    to_return.length = length;

    // encoded: 10-3-5-2-1-4-0-6-7-8-9

    size_t index = 0;
    size_t i = 0;
    while(i < length) {
        int number = 0;
        while(encoded[i] != '-' && i < length) {
            number = number * 10 + (encoded[i] - '0');
            i++;
        }

        to_return.int_permutation[index++] = number;
        i++;
    }

    to_return.length = index;
    return to_return;
}