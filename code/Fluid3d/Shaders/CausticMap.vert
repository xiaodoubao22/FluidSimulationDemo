#version 450

const float EPS = 1e-5;
const float FLT_MAX = 1e+10;

uniform float eta;
uniform mat4 lightView;
uniform mat4 lightProjection;
uniform vec4 lightIntrinsic;    // vec4(fxInv, fyInv, cx, cy)
uniform mat4 lightToWorld;
uniform mat4 lightToWorldRot;
uniform int imageWidth;
uniform int imageHeight;
uniform mat4 model;

layout(r32f, binding = 0) uniform image2D zBuffer;

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

vec2 imageCoordToNdc(vec2 imageCoord, float w, float h) {
    // [0, w] -> [-1, 1]
    return (imageCoord / vec2(w, h)) * 2.0 - vec2(1.0);
}

ivec2 VertexIdToImgCoord() {
    return ivec2(gl_VertexID % imageWidth, gl_VertexID / imageWidth);
}

vec3 Reproject(float zValue, ivec2 imageCoord, vec4 intrinsic) {
    float x = abs(zValue) * (float(imageCoord.x) - intrinsic.z) * intrinsic.x;
    float y = abs(zValue) * (float(imageCoord.y) - intrinsic.w) * intrinsic.y;   
    return vec3(x, y, zValue);
}

vec3 CalculateNormal(ivec2 curPixelId, float curDepth, vec3 curPos, vec4 intrinsic) {
    float upDepth = imageLoad(zBuffer, curPixelId + ivec2(0, 1) * 2).x;
    float downDepth = imageLoad(zBuffer, curPixelId + ivec2(0, -1) * 2).x;
    float leftDepth = imageLoad(zBuffer, curPixelId + ivec2(-1, 0) * 2).x;
    float rightDepth = imageLoad(zBuffer, curPixelId + ivec2(1, 0) * 2).x;
    
    vec3 upPos = Reproject(upDepth, curPixelId + ivec2(0, 1) * 2, intrinsic);
    vec3 downPos = Reproject(downDepth, curPixelId + ivec2(0, -1) * 2, intrinsic);
    vec3 leftPos = Reproject(leftDepth, curPixelId + ivec2(-1, 0) * 2, intrinsic);
    vec3 rightPos = Reproject(rightDepth, curPixelId + ivec2(1, 0) * 2, intrinsic);

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

vec3 imageCoordToWi(vec4 intrinsic, ivec2 imageCoord) {
    return vec3((imageCoord.x - intrinsic.z) * intrinsic.x, (imageCoord.y - intrinsic.w) * intrinsic.y, -1.0);
}

void main() {
    ivec2 imageCoord = VertexIdToImgCoord();
    float curDepth = imageLoad(zBuffer, imageCoord).x;
    if (curDepth > 0.0) {
        gl_Position = vec4(FLT_MAX);
        return;
    }

    vec3 curPos = Reproject(curDepth, imageCoord, lightIntrinsic);
    vec4 curPoseOnWorld = lightToWorld * vec4(curPos, 1.0);
    
    vec3 normal = CalculateNormal(imageCoord, curDepth, curPos, lightIntrinsic);
    vec4 normalOnWorld = lightToWorldRot * vec4(normal, 1.0);
    
    vec3 wiOnCamera = imageCoordToWi(lightIntrinsic, imageCoord);
    vec4 wiOnWorld = lightToWorldRot * vec4(normalize(wiOnCamera), 1.0);
    vec3 woRefract = refract(wiOnWorld.xyz, normalOnWorld.xyz, eta);

    mat4 worldToModel = inverse(model);

    Ray refractRay;     // 模型局部坐标系下
    refractRay.origin = (worldToModel * curPoseOnWorld).xyz;
    refractRay.direction = mat3(worldToModel) * woRefract;
    HitResult res = IntesectAllTriangles(refractRay);

    if (res.isHit != 1) {
        gl_Position = vec4(FLT_MAX);
        return;
    }

    vec4 posOnLight = lightProjection * lightView * model * vec4(res.hitPoint ,1.0);

    gl_PointSize = 4;
    gl_Position = posOnLight;
}