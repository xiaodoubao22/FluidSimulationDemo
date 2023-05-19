
#version 450

in float densNorm;

out vec4 FragColor;

void main() {
    vec3 color;
    if (densNorm < 0.3333) {
        color.r = 0.0;
        color.g = 3.0 * densNorm;
        color.b = 1.0;
    } 
    else if (densNorm >= 0.3333 && densNorm < 0.666) {
        color.r = 3.0 * densNorm - 1.0;
        color.g = 1.0;
        color.b = -3.0 * densNorm + 2.0;
    } 
    else {
        color.r = 1.0;
        color.g = -3.0 * densNorm + 3.0;
        color.b = 0.0;
    }

    FragColor = vec4(color, 1.0);
}
