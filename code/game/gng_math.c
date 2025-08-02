#include "gng_math.h"
#include "gng_bool.h"

// https://web.archive.org/web/20130927121234/http://devmaster.net/posts/9648/fast-and-accurate-sine-cosine
f32 fastSin2PI (f32 x) {
    while (x > 0.5f) {
        x -= 1.0f;
    }
    while (x < -0.5f) {
        x += 1.0f;
    }

    f32 absX = x > 0.0f ? x : -x;
    f32 y = 8.0f * x - 16.0f * x * absX;
    f32 absY = y > 0.0f ? y : -y;
    return 0.225f * (y * absY - y) + y;
}

f32 fastCos2PI (f32 x) {
    return fastSin2PI(x + 0.25f);
}

// TODO: accurate sin/cos
f32 sin2PI (f32 x) {
    return fastSin2PI(x);
}
f32 cos2PI (f32 x) {
    return fastCos2PI(x);
}

float sqrtF32 (f32 num) {
    if (num < 0.0f) {
        return -1.0f; // error
    }
    f32 error = 0.00001f * num;

    f32 estimate = num;
    f32 test;
    u32 iterations = 0;
    do {
        ++iterations;
        estimate = (num / estimate + estimate) / 2.0f;
        test = estimate - (num / estimate);
        if (test < 0) { test = -test; }
    } while (test > error && iterations < 50);
    return estimate;
}

mat3x3 mat3x3MatrixMul (mat3x3 a, mat3x3 b) {
    mat3x3 result = {
        .m[0] = a.m[0]*b.m[0] + a.m[1]*b.m[3] + a.m[2]*b.m[6],
        .m[1] = a.m[0]*b.m[1] + a.m[1]*b.m[4] + a.m[2]*b.m[7],
        .m[2] = a.m[0]*b.m[2] + a.m[1]*b.m[5] + a.m[2]*b.m[8],

        .m[3] = a.m[3]*b.m[0] + a.m[4]*b.m[3] + a.m[5]*b.m[6], 
        .m[4] = a.m[3]*b.m[1] + a.m[4]*b.m[4] + a.m[5]*b.m[7], 
        .m[5] = a.m[3]*b.m[2] + a.m[4]*b.m[5] + a.m[5]*b.m[8],

        .m[6] = a.m[6]*b.m[0] + a.m[7]*b.m[3] + a.m[8]*b.m[6], 
        .m[7] = a.m[6]*b.m[1] + a.m[7]*b.m[4] + a.m[8]*b.m[7], 
        .m[8] = a.m[6]*b.m[2] + a.m[7]*b.m[5] + a.m[8]*b.m[8] 
    };
    return result;
}

