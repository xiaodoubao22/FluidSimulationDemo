#version 430 core
out vec4 FragColor;
	
in vec2 TexCoords;
	
uniform sampler2D tex;
	
void main()
{             
    vec3 texCol = texture(tex, TexCoords).rgb;      
    FragColor = vec4(abs(texCol), 1.0);
    // FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}