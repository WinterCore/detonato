#version 330 core
layout (location = 0) in vec2 aPos;

uniform float viewWidth;
uniform float viewHeight;
uniform vec2 offset;
uniform float scale;

void main() {
    vec2 pos = aPos * scale + offset;
    float x = pos.x / viewWidth * 2.0 - 1.0;
    float y = -(pos.y / viewHeight * 2.0 - 1.0);
    gl_Position = vec4(x, y, 0.0, 1.0);
}
