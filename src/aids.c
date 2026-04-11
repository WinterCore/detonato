#include "aids.h"

float clamp(float min, float max, float v) {
  const float t = v < min ? min : v;
  return t > max ? max : t;
}

float lerp(float a, float b, float v) {
    return a + v * (b - a);
}

float remap(float source_a, float source_b, float dest_a, float dest_b, float value) {
    return dest_a + (value - source_a) * (dest_b - dest_a) / (source_b - source_a);
}
