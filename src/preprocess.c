#include "aids.h"
#include "gl_helpers.h"
#include "render.h"
#include "style.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
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

static DigitSegment *convert_shape_to_segment(NSVGshape *shape) {
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
    DigitSegment *segment = malloc(sizeof(DigitSegment) + total_verts * 2 * sizeof(float));
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

SegmentDigitShape preprocess_segment_svg(char *path) {
    struct NSVGimage *image = nsvgParseFromFile(path, "px", 96);
    SegmentDigitShape seven_segment_shape = {
        .segments = {NULL},
        .aspect_ratio = image->width / image->height,
    };

    int i = 0;

    struct NSVGshape *shape = image->shapes;
    while (shape != NULL) {
        seven_segment_shape.segments[i] = convert_shape_to_segment(shape);
        i += 1;

        shape = shape->next;
    }

    // Normalize all vertices to 0-1 range
    for (int j = 0; j < i; j++) {
        if (!seven_segment_shape.segments[j]) break;
        DigitSegment *seg = seven_segment_shape.segments[j];
        for (int k = 0; k < seg->vertices_count; k++) {
            seg->vertices[k * 2 + 0] /= image->width;
            seg->vertices[k * 2 + 1] /= image->height;
        }
    }

    nsvgDelete(image);

    seven_segment_shape.segment_count = i;

    return seven_segment_shape;
}

void destroy_segment_shape(SegmentDigitShape *shape) {
    for (int i = 0; i < shape->segment_count; i++) {
        free(shape->segments[i]);
        shape->segments[i] = NULL;
    }
}

DigitStyle build_segment_mesh(SegmentDigitShape *shape) {
    int total_floats = 0;
    for (int i = 0; i < shape->segment_count; i++) {
        if (!shape->segments[i]) break;
        total_floats += shape->segments[i]->vertices_count * 2;
    }

    DigitStyle data = {
        .vertices = malloc(total_floats * sizeof(float)),
        .total_floats = total_floats,
        .segment_count = shape->segment_count,
        .aspect_ratio = shape->aspect_ratio,
    };

    int offset = 0;
    for (int i = 0; i < shape->segment_count; i++) {
        if (!shape->segments[i]) break;
        DigitSegment *seg = shape->segments[i];
        int floats = seg->vertices_count * 2;

        data.segment_vertex_start[i] = offset / 2;
        data.segment_vertex_count[i] = seg->vertices_count;

        memcpy(&data.vertices[offset], seg->vertices, floats * sizeof(float));
        offset += floats;
    }

    return data;
}

int main() {
    char *name = "sports";

    char svg_path[100];
    sprintf(svg_path, "assets/%s.svg", name);
    SegmentDigitShape shape = preprocess_segment_svg(svg_path);

    GLFWwindow *window = create_window(800, 600, "7 Segment Display");

    GLuint shader_program = load_shader_program("shaders/segment.vert", "shaders/segment.frag");
    GLint viewWidthLoc = glGetUniformLocation(shader_program, "viewWidth");
    GLint viewHeightLoc = glGetUniformLocation(shader_program, "viewHeight");
    GLint offsetLoc = glGetUniformLocation(shader_program, "offset");
    GLint scaleLoc = glGetUniformLocation(shader_program, "scale");
    GLint alphaLoc = glGetUniformLocation(shader_program, "alpha");
    GLint colorLoc = glGetUniformLocation(shader_program, "color");

    Mesh mesh = create_mesh(2); // x, y

    DigitStyle style = build_segment_mesh(&shape);
    style.segment_bitmask[0] = 0b1110111;
    style.segment_bitmask[1] = 0b0100100;
    style.segment_bitmask[2] = 0b1011101;
    style.segment_bitmask[3] = 0b1101101;
    style.segment_bitmask[4] = 0b0101110;
    style.segment_bitmask[5] = 0b1101011;
    style.segment_bitmask[6] = 0b1111011;
    style.segment_bitmask[7] = 0b0100101;
    style.segment_bitmask[8] = 0b1111111;
    style.segment_bitmask[9] = 0b1101111;


    int w, h;

    int digit = 0;

    while (! glfwWindowShouldClose(window)) {
        if (is_key_pressed(window, KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, 1);
        }

        if (is_key_pressed(window, KEY_LEFT)) {
            digit = digit == 0 ? 9 : digit - 1;
        }

        if (is_key_pressed(window, KEY_RIGHT)) {
            digit = (digit + 1) % 10;
        }

        if (is_key_pressed(window, KEY_ENTER)) {
            write_digit_style(name, &style);
            printf("Wrote styles/%s.bin", name);

            break;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        upload_vertices(mesh, style.vertices, style.total_floats);
        
        float half_width = (float) w / 2;
        float half_height = (float) h / 2;

        float digit_width = 1000;
        float digit_height = 1000 / shape.aspect_ratio;

        float scale = MIN(half_width / digit_width, (half_height * 1.5f) / digit_height);

        digit_width *= scale;
        digit_height *= scale;

        glUniform1f(viewWidthLoc, (float) w);
        glUniform1f(viewHeightLoc, (float) h);
        glUniform2f(offsetLoc, half_width - digit_width / 2, half_height - digit_height / 2);
        glUniform2f(scaleLoc, digit_width, digit_height);

        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

        for (int i = 0; i < style.segment_count; i++) {
            if (should_render_segment(style.segment_bitmask[digit], i)) {
                glUniform1f(alphaLoc, 1);
                draw_range(mesh, style.segment_vertex_start[i], style.segment_vertex_count[i]);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    destroy_segment_shape(&shape);

    return 0;
}
