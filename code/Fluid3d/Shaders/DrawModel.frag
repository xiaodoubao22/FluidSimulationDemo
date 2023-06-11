#version 450

in vec2 oTexCoord;

out vec4 FragColor;

layout(binding=0) uniform sampler2D texAlbedo;

void main() {
    vec3 albedo = texture(texAlbedo, oTexCoord).xyz;
    FragColor = vec4(albedo, 1.0);
}