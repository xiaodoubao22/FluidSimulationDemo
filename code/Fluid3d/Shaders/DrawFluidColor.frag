#version 430 core

// ----------const----------
const float EPS = 1e-5;
const float FLT_MAX = 1e+10;

uniform vec4 cameraIntrinsic;  // vec4(fxInv, fyInv, cx, cy)
uniform float zNear;
uniform float zFar;
uniform mat4 model;

uniform float eta;
uniform vec3 f0;
uniform vec3 fluidColor;
uniform vec3 shadowColor;
uniform float thicknessFactor;

// ----------define----------
struct Vertex {
    vec3 position;
    vec2 texCoord;
};

struct Triangle {
    vec3 pos0;
    vec2 texCoord0;
    vec3 pos1;
    vec2 texCoord1;
    vec3 pos2;
    vec2 texCoord2;
};

struct HitResult {
    int isHit;
    float dist;
    vec3 hitPoint;
    vec3 normal;
    vec2 texCoord;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

// ----------in out uniform----------
in vec2 texCoord;
out vec4 FragColor;

uniform mat4 camToWorld;
uniform mat4 camToWorldRot;
uniform mat4 projection;

uniform mat4 lightView;
uniform mat4 lightProjection;

// ----------buffers----------
layout(r32f, binding = 0) uniform image2D zBuffer;
layout(r32f, binding = 1) uniform image2D thicknessBuffer;
// layout(std430, binding=0) buffer Triangles
// {
//     Triangle triangles[];
// };
layout(binding=0) uniform samplerCube skybox;
layout(binding=1) uniform sampler2D shadowMap;
layout(binding=2) uniform sampler2D causticMap;
layout(binding=3) uniform sampler2D texAlbedo;
layout(binding=4) uniform sampler2D texRoughness;

Triangle T0 = {
    vec3(1.0,  1.0, 0.0),
    vec2(1.0, 1.0),
    vec3(-1.0,  1.0, 0.0),
    vec2(0.0, 1.0),
    vec3(-1.0, -1.0, 0.0),
    vec2(0.0, 0.0)
};
Triangle T1 = {
    vec3(1.0,  1.0, 0.0),
    vec2(1.0, 1.0),
    vec3(-1.0, -1.0, 0.0),
    vec2(0.0, 0.0),
    vec3(1.0, -1.0, 0.0),
    vec2(1.0, 0.0)
};

Triangle triangles[2] = {
    T0, T1
};

// ----------functinons----------
vec3 FresnelSchlic(vec3 wi, vec3 wh) {
    float cosTheta = abs(dot(wh, wi));
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 Reproject(float depth, ivec2 imageCoord) { 
    // cameraIntrinsic = vec4(fxInv, fyInv, cx, cy)
    float x = abs(depth) * (float(imageCoord.x) - cameraIntrinsic.z) * cameraIntrinsic.x;
    float y = abs(depth) * (float(imageCoord.y) - cameraIntrinsic.w) * cameraIntrinsic.y;   
    return vec3(x, y, depth);
}

float ZToDepth(in float z) {
    z = abs(z);
    return zFar * (zNear - z) / (z * (zNear - zFar));
}

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

vec3 CalculateNormal(ivec2 curPixelId, float curDepth, vec3 curPos) {
    float upDepth = imageLoad(zBuffer, curPixelId + ivec2(0, 1) * 2).x;
    float downDepth = imageLoad(zBuffer, curPixelId + ivec2(0, -1) * 2).x;
    float leftDepth = imageLoad(zBuffer, curPixelId + ivec2(-1, 0) * 2).x;
    float rightDepth = imageLoad(zBuffer, curPixelId + ivec2(1, 0) * 2).x;
    
    vec3 upPos = Reproject(upDepth, curPixelId + ivec2(0, 1) * 2);
    vec3 downPos = Reproject(downDepth, curPixelId + ivec2(0, -1) * 2);
    vec3 leftPos = Reproject(leftDepth, curPixelId + ivec2(-1, 0) * 2);
    vec3 rightPos = Reproject(rightDepth, curPixelId + ivec2(1, 0) * 2);

    vec3 tangentURight = rightPos - curPos;
    vec3 tangentULeft = curPos - leftPos;
    vec3 tangentVDown = curPos - downPos;
    vec3 tangentVUp = upPos - curPos;
    
    int isRight = int(abs(tangentULeft.z) > abs(tangentURight.z));
    vec3 tangentU = isRight *  normalize(tangentURight) + (1.0 - isRight) * normalize(tangentULeft);

    int isDown = int(abs(tangentVUp.z) > abs(tangentVDown.z));
    vec3 tangentV = isDown * normalize(tangentVDown) + (1.0 - isDown) * normalize(tangentVUp);

    return cross(tangentU, tangentV);
}

HitResult Intersect(Ray ray, Triangle triangle) {
	HitResult res;
    res.isHit = -1;
    res.dist = FLT_MAX;
	
	vec3 E1 = triangle.pos1 - triangle.pos0;
	vec3 E2 = triangle.pos2 - triangle.pos0;
	vec3 Q = cross(ray.direction, E2);

	float a = dot(E1, Q);
	if (abs(a) < EPS) {
        return res;
    }

	float f = 1.0 / a;
	vec3 S = ray.origin - triangle.pos0;
	float u = f * dot(S, Q);
	if (u < 0.0) {
        return res;
    }

	vec3 R = cross(S, E1);
	float v = f * dot(ray.direction, R);
	if (v < 0.0 || u + v > 1.0) {
        return res;
    }

	float t = f * dot(E2, R);
	if (t < EPS) {
        return res;
    }

	res.isHit = 1;
	res.hitPoint = ray.origin + t * ray.direction;
	res.dist = t;
	res.normal = cross(E1, E2);
	res.texCoord = (1.0 - u - v) * triangle.texCoord0 + u * triangle.texCoord1  + v * triangle.texCoord2;
	return res;
}

HitResult IntesectAllTriangles(Ray ray) {
    HitResult minRes;
    minRes.isHit = -1;
    minRes.dist = FLT_MAX;
    for(int i = 0; i < 2; i++) {
        HitResult res = Intersect(ray, triangles[i]);
        if(res.isHit == 1 && res.dist < minRes.dist) {
            minRes = res;
        }
    }
    return minRes;
}

// ------------阴影相关函数--------------
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
    vec4 fragNDC = lightProjection * lightView * model * vec4(curPosition, 1.0);
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

vec3 GetRayTraceColor(Ray ray, vec3 fragPosition, vec3 viewPosition) {
    HitResult res = IntesectAllTriangles(ray);
    if (res.isHit == 1) {
        vec3 albedo = texture(texAlbedo, res.texCoord).rgb;
        float roughness = texture(texRoughness, res.texCoord).r;
        vec4 lightPos = inverse(lightView) * vec4(0.0, 0.0, 0.0, 1.0);
        vec3 originColor = PhongShading(albedo, roughness, lightPos.xyz, fragPosition, viewPosition);

        return ShadeFloorWithShadow(originColor, lightPos.xyz, res.hitPoint);
    } else {    
        return texture(skybox, ray.direction.xzy).rgb;
    }
    return vec3(0.0);
}

// beer定律
float TransparentFactor(float thickness) {
    return max(exp(-thicknessFactor * thickness), 0.2);
}

vec3 imageCoordToWi(vec4 intrinsic, ivec2 imageCoord) {
    return vec3((imageCoord.x - intrinsic.z) * intrinsic.x, (imageCoord.y - intrinsic.w) * intrinsic.y, -1.0);
}

void main()
{
    ivec2 curPixelId = ivec2(gl_FragCoord.xy);
    float curDepth = imageLoad(zBuffer, curPixelId).x;
    if (curDepth > 0.0) {
        discard;
    }

    // 写入深度
    gl_FragDepth = ZToDepth(curDepth);

    vec4 camearOrigin = camToWorld * vec4(0.0, 0.0, 0.0, 1.0);

    // 计算位置
    vec3 curPos = Reproject(curDepth, curPixelId);
    vec4 curPoseOnWorld = camToWorld * vec4(curPos, 1.0);

    // 计算各种向量
    vec4 normal = vec4(CalculateNormal(curPixelId, curDepth, curPos), 1.0);
    vec4 normalOnWorld = camToWorldRot * normal;
    vec3 wiOnCamera = imageCoordToWi(cameraIntrinsic, curPixelId);
    vec4 wiOnWorld = camToWorldRot * vec4(normalize(wiOnCamera), 1.0);
    vec3 woReflect = reflect(wiOnWorld.xyz, normalOnWorld.xyz);
    vec3 woRefract = refract(wiOnWorld.xyz, normalOnWorld.xyz, eta);
    vec3 fresnel = FresnelSchlic(wiOnWorld.xyz, normalOnWorld.xyz);

    mat4 worldToModel = inverse(model);

    // 折射颜色
    Ray refractRay;     // 模型局部坐标系下
    refractRay.origin = (worldToModel * curPoseOnWorld).xyz;
    refractRay.direction = mat3(worldToModel) * woRefract;
    vec3 refractColor = GetRayTraceColor(refractRay, curPoseOnWorld.xyz, camearOrigin.xyz);

    float thickness = imageLoad(thicknessBuffer, curPixelId).x;
    float transparentFactor = TransparentFactor(thickness * 2.0);
    refractColor = transparentFactor * refractColor + (1.0 - transparentFactor) * fluidColor;

    // 反射颜色
    Ray reflectRay;
    reflectRay.origin = (worldToModel * curPoseOnWorld).xyz;
    reflectRay.direction = mat3(worldToModel) * woReflect;
    vec3 reflectColor = GetRayTraceColor(reflectRay, curPoseOnWorld.xyz, camearOrigin.xyz);

    // 混合
    vec3 outColor = mix(refractColor, reflectColor, fresnel);

    FragColor = vec4(outColor, 1.0);
}

