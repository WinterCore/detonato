#include "gl_helpers.h"
#include "aids.h"
#include "glad/gl.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

GLFWwindow *create_window(int width, int height, const char *title) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return NULL;
    }

    glfwSwapInterval(1);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    return window;
}

static GLuint compile_shader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compilation failed: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint create_shader_program(const char *vert_src, const char *frag_src) {
    GLuint vert = compile_shader(GL_VERTEX_SHADER, vert_src);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    if (!vert || !frag) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Shader linking failed: %s\n", log);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;
}

static char *read_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    return buf;
}

GLuint load_shader_program(const char *vert_path, const char *frag_path) {
    char *vert_src = read_file(vert_path);
    char *frag_src = read_file(frag_path);
    if (!vert_src || !frag_src) {
        free(vert_src);
        free(frag_src);
        return 0;
    }

    GLuint program = create_shader_program(vert_src, frag_src);
    free(vert_src);
    free(frag_src);

    return program;
}

Mesh create_mesh(int stride) {
    Mesh mesh;
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    int offset = 0;
    int bytes = stride * sizeof(float);

    // position: first 2 floats
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, bytes, (void *)(intptr_t)offset);
    glEnableVertexAttribArray(0);
    offset += 2 * sizeof(float);

    // color: next (stride - 2) floats (e.g. r, g, b, a)
    if (stride > 2) {
        glVertexAttribPointer(1, stride - 2, GL_FLOAT, GL_FALSE, bytes, (void *)(intptr_t)offset);
        glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);

    return mesh;
}

void upload_vertices(Mesh mesh, float *vertices, int float_count) {
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, float_count * sizeof(float), vertices, GL_DYNAMIC_DRAW);
}

void draw_range(Mesh mesh, int start, int count) {
    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_TRIANGLES, start, count);
    glBindVertexArray(0);
}

static int map_key_to_glfw_key(Key key) {
    switch (key) {
        case KEY_ENTER:
            return GLFW_KEY_ENTER;
        case KEY_LEFT:
            return GLFW_KEY_LEFT;
        case KEY_RIGHT:
            return GLFW_KEY_RIGHT;
        case KEY_ESCAPE:
            return GLFW_KEY_ESCAPE;
        case KEY_COUNT:
            UNREACHABLE;
    }

    UNREACHABLE
}

uint8_t key_state[KEY_COUNT] = {0};

bool is_key_pressed(GLFWwindow *window, Key key) {
    if (glfwGetKey(window, map_key_to_glfw_key(key)) == GLFW_PRESS) {
        key_state[key] = true;

        return false;
    }

    if (glfwGetKey(window, map_key_to_glfw_key(key)) == GLFW_RELEASE && key_state[key]) {
        key_state[key] = false;

        return true;
    }

    return false;
}