mat4x4 mat4x4MatrixMul (mat4x4 a, mat4x4 b) {
    mat4x4 result = {
        .m[0] = a.m[0]*b.m[0] + a.m[1]*b.m[4] + a.m[2]*b.m[8]  + a.m[3]*b.m[12],
        .m[1] = a.m[0]*b.m[1] + a.m[1]*b.m[5] + a.m[2]*b.m[9]  + a.m[3]*b.m[13],
        .m[2] = a.m[0]*b.m[2] + a.m[1]*b.m[6] + a.m[2]*b.m[10] + a.m[3]*b.m[14],
        .m[3] = a.m[0]*b.m[3] + a.m[1]*b.m[7] + a.m[2]*b.m[11] + a.m[3]*b.m[15],

        .m[4] = a.m[4]*b.m[0] + a.m[5]*b.m[4] + a.m[6]*b.m[8]  + a.m[7]*b.m[12],
        .m[5] = a.m[4]*b.m[1] + a.m[5]*b.m[5] + a.m[6]*b.m[9]  + a.m[7]*b.m[13],
        .m[6] = a.m[4]*b.m[2] + a.m[5]*b.m[6] + a.m[6]*b.m[10] + a.m[7]*b.m[14],
        .m[7] = a.m[4]*b.m[3] + a.m[5]*b.m[7] + a.m[6]*b.m[11] + a.m[7]*b.m[15],

        .m[8]  = a.m[8]*b.m[0] + a.m[9]*b.m[4] + a.m[10]*b.m[8]  + a.m[11]*b.m[12],
        .m[9]  = a.m[8]*b.m[1] + a.m[9]*b.m[5] + a.m[10]*b.m[9]  + a.m[11]*b.m[13],
        .m[10] = a.m[8]*b.m[2] + a.m[9]*b.m[6] + a.m[10]*b.m[10] + a.m[11]*b.m[14],
        .m[11] = a.m[8]*b.m[3] + a.m[9]*b.m[7] + a.m[10]*b.m[11] + a.m[11]*b.m[15],

        .m[12] = a.m[12]*b.m[0] + a.m[13]*b.m[4] + a.m[14]*b.m[8]  + a.m[15]*b.m[12],
        .m[13] = a.m[12]*b.m[1] + a.m[13]*b.m[5] + a.m[14]*b.m[9]  + a.m[15]*b.m[13],
        .m[14] = a.m[12]*b.m[2] + a.m[13]*b.m[6] + a.m[14]*b.m[10] + a.m[15]*b.m[14],
        .m[15] = a.m[12]*b.m[3] + a.m[13]*b.m[7] + a.m[14]*b.m[11] + a.m[15]*b.m[15]
    };
    return result;
}

mat3x3 mat3x3ScalarMul (f32 a, mat3x3 b) {
    mat3x3 result = {
        .m[0] = a*b.m[0],
        .m[1] = a*b.m[1],
        .m[2] = a*b.m[2],
        .m[3] = a*b.m[3],
        .m[4] = a*b.m[4],
        .m[5] = a*b.m[5],
        .m[6] = a*b.m[6],
        .m[7] = a*b.m[7],
        .m[8] = a*b.m[8]
    };
    return result;
}

mat3x3 mat3x3Transpose (mat3x3 a) {
    mat3x3 result = {
        .m[0] = a.m[0],
        .m[1] = a.m[3],
        .m[2] = a.m[6],
        .m[3] = a.m[1],
        .m[4] = a.m[4],
        .m[5] = a.m[7],
        .m[6] = a.m[2],
        .m[7] = a.m[5],
        .m[8] = a.m[8]
    };
    return result;
}

mat4x4 mat4x4Transpose (mat4x4 a) {
    mat4x4 result = {
        .m[0] = a.m[0],
        .m[1] = a.m[4],
        .m[2] = a.m[8],
        .m[3] = a.m[12],
        .m[4] = a.m[1],
        .m[5] = a.m[5],
        .m[6] = a.m[9],
        .m[7] = a.m[13],
        .m[8] = a.m[2],
        .m[9] = a.m[6],
        .m[10] = a.m[10],
        .m[11] = a.m[14],
        .m[12] = a.m[3],
        .m[13] = a.m[7],
        .m[14] = a.m[11],
        .m[15] = a.m[15]
    };

    return result;
}

mat4x4 mat4x4FromQuat (quat q) {
    mat4x4 result = {
        .m[0] = 1.0f - (2.0f*q.y*q.y) - (2.0f*q.z*q.z),
        .m[1] = (2.0f*q.x*q.y) - (2.0f*q.w*q.z),
        .m[2] = (2.0f*q.x*q.z) + (2.0f*q.w*q.y),
        .m[3] = 0.0f,

        .m[4] = (2.0f*q.x*q.y) + (2.0f*q.w*q.z),
        .m[5] = 1.0f - (2.0f*q.x*q.x) - (2.0f*q.z*q.z),
        .m[6] = (2.0f*q.y*q.z) - (2.0f*q.w*q.x),
        .m[7] = 0.0f,

        .m[8] = (2.0f*q.x*q.z) - (2.0f*q.w*q.y),
        .m[9] = (2.0f*q.y*q.z) + (2.0f*q.w*q.x),
        .m[10] = 1.0f - (2.0f*q.x*q.x) - (2.0f*q.y*q.y),
        .m[11] = 0.0f,

        .m[12] = 0.0f,
        .m[13] = 0.0f,
        .m[14] = 0.0f,
        .m[15] = 1.0f
    };
    return result;
}

