#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in float density;

uniform mat4 view;
uniform mat4 projection;

out vec3 color;

vec3 ValueToColor(float value) {
    vec3 color;
    if (value < 0.3333) {
        color.r = 0.0;
        color.g = 3.0 * value;
        color.b = 1.0;
    } 
    else if (value >= 0.3333 && value < 0.666) {
        color.r = 3.0 * value - 1.0;
        color.g = 1.0;
        color.b = -3.0 * value + 2.0;
    } 
    else {
        color.r = 1.0;
        color.g = -3.0 * value + 3.0;
        color.b = 0.0;
    }
    return color;
}

void main() {
    gl_PointSize = 4;
    gl_Position = projection * view * vec4(position, 1.0);

    color = ValueToColor((density - 500.0) / 2000.0);
}