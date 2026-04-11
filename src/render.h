#ifndef RENDER_H
#define RENDER_H

#include "aids.h"
#include "gl_helpers.h"
#include "glad/gl.h"
#include "style.h"
#include <stdbool.h>
#include <stdint.h>

#define MAX_LAYOUT_SLOTS 16

typedef enum SlotKind {
    SLOT_DIGIT,
    SLOT_COLON,
    SLOT_DOT,
} SlotKind;

typedef struct LayoutSlot {
    float x;
    float w;
    SlotKind kind;
} LayoutSlot;

typedef struct DigitLayout {
    LayoutSlot slots[MAX_LAYOUT_SLOTS];
    int slot_count;
    float digit_w;
    float digit_h;
    float narrow_w;
} DigitLayout;

typedef enum DurationFormat {
    FMT_MM_SS,
    FMT_HH_MM_SS,
    FMT_HH_MM_SS_MS,
} DurationFormat;

DigitLayout compute_layout(
    DurationFormat fmt,
    int vw,
    float pad_l, float pad_r,
    float gap,
    float narrow_ratio,
    float aspect_ratio
);

void format_duration(DurationFormat fmt, int64_t ms, uint8_t *digits_out);

typedef struct RenderContext {
    GLint uniformOffset, uniformScale, uniformAlpha, uniformColor;
    DigitStyle *style;
    Mesh mesh;
} RenderContext;

void render_digit(
    RenderContext *ctx,
    Vec2 *offset,
    Vec2 *scale,
    Vec3 *color,
    float *alpha,
    uint8_t render_mask,
    int digit
);

#endif