mat4x4 identityMatrix4x4 () {
    mat4x4 result = {
        .m[0] = 1.0f,
        .m[5] = 1.0f,
        .m[10] = 1.0f,
        .m[15] = 1.0f,
    };

    return result;
}

mat4x4 translationMatrix (f32 x, f32 y, f32 z) {
    mat4x4 result = identityMatrix4x4();

    result.m[3] = x;
    result.m[7] = y;
    result.m[11] = z;

    return result;
}

mat4x4 scaleMatrix4x4 (f32 s) {
    mat4x4 result = {};

    result.m[0] = s;
    result.m[5] = s;
    result.m[10] = s;
    result.m[15] = 1.0f;

    return result;
}

mat4x4 scaleMatrix4x4XYZ (f32 x, f32 y, f32 z) {
    mat4x4 result = {};

    result.m[0] = x;
    result.m[5] = y;
    result.m[10] = z;
    result.m[15] = 1.0f;

    return result;
}


mat4x4 createViewMatrix (quat rotation, float x, float y, float z) {
    mat4x4 result = mat4x4FromQuat(rotation);
    result = mat4x4Transpose(result);

    float newCamX = vec3Dot((vec3){ .x = x, .y = y, .z = z}, (vec3){ .x = result.m[0], .y = result.m[1], .z = result.m[2]});
    float newCamY = vec3Dot((vec3){ .x = x, .y = y, .z = z}, (vec3){ .x = result.m[4], .y = result.m[5], .z = result.m[6]});
    float newCamZ = vec3Dot((vec3){ .x = x, .y = y, .z = z}, (vec3){ .x = result.m[8], .y = result.m[9], .z = result.m[10]});
    result.m[3] = -newCamX;
    result.m[7] = -newCamY;
    result.m[11] = -newCamZ;

    return result;
}

mat4x4 createPerspectiveMatrix (float nearPlane, float farPlane, float aspectRatio, float tanfFovYOver2) {
    //float f = 1.0f / tanf(fovy / 2.0f);
    float f = 1.0f / tanfFovYOver2;
    float nf = 1.0f / (farPlane - nearPlane);

    mat4x4 result = identityMatrix4x4();

    result.m[0] = f / aspectRatio;
    result.m[5] = f;
    result.m[10] = -(farPlane + nearPlane) * nf;
    result.m[11] = -2 * (farPlane * nearPlane) * nf;
    result.m[14] = -1.0f;
    result.m[15] = 0;

    return result;
}

mat4x4 rotationMatrixFromAxisAngle (vec3 axis, f32 angle) {
    mat4x4 result;

    quat axisAngleQuaternion = quaternionFromAxisAngle(axis, angle);
    result = mat4x4FromQuat(axisAngleQuaternion);

    return result;
}

mat3x3 mat3x3Inv (mat3x3 a) {
    // 0 1 2
    // 3 4 5
    // 6 7 8

    mat3x3 minors = {
        .m[0] = a.m[4]*a.m[8] - a.m[5]*a.m[7],
        .m[1] = -(a.m[3]*a.m[8] - a.m[5]*a.m[6]),
        .m[2] = a.m[3]*a.m[7] - a.m[4]*a.m[6],

        .m[3] = -(a.m[1]*a.m[8] - a.m[2]*a.m[7]),
        .m[4] = a.m[0]*a.m[8] - a.m[2]*a.m[6],
        .m[5] = -(a.m[0]*a.m[7] - a.m[1]*a.m[6]),

        .m[6] = a.m[1]*a.m[5] - a.m[2]*a.m[4],
        .m[7] = -(a.m[0]*a.m[5] - a.m[2]*a.m[3]),
        .m[8] = a.m[0]*a.m[4] - a.m[1]*a.m[3]
    };


    f32 determinant = a.m[0]*minors.m[0] + a.m[1]*minors.m[1] + a.m[2]*minors.m[2]; // Check for 0?
    f32 ood = 1.0f / determinant;

    mat3x3 result = mat3x3ScalarMul(ood, mat3x3Transpose(minors));
    return result;
}

