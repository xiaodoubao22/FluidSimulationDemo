#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 fragTexCoord;
out vec3 fragPosition;
out vec3 viewPosition;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);

    fragPosition = (model * vec4(position, 1.0)).xyz;
    fragTexCoord = texCoord;
    viewPosition = (inverse(view) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
}