#version 450

out vec4 fragColor;

uniform vec3 photonColor;

void main() {
    fragColor = vec4(photonColor, 1.0);
}