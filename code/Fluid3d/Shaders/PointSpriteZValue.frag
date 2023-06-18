#version 450

in vec3 particalCenter;
in vec3 fragPosition;
in vec2 texCoordQuad;

out vec4 fragColor;

uniform float particalRadius;
uniform float zNear;
uniform float zFar;

float DepthToZ(float depth) { 
    return - zFar * zNear / (zNear * depth + zFar * (1.0 - depth));
}

float ZToDepth(in float z) {
    z = abs(z);
    return zFar * (zNear - z) / (z * (zNear - zFar));
}

void main() {

    float dist = distance(fragPosition, particalCenter);
    if (dist > particalRadius) {
        discard;
    }

    float deltaDepthNorm = 2.0 * sqrt(0.5 * 0.5 - pow(texCoordQuad.x - 0.5, 2) - pow(texCoordQuad.y - 0.5, 2) + 1e-5);
    float ZValue = DepthToZ(gl_FragCoord.z);
    ZValue += particalRadius * deltaDepthNorm;

    gl_FragDepth = ZToDepth(ZValue);

    fragColor = vec4(ZValue, 0.0, 0.0, 0.0);
    return;
}