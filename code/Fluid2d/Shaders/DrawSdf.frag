
#version 450

out vec4 FragColor;

in vec2 particalCenter;

float ScreenWidth = 1000.0;
float ScreenHeight = 1000.0;
float radius = 0.03;

vec4 gBlack = vec4(0.0, 0.0, 0.0, 1.0);
vec4 gWhite = vec4(1.0, 1.0, 1.0, 1.0);
float gEdgeThreShold = 0.8;

void main() {
    vec2 pixelCoordNDC = vec2(gl_FragCoord.x * 2.0 / ScreenWidth - 1.0, gl_FragCoord.y * 2.0 / ScreenHeight - 1.0);
    float dist = distance(pixelCoordNDC, particalCenter);
    gl_FragDepth = dist / radius;

    if (gl_FragDepth < gEdgeThreShold) {
        FragColor = gBlack;
    }
    else {
        FragColor = gWhite;
    }
}
