#version 430 core

// ----------const----------
float fxInv = 0.0011547;    // 2 * tan(fovx/2) / w
float fyInv = 0.0011547;    // 2 * tan(fovy/2) / w
float cx = 500;
float cy = 500;

float ior = 1.0 / 1.5;

float zFarInv = 1.0 / 100.0;
float zNearInv = 1.0 / 0.1;

vec3 f0 = vec3(0.15, 0.15, 0.15);

vec3 fluidColor = vec3(0.1, 0.55, 1.0);

const float EPS = 1e-5;

// ----------define----------
struct Vertex {
    vec3 position;
    vec2 texCoord;
};

struct Triangle {
    // Vertex v0;
    // Vertex v1;
    // Vertex v2;
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

// ----------buffers----------
uniform samplerCube skybox;
layout(r32f, binding = 0) uniform image2D zBuffer;
layout(r32f, binding = 1) uniform image2D thicknessBuffer;
// layout(std430, binding=0) buffer Triangles
// {
//     Triangle triangles[];
// };
uniform sampler2D texAlbedo;

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
    float x = abs(depth) * (float(imageCoord.x) - cx) * fxInv;
    float y = abs(depth) * (float(imageCoord.y) - cy) * fyInv;   
    return vec3(x, y, depth);
}

float ZToDepth(float z) {
    return ((1 / abs(z)) - zNearInv) / (zFarInv - zNearInv);
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
    res.dist = 100000;
	
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
    minRes.dist = 1e+10;
    for(int i = 0; i < 2; i++) {
        HitResult res = Intersect(ray, triangles[i]);
        if(res.isHit == 1 && res.dist < minRes.dist) {
            minRes = res;
        }
    }
    return minRes;
}

vec3 GetColor(Ray ray) {
    HitResult res = IntesectAllTriangles(ray);
    if (res.isHit == 1) {
        return texture(texAlbedo, res.texCoord).rgb;
    } else {
        return texture(skybox, ray.direction.xzy).rgb;
    }
    return vec3(0.0);
}

float TransparentFactor(float thickness) {
    return max(exp(-0.5 * thickness), 0.2);
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

    // 计算位置
    vec3 curPos = Reproject(curDepth, curPixelId);
    vec4 curPoseOnWorld = camToWorld * vec4(curPos, 1.0);

    // 计算各种向量
    vec4 normal = vec4(CalculateNormal(curPixelId, curDepth, curPos), 1.0);
    vec4 normalOnWorld = camToWorldRot * normal;
    vec3 wiOnCamera = vec3((gl_FragCoord.x - cx) * fxInv, (gl_FragCoord.y - cy) * fyInv, -1.0);
    vec4 wiOnWorld = camToWorldRot * vec4(normalize(wiOnCamera), 1.0);
    vec3 woReflect = reflect(wiOnWorld.xyz, normalOnWorld.xyz);
    vec3 woRefract = refract(wiOnWorld.xyz, normalOnWorld.xyz, ior);
    vec3 fresnel = FresnelSchlic(wiOnWorld.xyz, normalOnWorld.xyz);

    Ray refractRay;
    refractRay.origin = curPoseOnWorld.xyz;
    refractRay.direction = woRefract;
    vec3 refractColor = GetColor(refractRay);

    float thickness = imageLoad(thicknessBuffer, curPixelId).x;
    float transparentFactor = TransparentFactor(thickness * 2.0);
    refractColor = transparentFactor * refractColor + (1.0 - transparentFactor) * fluidColor;

    Ray reflectRay;
    reflectRay.origin = curPoseOnWorld.xyz;
    reflectRay.direction = woReflect;
    vec3 reflectColor = GetColor(reflectRay);

    vec3 outColor = fresnel * reflectColor + (vec3(1.0) - fresnel) * refractColor;

    FragColor = vec4(outColor, 1.0);
}