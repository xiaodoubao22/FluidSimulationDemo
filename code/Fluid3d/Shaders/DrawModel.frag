#version 450

in vec2 fragTexCoord;
in vec3 fragPosition;
in vec3 viewPosition;

out vec4 FragColor;

layout(binding=0) uniform sampler2D shadowMap;
layout(binding=1) uniform sampler2D causticMap;
layout(binding=2) uniform samplerCube skybox;
layout(binding=3) uniform sampler2D texAlbedo;
layout(binding=4) uniform sampler2D texRoughness;

uniform mat4 lightView;
uniform mat4 lightProjection;

const vec3 shadowColor = 0.5 * vec3(0.1, 0.5, 1.0);

vec3 lightColor = vec3(1.0);
vec3 Normal = vec3(0.0, 0.0, 1.0);
vec3 PhongShading(vec3 albedo, float roughness, vec3 lightPos, vec3 fragPos, vec3 viewPos) {
    // ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    float defuseStrength = 3.0;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = roughness * defuseStrength * diff * lightColor;
    
    // specular
    float specularStrength = 0.3;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 4);
    vec3 specular = (1.0 - roughness) * specularStrength * spec * lightColor;  
        
    return (ambient + diffuse + specular) * albedo;
}

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
    vec2 texCoord = NdcToTexCoord(fragNDC.xy);

    // PCF法计算阴影
    float fragDist = distance(lightPos, curPosition);
    float shadowFactor = 0.2 * Pcf(texCoord, fragDist);
    vec3 colorWithShadow = mix(originColor, shadowColor, shadowFactor);

    // 添加焦散
    vec3 caustic = texture(causticMap, texCoord).xyz;

    return colorWithShadow + caustic;
}

void main() {
    vec4 lightPos = inverse(lightView) * vec4(0.0, 0.0, 0.0, 1.0);
    vec3 albedo = texture(texAlbedo, fragTexCoord).rgb;
    float roughness = texture(texRoughness, fragTexCoord).r;
    vec3 originColor = PhongShading(albedo, roughness, lightPos.xyz, fragPosition, viewPosition);

    FragColor = vec4(ShadeFloorWithShadow(originColor, lightPos.xyz, fragPosition), 1.0);
}