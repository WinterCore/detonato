#ifndef STYLE_C
#define STYLE_C

#include <stdint.h>
#include <stdbool.h>

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

void destroy_digit_style(DigitStyle *style);

void write_digit_style(const char *name, DigitStyle *style);
void load_digit_style(const char *name, DigitStyle *out);

bool is_segment_visible(DigitStyle *style, uint8_t digit, uint8_t segment_index);

#endif
