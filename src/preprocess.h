#ifndef PROCESS_C
#define PROCESS_C

#include <stdint.h>
#include "style.h"

typedef struct DigitSegment {
    int vertices_count;
    float vertices[];
} DigitSegment;

typedef struct SegmentDigitShape {
    DigitSegment *segments[MAX_SEGMENTS];
    int segment_count;

    float aspect_ratio;
} SegmentDigitShape;

SegmentDigitShape preprocess_segment_svg(char *path);
void destroy_segment_shape(SegmentDigitShape *shape);
DigitStyle build_segment_mesh(SegmentDigitShape *shape);

#endif
