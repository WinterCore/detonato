#ifndef STYLE_C
#define STYLE_C

#include <stdint.h>

#define MAX_SEGMENTS 64

typedef struct {
    float aspect_ratio;

    int segment_count;
    uint64_t segment_bitmask[10];

    int segment_vertex_start[MAX_SEGMENTS];
    int segment_vertex_count[MAX_SEGMENTS];
    int total_floats;
    float *vertices;
} DigitStyle;

void write_digit_style(char *name, DigitStyle *style);

#endif
