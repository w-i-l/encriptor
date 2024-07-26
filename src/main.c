#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "headers/random_permutation.h"
#include "headers/coder.h"
#include "headers/coder_multiprocess.h"

int main(void) {
    char word[] = "Hello, World!";
    permutation p = random_permutation(word);
    print_permutation(p);

    char* decoded = decode_permutation(p);
    printf("Decoded: %s\n", decoded);

    free_permutation(p);

    return 0;
}
