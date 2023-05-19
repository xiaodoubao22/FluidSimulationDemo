
#version 450

layout(location = 0) in vec2 pointPosition;
layout(location = 1) in float density;

out float densNorm;

void main() {

    gl_PointSize = 3;
    gl_Position = vec4(pointPosition, 0.0, 1.0);

    densNorm = density / 4000.0f;
}
