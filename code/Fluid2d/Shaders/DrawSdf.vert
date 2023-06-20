
#version 450

layout(location = 0) in vec2 pointPosition;
layout(location = 1) in float density;

void main() {

    gl_Position = vec4(pointPosition, 0.0, 1.0);
}
