#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "style.h"
#include "aids.h"



void write_digit_style(const char *name, DigitStyle *style) {
    char filename[100];
    snprintf(filename, sizeof(filename), "styles/%s.bin", name);
    FILE *file = fopen(filename, "wb");

    if (file == NULL) {
        PANIC("Failed to open file: %s", strerror(errno));
    }

    fwrite(&style->aspect_ratio, sizeof(style->aspect_ratio), 1, file);
    fwrite(&style->segment_count, sizeof(style->segment_count), 1, file);
    fwrite(style->segment_bitmask, sizeof(style->segment_bitmask[0]), 10, file);
    fwrite(style->segment_vertex_start, sizeof(style->segment_vertex_start[0]), style->segment_count, file);
    fwrite(style->segment_vertex_count, sizeof(style->segment_vertex_count[0]), style->segment_count, file);
    fwrite(&style->total_floats, sizeof(style->total_floats), 1, file);
    fwrite(style->vertices, sizeof(style->vertices[0]), style->total_floats, file);

    fclose(file);
}

void load_digit_style(const char *name, DigitStyle *style) {
    char filename[100];
    snprintf(filename, sizeof(filename), "styles/%s.bin", name);
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        PANIC("Failed to open file: %s", strerror(errno));
    }

    fread(&style->aspect_ratio, sizeof(style->aspect_ratio), 1, file);
    fread(&style->segment_count, sizeof(style->segment_count), 1, file);

    if (style->segment_count <= 0 || MAX_SEGMENTS < style->segment_count) {
        PANIC("Invalid segment count!");
    }

    fread(style->segment_bitmask, sizeof(style->segment_bitmask[0]), 10, file);
    fread(style->segment_vertex_start, sizeof(style->segment_vertex_start[0]), style->segment_count, file);
    fread(style->segment_vertex_count, sizeof(style->segment_vertex_count[0]), style->segment_count, file);
    fread(&style->total_floats, sizeof(style->total_floats), 1, file);

    style->vertices = malloc(style->total_floats * sizeof(float));
    fread(style->vertices, sizeof(style->vertices[0]), style->total_floats, file);

    fclose(file);
}

void destroy_digit_style(DigitStyle *style) {
    free(style->vertices);
    style->vertices = NULL;
}

bool is_segment_visible(DigitStyle *style, uint8_t digit, uint8_t segment_index) {
    if (digit > 9) {
        PANIC("Invalid digit %hu", digit);
    }

    return (style->segment_bitmask[digit] >> segment_index) & 0b1;
}
