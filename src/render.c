#include "render.h"
#include "aids.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

DigitLayout compute_layout(
    const char *pattern,
    int vw,
    float pad_l, float pad_r,
    float gap,
    float narrow_ratio,
    float aspect_ratio
) {
    int pattern_len = strlen(pattern);
    assert(0 < pattern_len && pattern_len <= MAX_LAYOUT_SLOTS);

    DigitLayout layout = { };

    int digit_count = 0, narrow_count = 0;

    for (int i = 0; i < pattern_len; i += 1) {
        switch (pattern[i]) {
            case 'D':
                digit_count += 1;
                break;
            case ':':
            case '.':
                narrow_count += 1;
                break;
            default:
                PANIC("Unsupported pattern char %c", pattern[i]);
        }
    }

    float gap_widths = (digit_count + narrow_count - 1) * gap;
    float working_width = vw - pad_l - pad_r - gap_widths;

    // width = (digit_width * digits_count) + (narrow_count * digit_width * narrow_ratio)
    // width = digit_width * (digits_count + (narrow_count * narrow_ratio))
    // digit_width = width / (digits_count + (narrow_count * narrow_ratio))
    float digit_width = working_width / (digit_count + narrow_count * narrow_ratio);
    float narrow_width = digit_width * narrow_ratio;

    layout.digit_w = digit_width;
    layout.digit_h = digit_width / aspect_ratio;
    layout.narrow_w = narrow_width;
    layout.slot_count = digit_count + narrow_count;

    float x = pad_l;

    // Build slots
    for (int i = 0; i < pattern_len; i += 1) {
        switch (pattern[i]) {
            case 'D':
                layout.slots[i] = (LayoutSlot) {
                    .x = x,
                    .w = digit_width,
                    .kind = SLOT_DIGIT,
                };
                
                x += digit_width + gap;
                break;
            case ':':
                layout.slots[i] = (LayoutSlot) {
                    .x = x,
                    .w = narrow_width,
                    .kind = SLOT_COLON,
                };

                x += narrow_width + gap;
                break;
            case '.':
                layout.slots[i] = (LayoutSlot) {
                    .x = x,
                    .w = narrow_width,
                    .kind = SLOT_DOT,
                };

                x += narrow_width + gap;
                break;
            default:
                PANIC("Unsupported pattern char %c", pattern[i]);
        }
    }

    return layout;
}


bool should_render_segment(uint64_t bitmask, uint8_t index) {
    return (bitmask >> index) & 0b1;
}
