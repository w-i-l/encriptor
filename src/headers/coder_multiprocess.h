#ifndef CODER_MULTIPROCESS_H
#define CODER_MULTIPROCESS_H

#include "random_permutation.h"

void encode_multiprocess(const char* input_file, const char* output_file);

typedef struct {
    int no_of_words;
    int no_of_chars;
    size_t begin_offset;
    size_t segment_length;
} process_segment_info;

#endif
