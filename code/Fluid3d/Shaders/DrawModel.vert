#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

uniform mat4 view;
uniform mat4 projection;

out vec2 fragTexCoord;
out vec3 fragPosition;

void main() {
    gl_Position = projection * view * vec4(position, 1.0);

    fragPosition = position;
    fragTexCoord = texCoord;
}