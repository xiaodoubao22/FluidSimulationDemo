#version 430 core
out vec4 FragColor;
	
in vec2 TexCoords;
	
uniform sampler2D tex;
	
void main()
{	
    vec4 texCol = texture(tex, TexCoords);
    FragColor = vec4(vec3(texCol.r), 1.0);
}