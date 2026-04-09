#ifndef PROCESS_C
#define PROCESS_C

typedef struct SevenSegmentSegment {
    int vertices_count;
    float vertices[];
} SevenSegmentSegment;

typedef struct SevenSegmentShape {
    SevenSegmentSegment *segments[7];
    float aspect_ratio;
} SevenSegmentShape;

typedef struct {
    float *vertices;
    int total_floats;
    int segment_start[7];
    int segment_count[7];
} SegmentMeshData;

SevenSegmentShape preprocess_7_segment_svg(char *path);
SegmentMeshData build_segment_mesh(SevenSegmentShape *shape);

#endif
