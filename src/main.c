#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "headers/random_permutation.h"
#include "headers/coder.h"
#include "headers/coder_multiprocess.h"
#include "headers/segmentation.h"

int main(void) {
    char input_filename[] = "files/file.txt";
    char permutations_filename[] = "files/permutations.txt";
    char encoded_filename[] = "files/encoded.txt";
    char output_filename[] = "files/decoded.txt";
    int process_count = 4;

    process_segment_info* segments = get_segments_for_file(input_filename, process_count);

    for(int i = 0; i < process_count; i++) {
        print_segment_info(segments[i]);
        printf("\n");
    }
    printf("Is file segmented correctly: %d\n", is_file_segmented_correctly(input_filename, segments, process_count));

    int shm_fd = encode_multiprocess(input_filename, permutations_filename, encoded_filename, segments, process_count);
    if(shm_fd == -1) {
        perror("Error encoding file");
        return 1;
    }

    printf("\n\n DECODING \n\n");

    process_segment_info* segments2 = get_segments_for_file(encoded_filename, process_count);
    for(int i = 0; i < process_count; i++) {
        print_segment_info(segments[i]);
        printf("\n");
    }
    
    int decode_result = decode_multiprocess(encoded_filename, permutations_filename, output_filename, segments2, process_count);
    if(decode_result == -1) {
        perror("Error decoding file");
        return 1;
    }


    free(segments);
    free(segments2);

    // permutation p = random_permutation(input_filename);
    // print_permutation(p);
    // char* decoded = decode_permutation(p);
    // printf("%s", decoded);

    return 0;
}
