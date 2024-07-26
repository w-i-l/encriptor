#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int no_of_words;
    int no_of_chars;
    size_t begin_offset;
    size_t end_offset;
    size_t segment_length;
} process_segment_info;

void print_segment_info(process_segment_info);

process_segment_info* get_segments_for_file(const char*, int);

void copy_file_segment(const char*, const char*, process_segment_info*, int);
bool is_file_segmented_correctly(const char*, process_segment_info*, int);

#endif