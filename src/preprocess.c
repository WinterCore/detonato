#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NANOSVG_IMPLEMENTATION

#include "preprocess.h"
#include "nanosvg.h"
#include "tesselator.h"

// Flatten a cubic bezier into `n` line segments
void flatten_cubic_bez(float* pts, int n, float* out, int* outCount, bool skip_first) {
    *outCount = 0;

    for (int i = 0; i <= n; i++) {
        if (skip_first && i == 0) {
            continue;
        }

        float t = (float)i / (float)n;
        float u = 1.0f - t;
        float x = u*u*u*pts[0] + 3*u*u*t*pts[2] + 3*u*t*t*pts[4] + t*t*t*pts[6];
        float y = u*u*u*pts[1] + 3*u*u*t*pts[3] + 3*u*t*t*pts[5] + t*t*t*pts[7];

        out[*outCount * 2 + 0] = x;
        out[*outCount * 2 + 1] = y;
        *outCount += 1;
    }
}

static SevenSegmentSegment *convert_shape_to_segment(NSVGshape *shape) {
    TESStesselator *tess = tessNewTess(NULL);

    // Flatten each path's beziers into a contour and add to tessellator
    for (NSVGpath *path = shape->paths; path; path = path->next) {
        int nsegs = (path->npts - 1) / 3;
        int max_pts = nsegs * 30 + 1;
        float *contour = malloc(max_pts * 2 * sizeof(float));
        int contour_count = 0;

        for (int j = 0; j < path->npts - 1; j += 3) {
            int count;
            flatten_cubic_bez(&path->pts[j * 2], 30,
                              &contour[contour_count * 2], &count, j != 0);
            contour_count += count;
        }

        tessAddContour(tess, 2, contour, sizeof(float) * 2, contour_count);
        free(contour);
    }

    // Tessellate into triangles
    int winding = shape->fillRule == NSVG_FILLRULE_EVENODD
        ? TESS_WINDING_ODD : TESS_WINDING_NONZERO;
    tessTesselate(tess, winding, TESS_POLYGONS, 3, 2, NULL);

    const float *verts = tessGetVertices(tess);
    const TESSindex *elems = tessGetElements(tess);
    int nelems = tessGetElementCount(tess);

    // Each element is a triangle (3 indices), output as 3 vertices × 2 floats
    int total_verts = nelems * 3;
    SevenSegmentSegment *segment = malloc(sizeof(SevenSegmentSegment) + total_verts * 2 * sizeof(float));
    segment->vertices_count = total_verts;

    for (int i = 0; i < nelems; i++) {
        for (int j = 0; j < 3; j++) {
            TESSindex idx = elems[i * 3 + j];
            segment->vertices[(i * 3 + j) * 2 + 0] = verts[idx * 2 + 0];
            segment->vertices[(i * 3 + j) * 2 + 1] = verts[idx * 2 + 1];
        }
    }

    tessDeleteTess(tess);

    return segment;
}

SevenSegmentShape preprocess_7_segment_svg(char *path) {
    struct NSVGimage *image = nsvgParseFromFile(path, "px", 96);
    SevenSegmentShape seven_segment_shape = {
        .segments = {NULL},
        .aspect_ratio = image->width / image->height,
    };

    int i = 0;

    struct NSVGshape *shape = image->shapes;
    while (shape != NULL) {
        assert(i < 7);

        seven_segment_shape.segments[i] = convert_shape_to_segment(shape);
        i += 1;

        shape = shape->next;
    }

    // Normalize all vertices to 0-1 range
    for (int j = 0; j < 7; j++) {
        if (!seven_segment_shape.segments[j]) break;
        SevenSegmentSegment *seg = seven_segment_shape.segments[j];
        for (int k = 0; k < seg->vertices_count; k++) {
            seg->vertices[k * 2 + 0] /= image->width;
            seg->vertices[k * 2 + 1] /= image->height;
        }
    }

    nsvgDelete(image);

    return seven_segment_shape;
}

SegmentMeshData build_segment_mesh(SevenSegmentShape *shape) {
    int total_floats = 0;
    for (int i = 0; i < 7; i++) {
        if (!shape->segments[i]) break;
        total_floats += shape->segments[i]->vertices_count * 2;
    }

    SegmentMeshData data = {
        .vertices = malloc(total_floats * sizeof(float)),
        .total_floats = total_floats,
    };

    int offset = 0;
    for (int i = 0; i < 7; i++) {
        if (!shape->segments[i]) break;
        SevenSegmentSegment *seg = shape->segments[i];
        int floats = seg->vertices_count * 2;

        data.segment_start[i] = offset / 2;
        data.segment_count[i] = seg->vertices_count;

        memcpy(&data.vertices[offset], seg->vertices, floats * sizeof(float));
        offset += floats;
    }

    return data;
}
