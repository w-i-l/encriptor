#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "headers/random_permutation.h"
#include "headers/coder.h"
#include "headers/coder_multiprocess.h"
#include "headers/segmentation.h"

int main(void) {
    char input_filename[] = "files/file.txt";
    char output_filename[] = "files/verification.txt";
    int process_count = 1;

    process_segment_info* segments = get_segments_for_file(input_filename, process_count);

    for(int i = 0; i < process_count; i++) {
        print_segment_info(segments[i]);
        printf("\n");
    }

    copy_file_segment(input_filename, output_filename, segments, process_count);
    size_t sum = 0;
    for(int i = 0; i < process_count; i++) {
        sum += segments[i].segment_length;
    }
    printf("Sum: %ld\n", sum);
    printf("Is file segmented correctly: %d\n", is_file_segmented_correctly(input_filename, segments, process_count));

    return 0;
}
