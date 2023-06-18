#version 430 core
out vec4 FragColor;
	
in vec2 TexCoords;
	
uniform sampler2D tex;
	
void main()
{	
    vec4 texCol = texture(tex, TexCoords);
//	if(texCol.x > 0) {
//		discard;
//	}
    FragColor = vec4(vec3(texCol.rgb), 1.0);
}