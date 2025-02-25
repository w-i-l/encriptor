#ifndef CODER_HEADER_H
#define CODER_HEADER_H

void encode(
    const char* input_file,
    const char* output_file, 
    const char* permutation_file
);

void decode(
    const char* input_file, 
    const char* output_file, 
    const char* permutation_file
);

#endif