mat3x3 mat3x3Identity () {
    mat3x3 result = {
        .m[0] = 1.0f,
        .m[4] = 1.0f,
        .m[8] = 1.0f
    };
    return result;
}

mat3x3 mat3x3Translate (f32 x, f32 y) {
    mat3x3 result = mat3x3Identity();
    result.m[2] = x;
    result.m[5] = y;
    return result;
}

mat3x3 mat3x3Scale (f32 s) {
    mat3x3 result = {
        .m[0] = s,
        .m[4] = s,
        .m[8] = 1.0f
    };
    return result;
}

mat3x3 mat3x3ScaleXY (f32 x, f32 y) {
    mat3x3 result = {
        .m[0] = x,
        .m[4] = y,
        .m[8] = 1.0f
    };
    return result;
}

mat3x3 mat3x3Rotate2PI (f32 angle) {
    mat3x3 result = mat3x3Identity();

    result.m[0] = cos2PI(angle);
    result.m[1] = -sin2PI(angle);
    result.m[3] = sin2PI(angle);
    result.m[4] = cos2PI(angle);

    return result;
}

f32 squaref32 (f32 a) {
    return a * a;
}

vec2 vec2Add (vec2 a, vec2 b) {
    vec2 result = {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
    return result;
}

vec2 vec2Subtract (vec2 a, vec2 b) {
    vec2 result = {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
    return result;
}

vec2 vec2Negate (vec2 a) {
    vec2 result = {
        .x = -a.x,
        .y = -a.y
    };
    return result;
}

vec2 vec2ScalarMul (f32 a, vec2 b) {
    vec2 result = {
        .x = a * b.x,
        .y = a * b.y
    };
    return result;
}

vec2 vec2HadamardMul (vec2 a, vec2 b) {
    vec2 result = {
        .x = a.x * b.x,
        .y = a.y * b.y
    };
    return result;
}

f32 vec2Dot (vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

f32 vec2LengthSqr (vec2 a) {
    return vec2Dot(a, a);
}

f32 vec2Length (vec2 a) {
    return sqrtF32(vec2LengthSqr(a));
}

vec2 vec2Normalize (vec2 a) {
    return vec2ScalarMul((1.0f / vec2Length(a)), a);
}


// shortcuts for using mat3x3's with vec2's
vec2 vec2Mat3x3Mul (mat3x3 m, vec2 v){
    vec2 result = {
        .x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*1.0f,
        .y = m.m[3]*v.x + m.m[4]*v.y + m.m[5]*1.0f
    };

    // w component would be here
    // TODO(ebuchholz): check if this is a good idea

    return result;
}

vec2 vec2TransformPoint (mat3x3 m, vec2 v, f32* z){
    vec2 result = {
        .x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*(*z),
        .y = m.m[3]*v.x + m.m[4]*v.y + m.m[5]*(*z)
    };

    // z as an out parameter so we don't have to make a vector3
    *z = m.m[6]*v.x + m.m[7]*v.y + m.m[8]*1.0f;

    return result;
}

// Multiplies the vector like a direction
// TODO(ebuchholz): make a separate point data type?
vec2 vec2TransformDir (mat3x3 m, vec2 v) {
    vec2 result = {
        .x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*0.0f,
        .y = m.m[3]*v.x + m.m[4]*v.y + m.m[5]*0.0f
    };

    // w component would be here
    // TODO(ebuchholz): check if this is a good idea

    return result;
}

vec3 vec3Add (vec3 a, vec3 b) {
    vec3 result = {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
    return result;
}

vec3 vec3Subtract (vec3 a, vec3 b) {
    vec3 result = {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
    return result;
}

vec3 vec3Negate (vec3 a) {
    vec3 result = {
        .x = -a.x,
        .y = -a.y,
        .z = -a.z
    };
    return result;
}

vec3 vec3ScalarMul (f32 a, vec3 b) {
    vec3 result = {
        .x = a * b.x,
        .y = a * b.y,
        .z = a * b.z
    };
    return result;
}

vec3 vec3HadamardMul (vec3 a, vec3 b) {
    vec3 result = {
        .x = a.x * b.x,
        .y = a.y * b.y,
        .z = a.z * b.z 
    };
    return result;
}

f32 vec3Dot (vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3 crossProduct (vec3 a, vec3 b) {
    vec3 result = {
        .x = -b.y * a.z + a.y * b.z,
        .y = -b.z * a.x + a.z * b.x,
        .z = -b.x * a.y + a.x * b.y
    };
    return result;
}

vec3 vec3MatrixMul (mat3x3 m, vec3 v){
    vec3 result = {
        .x = m.m[0]*v.x + m.m[1]*v.y + m.m[2]*v.z,
        .y = m.m[3]*v.x + m.m[4]*v.y + m.m[5]*v.z,
        .z = m.m[6]*v.x + m.m[7]*v.y + m.m[8]*v.z
    };
    return result;
}

f32 vec3LengthSqr (vec3 a) {
    return vec3Dot(a, a);
}

b32 rectContainsPoint (rect r, f32 x, f32 y) {
    return x >= r.min.x && x < r.max.x && y >= r.min.y && y < r.max.y;
}

b32 rectsIntersect (rect a, rect b) {
    if (a.max.x < b.min.x) { return false; }
    if (a.min.x > b.max.x) { return false; }
    if (a.max.y < b.min.y) { return false; }
    if (a.min.y > b.max.y) { return false; }
    return true;
}

quat quaternionFromAxisAngle (vec3 axis, f32 angle) {
    quat result;
    float angleOverTwo = angle / 2.0f;
    float sinAngleOverTwo = fastSin2PI(angleOverTwo);

    result.w = fastCos2PI(angleOverTwo);
    result.x = axis.x * sinAngleOverTwo;
    result.y = axis.y * sinAngleOverTwo;
    result.z = axis.z * sinAngleOverTwo;

    return result;
}

quat quatMul (quat a, quat b) {
    quat result;

    result.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
    result.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
    result.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x;
    result.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w;

    return result;
}

quat conjugate (quat a) {
    quat result;

    result.w = a.w;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;

    return result;
}

vec3 rotateVectorByQuaternion (vec3 v, quat q) {
    quat qConjugate = conjugate(q);

    quat quaternionVersionOfVector;
    quaternionVersionOfVector.w = 0.0f;
    quaternionVersionOfVector.x = v.x;
    quaternionVersionOfVector.y = v.y;
    quaternionVersionOfVector.z = v.z;

    quat newQ1 = quatMul(q, quaternionVersionOfVector);
    quat newQ2 = quatMul(newQ1, qConjugate);

    return (vec3){ .x=newQ2.x, .y=newQ2.y, .z=newQ2.z};
}

vec3 transformPoint (mat4x4 m, vec3 v, float* w){
    vec3 result;

    result.x = m.m[0]*v.x + m.m[1]*v.y +  m.m[2]*v.z +  m.m[3]*(*w);
    result.y = m.m[4]*v.x + m.m[5]*v.y +  m.m[6]*v.z +  m.m[7]*(*w);
    result.z = m.m[8]*v.x + m.m[9]*v.y + m.m[10]*v.z + m.m[11]*(*w);

    // w as an out parameter so we don't have to make a vector4
    *w = m.m[12]*v.x + m.m[13]*v.y + m.m[14]*v.z + m.m[15]*1.0f;

    return result;
}

// TODO: implement sqrt
//f32 vec3Length (vec3 a) {
//    return sqrtf(vec3LengthSqr(a));
//}

//vec3 vec3Normalize (vec3 a) {
//    return a * (1.0f / vec3Length(a));
//}
