#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/random_permutation.h"
#include "headers/coder.h"
#include "headers/coder_multiprocess.h"

int main(void) {

    char input_file[] = "file.txt";
    char output_file[] = "encoded.txt";
    char permutation_file[] = "permutation.txt";
    // char decoded_file[] = "decoded.txt";

    // encode(input_file, output_file, permutation_file);
    encode_multiprocess(input_file, output_file, permutation_file);
    // decode(output_file, decoded_file, permutation_file);

    // permutation p = random_permutation(c, strlen(c));

    // print_permutation(p);

    // char* decoded = decode_permutation(p, strlen(c));
    // printf("%s\n", decoded);

    return 0;
}
