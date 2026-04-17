#include "gl_helpers.h"
#include "glad/gl.h"
#include "render.h"
#include "style.h"
#include <GLFW/glfw3.h>
#include <time.h>
#include <time.h>

const char *STYLES[] = {
    "double-bubble",
    "sports",
    "x-black"
};

int main(int argc, char *argv[]) {
    char *style_name = "sports";

    if (argc > 1) {
        style_name = argv[1];
    }

    GLFWwindow *window = create_window(800, 600, "Detonato");

    GLuint shader_program = load_shader_program("shaders/segment.vert", "shaders/segment.frag");
    GLint viewWidthLoc = glGetUniformLocation(shader_program, "viewWidth");
    GLint viewHeightLoc = glGetUniformLocation(shader_program, "viewHeight");
    GLint offsetLoc = glGetUniformLocation(shader_program, "offset");
    GLint scaleLoc = glGetUniformLocation(shader_program, "scale");
    GLint alphaLoc = glGetUniformLocation(shader_program, "alpha");
    GLint colorLoc = glGetUniformLocation(shader_program, "color");

    Mesh mesh = create_mesh(2); // x, y
    
    DigitStyle style;
    load_digit_style(style_name, &style);

    RenderContext render_ctx = {
        .uniformAlpha = alphaLoc,
        .uniformColor = colorLoc,
        .uniformOffset = offsetLoc,
        .uniformScale = scaleLoc,
        .style = &style,
        .mesh = mesh,
    };
    

    int w, h;

    while (! glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, 1);
        }

        glClearColor(UNIT_RGB(104, 162, 88), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 104, 162, 88
        glUseProgram(shader_program);

        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        upload_vertices(mesh, style.vertices, style.total_floats);

        DurationFormat format = FMT_HH_MM_SS;

        DigitLayout layout = compute_layout(format, w, 100, 100, 0.02, 0.4, style.aspect_ratio);

        glUniform1f(viewWidthLoc, (float) w);
        glUniform1f(viewHeightLoc, (float) h);

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        long milliseconds = ts.tv_sec * 1000L + ts.tv_nsec / 1000000;

        uint8_t digits[10];
        format_duration(format, milliseconds, digits);

        int digit_count = 0;

        for (int i = 0; i < layout.slot_count; i += 1) {
            if (layout.slots[i].kind != SLOT_DIGIT) {
                continue;
            }

            float x = layout.slots[i].x;
            float y = (float) h / 2 - layout.digit_h / 2;

            {
                // Pass 1: Render unlint segments
                Vec2 offset = { .x = x, .y = y };

                Vec2 scale = { .x = layout.slots[i].w, .y = layout.digit_h };

                Vec3 color = { 23, 34, 19 };

                float alpha = 0.3f;

                render_digit(
                    &render_ctx,
                    &offset,
                    &scale,
                    &color,
                    &alpha,
                    0b10,
                    digits[digit_count]
                );
            }

            {
                // Pass 2: Wanna be drop shadow
                Vec2 offsets[8] = {
                    (Vec2) { x + 10, y + 1 },
                    (Vec2) { x + 8, y + 3 },
                    (Vec2) { x + 6, y + 6 },
                    (Vec2) { x + 4, y + 8 },
                    (Vec2) { x + 4, y + 1 },
                    (Vec2) { x + 6, y + 3 },
                    (Vec2) { x + 8, y + 6 },
                    (Vec2) { x + 10, y + 8 },
                };

                for (int j = 0; j < 8; j += 1) {
                    Vec2 offset = offsets[j];

                    Vec2 scale = {
                        .x = layout.slots[i].w,
                        .y = layout.digit_h,
                    };

                    Vec3 color = {23, 34, 19};

                    float alpha = 0.28f;

                    render_digit(
                        &render_ctx,
                        &offset,
                        &scale,
                        &color,
                        &alpha,
                        0b01,
                        digits[digit_count]
                    );
                }
            }

            {
                // Pass 3: Render lit segments
                Vec2 offset = {
                    .x = layout.slots[i].x,
                    .y = (float) h / 2 - layout.digit_h / 2,
                };

                Vec2 scale = {
                    .x = layout.slots[i].w,
                    .y = layout.digit_h,
                };

                Vec3 color = {23, 34, 19};

                float alpha = 1.0f;

                render_digit(
                    &render_ctx,
                    &offset,
                    &scale,
                    &color,
                    &alpha,
                    0b01,
                    digits[digit_count]
                );
            }

            digit_count += 1;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    destroy_digit_style(&style);

    return 0;
}
