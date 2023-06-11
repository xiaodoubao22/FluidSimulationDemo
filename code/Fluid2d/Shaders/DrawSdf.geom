
#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

float radius = 0.03;
out vec2 particalCenter;
void main() {
    particalCenter = vec2(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y);
    // left down
    gl_Position = gl_in[0].gl_Position + vec4(-radius, -radius, 0.0, 0.0);
    EmitVertex();

    // right down
    gl_Position = gl_in[0].gl_Position + vec4(radius, -radius, 0.0, 0.0);
    EmitVertex();

    // left up
    gl_Position = gl_in[0].gl_Position + vec4(-radius, radius, 0.0, 0.0);
    EmitVertex();

    // right up
    gl_Position = gl_in[0].gl_Position + vec4(radius, radius, 0.0, 0.0);
    EmitVertex();
    EndPrimitive();
}