#version 430 core
vec2 ScreenQuadPositions[4] = {
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
};

vec2 ScreenQuadTexCoords[4] = {
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
};
    
out vec2 texCoord;
    
void main()
{
    texCoord = ScreenQuadTexCoords[gl_VertexID];
    gl_Position = vec4(ScreenQuadPositions[gl_VertexID], 0.0, 1.0);
}