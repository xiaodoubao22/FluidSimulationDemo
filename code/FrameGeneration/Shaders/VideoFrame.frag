#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D videoTexture;
void main()
{
    FragColor = texture(videoTexture, TexCoord);
}
