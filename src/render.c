#include "render.h"
#include "aids.h"
#include "gl_helpers.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

DigitLayout compute_layout(
    DurationFormat format,
    int vw,
    float pad_l, float pad_r,
    float gap_width_pct,
    float narrow_ratio,
    float aspect_ratio
) {
    char *pattern;

    switch (format) {
        case FMT_MM_SS:
            pattern = "DD:DD";
            break;
        case FMT_HH_MM_SS:
            pattern = "DD:DD:DD";
            break;
        case FMT_HH_MM_SS_MS:
            pattern = "DD:DD:DD.DDD";
            break;
        default:
            PANIC("Unsupported format");
    }

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

    float gap = gap_width_pct * vw;

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

void format_duration(DurationFormat fmt, int64_t ms, uint8_t *digits_out) {
    int64_t total_s = ms / 1000;
    int sub_ms = ms % 1000;
    int ss = total_s % 60;
    int total_m = total_s / 60;
    int mm = total_m % 60;
    int total_h = total_m / 60;
    int hh = total_h % 24;

    if (fmt == FMT_MM_SS) {
        digits_out[0] = mm / 10;
        digits_out[1] = mm % 10;
        digits_out[2] = ss / 10;
        digits_out[3] = ss % 10;

        return;
    }

    if (fmt == FMT_HH_MM_SS) {
        digits_out[0] = hh / 10;
        digits_out[1] = hh % 10;
        digits_out[2] = mm / 10;
        digits_out[3] = mm % 10;
        digits_out[4] = ss / 10;
        digits_out[5] = ss % 10;

        return;
    }

    if (fmt == FMT_HH_MM_SS_MS) {
        digits_out[0] = hh / 10;
        digits_out[1] = hh % 10;
        digits_out[2] = mm / 10;
        digits_out[3] = mm % 10;
        digits_out[4] = ss / 10;
        digits_out[5] = ss % 10;
        digits_out[6] = sub_ms / 100;
        digits_out[7] = (sub_ms / 10) % 10;
        digits_out[8] = sub_ms % 10;

        return;
    }

    UNIMPLEMENTED
}

void render_digit(
    RenderContext *ctx,
    Vec2 *offset,
    Vec2 *scale,
    Vec3 *color,
    float *alpha,
    uint8_t render_mask, // 2 bits: unlit,lit
    int digit
) {
    glUniform2f(ctx->uniformOffset, offset->x, offset->y);
    glUniform2f(ctx->uniformScale, scale->x, scale->y);

    glUniform3f(ctx->uniformColor, UNIT_RGB(color->x, color->y, color->z));

    for (int i = 0; i < ctx->style->segment_count; i++) {
        bool lit = is_segment_visible(ctx->style, digit, i);

        if ((lit && (render_mask >> 0 & 0b1) == 0) ||
            (!lit && (render_mask >> 1 & 0b1) == 0)) {
            continue;
        }
        
        float a = lit ? 1.0f : 0.3f;

        if (alpha != NULL) {
            a = *alpha;
        }

        glUniform1f(ctx->uniformAlpha, a);
        draw_range(
            ctx->mesh,
            ctx->style->segment_vertex_start[i],
            ctx->style->segment_vertex_count[i]
        );
    }
}
