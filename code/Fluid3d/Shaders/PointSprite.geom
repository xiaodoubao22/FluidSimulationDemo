#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float particalRadius;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

uniform mat4 view;
uniform mat4 projection;

out vec3 particalCenter;
out vec3 fragPosition;
out vec2 texCoordQuad;

void main() {
    particalCenter = gl_in[0].gl_Position.xyz;

    mat4 matVP = projection * view;

    // left down
    fragPosition = particalCenter - particalRadius * cameraRight - particalRadius * cameraUp;
    texCoordQuad = vec2(0, 0);
    gl_Position = matVP * vec4(fragPosition, 1.0);
    EmitVertex();

    // right down
    fragPosition = particalCenter + particalRadius * cameraRight - particalRadius * cameraUp;
    texCoordQuad = vec2(1, 0);
    gl_Position = matVP * vec4(fragPosition, 1.0);
    EmitVertex();

    // left up
    fragPosition = particalCenter - particalRadius * cameraRight + particalRadius * cameraUp;
    texCoordQuad = vec2(0, 1);
    gl_Position = matVP * vec4(fragPosition, 1.0);
    EmitVertex();

    // right up
    fragPosition = particalCenter + particalRadius * cameraRight + particalRadius * cameraUp;
    texCoordQuad = vec2(1, 1);
    gl_Position = matVP * vec4(fragPosition, 1.0);
    EmitVertex();
    EndPrimitive();

    return;
}