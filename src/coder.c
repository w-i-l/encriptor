#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/random_permutation.h"
#include "headers/coder.h"

void encode(const char* input_file, const char* output_file, const char* permutation_file) {
    FILE* input = fopen(input_file, "r");
    FILE* permutations = fopen(permutation_file, "w");
    FILE* encoded = fopen(output_file, "w");

    if(permutations == NULL || encoded == NULL || input == NULL) {
        printf("Error opening file\n");
        return;
    }

    char buffer[100] = {0};
    permutation aux_permutation;
    aux_permutation.char_permutation = NULL;
    aux_permutation.int_permutation = NULL;

    do {

        memset(buffer, 0, 100);
        free_permutation(aux_permutation);

        fscanf(input, "%s\n", buffer);

        size_t size = 0;
        for(; buffer[size] != '\0'; size++);
        size++;

        aux_permutation = random_permutation(buffer, size);

        if(fprintf(encoded, "%s ", aux_permutation.char_permutation) == 0) {
            printf("Error writing to file\n");
            return;
        }

        for(size_t i = 0; i < size; i++) {
            if(fprintf(permutations, "%d-", aux_permutation.int_permutation[i]) == 0) {
                printf("Error writing to file\n");
                return;
            }
        }
        fprintf(permutations, " ");

    } while(!feof(input));

    fclose(input);
    fclose(permutations);
    fclose(encoded);
}

void decode(const char* input_file, const char* output_file, const char* permutation_file) {
    FILE* input = fopen(input_file, "r");
    FILE* permutations = fopen(permutation_file, "r");
    FILE* decoded = fopen(output_file, "w");

    if(permutations == NULL || decoded == NULL || input == NULL) {
        printf("Error opening file\n");
        return;
    }

    char buffer[100] = {0};
    char permutation_buffer[100] = {0};
    char* decoded_buffer;
    permutation aux_permutation;

    do {
        memset(buffer, 0, 100);
        memset(permutation_buffer, 0, 100);

        fscanf(input, " %s ", buffer);
        fscanf(permutations, " %s ", permutation_buffer);

        aux_permutation.char_permutation = buffer;
        aux_permutation.int_permutation = (int*) malloc(strlen(permutation_buffer) / 2 * sizeof(int));
        
        if (aux_permutation.int_permutation == NULL) {
            printf("Error allocating memory\n");
            return;
        }

        int index = 0;
        for(size_t i = 0; i < strlen(permutation_buffer) - 1;) {
            if (permutation_buffer[i] == '-'){
                i++;
                int aux = 0;
                
                while(permutation_buffer[i] != '-') {
                    aux *= 10;
                    aux += permutation_buffer[i] - '0';
                    i++;
                }

                aux_permutation.int_permutation[index ++] = aux;  
                continue; 
            }

            aux_permutation.int_permutation[index ++] = permutation_buffer[i] - '0';
            i++;
        }

        decoded_buffer = decode_permutation(aux_permutation, index);

        if(fprintf(decoded, "%s ", decoded_buffer) == 0) {
            printf("Error writing to file\n");
            return;
        }

        free(aux_permutation.int_permutation);

    } while(!feof(input));

    fclose(input);
    fclose(permutations);
    fclose(decoded);
}
