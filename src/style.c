#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "style.h"
#include "aids.h"

void write_digit_style(char *name, DigitStyle *style) {
    char filename[100];
    sprintf(filename, "styles/%s.bin", name);
    FILE *file = fopen(filename, "w+");

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
