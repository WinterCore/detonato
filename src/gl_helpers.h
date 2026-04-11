#ifndef GL_HELPERS_H
#define GL_HELPERS_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define UNIT_RGB(r, g, b) ((float) r / 255.0), ((float) g / 255.0), ((float) b / 255.0)

GLFWwindow *create_window(int width, int height, const char *title);
GLuint create_shader_program(const char *vert_src, const char *frag_src);
GLuint load_shader_program(const char *vert_path, const char *frag_path);
typedef struct {
    GLuint vao;
    GLuint vbo;
} Mesh;

Mesh create_mesh(int stride);
void upload_vertices(Mesh mesh, float *vertices, int float_count);
void draw_range(Mesh mesh, int start, int count);

typedef enum Key {
    KEY_LEFT = 0,
    KEY_RIGHT,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_COUNT,
} Key;

bool is_key_pressed(GLFWwindow *window, Key key);

#endif
