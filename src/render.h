#ifndef RENDER_H
#define RENDER_H

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

DigitLayout compute_layout(
    const char *pattern,
    int vw,
    float pad_l, float pad_r,
    float gap,
    float narrow_ratio,
    float aspect_ratio
);

bool should_render_segment(uint64_t bitmask, uint8_t index);

#endif
