#version 450

uniform float particalRadius;
uniform vec3 cameraFront;
uniform mat4 view;
uniform mat4 projection;

in vec3 particalCenter;
in vec3 fragPosition;
in vec2 texCoordQuad;

out vec4 fragColor;

float invFar = 1.0 / 100.0;
float invNear = 1.0 / 0.1;

float DepthToZ(float depth) { 
    return - 1.0 / (depth * (invFar - invNear) + invNear);
}

float ZToDepth(float z) {
    return ((1 / abs(z)) - invNear) / (invFar - invNear);
}

void main() {

    float dist = distance(fragPosition, particalCenter);
    if (dist > particalRadius) {
        discard;
    }

    float deltaDepthNorm = 2.0 * sqrt(0.5 * 0.5 - pow(texCoordQuad.x - 0.5, 2) - pow(texCoordQuad.y - 0.5, 2) + 1e-5);
    float thickness = 2.0 * particalRadius * deltaDepthNorm;

    fragColor = vec4(thickness, 0.0, 0.0, 0.0);
    return;
}