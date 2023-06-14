#version 450

in vec2 fragTexCoord;
in vec3 fragPosition;

out vec4 FragColor;

layout(binding=0) uniform sampler2D texAlbedo;
layout(binding=1) uniform sampler2D shadowMap;

uniform mat4 lightView;
uniform mat4 lightProjection;

const vec3 shadowColor = 0.5 * vec3(0.1, 0.5, 1.0);

vec2 NdcToTexCoord(vec2 NdcXy) {
    // [-1, 1] -> [0, 1]
    return NdcXy * 0.5 + vec2(0.5);
}

float Pcf(vec2 texCoord, float fragDist) {
    float res = 0.0;
    for(int i = -2; i <= 2; i++) {
        for(int j = -2; j <= 2; j++) {
            vec2 tc = texCoord + 2e-3 * vec2(i, j);
            float shadowDist = texture(shadowMap, tc).r;
            if (shadowDist < 0 && abs(shadowDist) < fragDist) {
                res += 1.0;
            }
        }
    }
    return res / 25.0;
}

vec3 ShadeFloorWithShadow(vec3 originColor, vec3 lightPos, vec3 curPosition) {
    // 投影到光源，取纹理坐标
    vec4 fragNDC = lightProjection * lightView * vec4(curPosition, 1.0);
    fragNDC /= fragNDC.w;
    vec2 shadowTexCoord = NdcToTexCoord(fragNDC.xy);

    // PCF法计算阴影
    float fragDist = distance(lightPos, curPosition);
    float shadowFactor = 0.2 * Pcf(shadowTexCoord, fragDist);
    return mix(originColor, shadowColor, shadowFactor);
}

void main() {
    vec4 lightPos = inverse(lightView) * vec4(0.0, 0.0, 0.0, 1.0);
    vec3 albedo = texture(texAlbedo, fragTexCoord).xyz;
    FragColor = vec4(ShadeFloorWithShadow(albedo, lightPos.xyz, fragPosition), 1.0);
}