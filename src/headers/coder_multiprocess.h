#ifndef CODER_MULTIPROCESS_H
#define CODER_MULTIPROCESS_H

#include "segmentation.h"

int encode_multiprocess(const char* input_file, const char* output_file, process_segment_info* segments, int no_of_processes);
size_t round_to_page_size(size_t size);
#endif
