#ifndef MATH_H
#define MATH_H

#include <SDL3/SDL_stdinc.h>

struct APP_Vector3 {
    float x, y, z;
};

struct APP_Matrix4x4 {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
};

struct APP_PositionColorVertex {
    float x, y, z;
    Uint8 f, g, b, a;
};

float APP_Vector3_Dot(struct APP_Vector3 vec_a, struct APP_Vector3 vec_b);
struct APP_Vector3 APP_VECTOR3_Normalize(struct APP_Vector3 v3);
struct APP_Vector3 APP_Vector3_Cross(struct APP_Vector3 vec_a, struct APP_Vector3 vec_b); 

struct APP_Matrix4x4 APP_Matrix4x4_CreatePerspectiveFieldOfView(
        float field_of_view, 
        float aspect_ratio, 
        float near_plane_distance,
        float far_plane_distance
);

struct APP_Matrix4x4 APP_Matrix4x4_CreateLookAt(
        struct APP_Vector3 camera_pos,
        struct APP_Vector3 camera_target,
        struct APP_Vector3 camera_up_vector
);

struct APP_Matrix4x4 APP_Matrix4x4_Mutliply(
        struct APP_Matrix4x4 m_a,
        struct APP_Matrix4x4 m_b
);

#endif
