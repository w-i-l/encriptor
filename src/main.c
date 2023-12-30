#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "headers/random_permutation.h"
#include "headers/coder.h"
#include "headers/coder_multiprocess.h"

void segfault_handler(int signal) {
    printf("Segmentation fault caught! Signal: %d\n", signal);
    printf("errno: %d\n", errno);
    // You can perform additional actions or logging here if needed
    exit(EXIT_FAILURE); // Exit the program or take appropriate action
}

int main(void) {

    char input_file[] = "file.txt";
    char output_file[] = "encoded.txt";
    // char permutation_file[] = "permutation.txt";
    // char decoded_file[] = "decoded.txt";
    signal(SIGSEGV, segfault_handler);
    // encode(input_file, output_file, permutation_file);
    encode_multiprocess(input_file, output_file);
    // decode(output_file, decoded_file, permutation_file);

    // char c[] = "ana are mere";
    // permutation p = random_permutation(c, strlen(c));

    // print_permutation(p);

    // char* decoded = decode_permutation(p, strlen(c));
    // printf("%s\n", decoded);

    return 0;
}
