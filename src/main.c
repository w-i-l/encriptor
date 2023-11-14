#include <stdio.h>
#include "headers/random_permutation.h"

int main(void) {
    char array[] = "Hello";
    size_t size = sizeof(array) / sizeof(array[0]);

    permutation returned = random_permutation(array, size);
    print_permutation(returned);
    free_permutation(returned);
    
    return 0;
}
