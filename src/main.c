#include "gl_helpers.h"
#include "glad/gl.h"
#include "preprocess.h"
#include "render.h"
#include <GLFW/glfw3.h>

int main() {
    SegmentDigitShape shape = preprocess_segment_svg("assets/sports.svg");

    GLFWwindow *window = create_window(800, 600, "7 Segment Display");

    GLuint shader_program = load_shader_program("shaders/segment.vert", "shaders/segment.frag");
    GLint viewWidthLoc = glGetUniformLocation(shader_program, "viewWidth");
    GLint viewHeightLoc = glGetUniformLocation(shader_program, "viewHeight");
    GLint offsetLoc = glGetUniformLocation(shader_program, "offset");
    GLint scaleLoc = glGetUniformLocation(shader_program, "scale");
    GLint alphaLoc = glGetUniformLocation(shader_program, "alpha");
    GLint colorLoc = glGetUniformLocation(shader_program, "color");

    Mesh mesh = create_mesh(2); // x, y

    SegmentMeshData mesh_data = build_segment_mesh(&shape);

    int w, h;

    while (! glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, 1);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        upload_vertices(mesh, mesh_data.vertices, mesh_data.total_floats);

        DigitLayout layout = compute_layout("DD:DD.DDD", w, 100, 100, 50, 0.8, shape.aspect_ratio);

        for (int i = 0; i < layout.slot_count; i += 1) {
            glUniform1f(viewWidthLoc, (float) w);
            glUniform1f(viewHeightLoc, (float) h);
            glUniform2f(offsetLoc, layout.slots[i].x, (float) h / 2 - layout.digit_h / 2);
            glUniform2f(scaleLoc, layout.slots[i].w, layout.digit_h);

            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

            for (int i = 0; i < 7; i++) {
                glUniform1f(alphaLoc, 1);
                draw_range(mesh, mesh_data.segment_vertex_start[i], mesh_data.segment_vertex_count[i]);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    destroy_segment_shape(&shape);

    return 0;
}
