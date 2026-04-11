#ifndef AIDS_H
#define AIDS_H
#define PI 3.14159265359

#include <stdio.h>
#include <stdlib.h>

#define DEBUG_PRINT_ENABLED 1
#define DEBUG_PRINTF(...) \
    do { if (DEBUG_PRINT_ENABLED) fprintf(stderr, __VA_ARGS__); } while (0)

#define UNIMPLEMENTED \
    fprintf(stderr, "UNIMPLEMENTED"); \
    exit(EXIT_FAILURE); \

#define UNREACHABLE \
    fprintf(stderr, "UNREACHABLE"); \
    exit(EXIT_FAILURE); \

#define PANIC(...) \
    do { \
        fprintf(stderr, "PANIC at %s:%d: ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
        exit(EXIT_FAILURE); \
    } while (0)

#define MAX(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define MIN(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
    _a > _b ? _b : _a; })

float clamp(float min, float max, float v);
float lerp(float a, float b, float v);
float remap(float source_a, float source_b, float dest_a, float dest_b, float value);

typedef struct Vec2 {
    float x, y;
} Vec2;

typedef struct Vec3 {
    float x, y, z;
} Vec3;

#endif
