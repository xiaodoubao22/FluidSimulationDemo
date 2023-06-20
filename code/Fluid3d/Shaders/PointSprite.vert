#version 450

layout(location = 0) in vec3 pointPosition;

void main() {
    gl_Position = vec4(pointPosition, 1.0);
    return;
}