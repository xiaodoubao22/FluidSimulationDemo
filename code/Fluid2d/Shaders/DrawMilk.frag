
#version 450

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D textureSdf;

vec4 gMilkColor = vec4(1.0, 1.0, 0.9, 1.0);
vec4 gBakGroundColor = vec4(0.5, 0.8, 1.0, 1.0);
ivec2 gKernelSize = ivec2(11, 11);

float gScreenWidth = 1000.0;
float gScreenHeight = 1000.0;

void main() {
    
    ivec2 halfKernelSize = gKernelSize / 2;
    vec4 sumColor = vec4(0.0, 0.0, 0.0, 0.0);

    for (int i = -halfKernelSize.x; i < gKernelSize.x - halfKernelSize.x; i++) {
        for(int j = -halfKernelSize.y; j < gKernelSize.y - halfKernelSize.y; j++) {
            float x = i / gScreenWidth * 2.0;
            float y = j / gScreenHeight * 2.0;
            sumColor += texture(textureSdf, texCoord + vec2(x, y));
        }
    }
    FragColor = sumColor / (gKernelSize.x * gKernelSize.y);

    if (FragColor.x < 0.5) {
        FragColor = gMilkColor;
    }
    else {
        FragColor = gBakGroundColor;
    }
}
