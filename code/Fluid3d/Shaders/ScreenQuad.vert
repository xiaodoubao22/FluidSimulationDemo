#version 430 core
vec2 positions[4] = {
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
};

vec2 texCoords[4] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
};

uniform vec3 testData;
	
out vec2 TexCoords;
	
void main()
{
    TexCoords = texCoords[gl_VertexID];
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}