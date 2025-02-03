#include "math.h"
#include <SDL3/SDL_stdinc.h>

struct APP_Matrix4x4 
APP_Matrix4x4_CreatePerspectiveFieldOfView(
    float field_of_view, 
    float aspect_ratio, 
    float near_plane_distance,
    float far_plane_distance
) {
    float num = 1.0f / ((float)SDL_tanf(field_of_view * 0.5f));

    return (struct APP_Matrix4x4){
      num / aspect_ratio, 0, 0, 0,
      0, num, 0, 0, 0,
      0, far_plane_distance / (near_plane_distance - far_plane_distance), -1,
      0, 0, (near_plane_distance * far_plane_distance) / (near_plane_distance - far_plane_distance), 0
  };
}

struct APP_Matrix4x4 
APP_Matrix4x4_CreateLookAt(
    struct APP_Vector3 camera_pos,
    struct APP_Vector3 camera_target,
    struct APP_Vector3 camera_up_vector
) {
  struct APP_Vector3 target_to_pos = {
      camera_pos.x - camera_target.x,
      camera_pos.y - camera_target.y,
      camera_pos.z - camera_target.z,
  };

  struct APP_Vector3 v_a = APP_VECTOR3_Normalize(target_to_pos);
  struct APP_Vector3 v_b = APP_VECTOR3_Normalize(APP_Vector3_Cross(camera_up_vector, v_a));
  struct APP_Vector3 v_c = APP_Vector3_Cross(v_a, v_b);

  return (struct APP_Matrix4x4) {
    v_b.x, v_c.x, v_a.x, 0,
    v_b.y, v_c.y, v_a.y, 0,
    v_b.z, v_c.z, v_a.z, 0,
    -APP_Vector3_Dot(v_b, camera_pos), -APP_Vector3_Dot(v_c, camera_pos), -APP_Vector3_Dot(v_a, camera_pos), 1
  };
}

struct APP_Vector3
APP_VECTOR3_Normalize(struct APP_Vector3 v3)
{
    float magnitude = SDL_sqrt((v3.x * v3.x) + (v3.y * v3.y) + (v3.z * v3.z));
    return (struct APP_Vector3) {
        v3.x / magnitude,
        v3.y / magnitude,
        v3.z / magnitude,
    };
}

float
APP_Vector3_Dot(struct APP_Vector3 vec_a, struct APP_Vector3 vec_b)
{
    return (vec_a.x * vec_b.x) + (vec_a.y * vec_b.y) + (vec_a.z * vec_b.z);
}

struct APP_Vector3
APP_Vector3_Cross(struct APP_Vector3 vec_a, struct APP_Vector3 vec_b) 
{
    return (struct APP_Vector3) {
        vec_a.y * vec_b.z - vec_b.y * vec_a.z,
		-(vec_a.x * vec_b.z - vec_b.x * vec_a.z),
		vec_a.x * vec_b.y - vec_b.x * vec_a.y
    };
}

struct APP_Matrix4x4
APP_Matrix4x4_Mutliply(struct APP_Matrix4x4 m_a, struct APP_Matrix4x4 m_b)
{
    struct APP_Matrix4x4 out;

    out.m11 = (
        (m_a.m11 * m_b.m11) +
        (m_a.m12 * m_b.m21) +
        (m_a.m13 * m_b.m31) +
        (m_a.m14 * m_b.m41)
    );

    out.m12 = (
        (m_a.m11 * m_b.m12) +
        (m_a.m12 * m_b.m22) + 
        (m_a.m13 * m_b.m32) + 
        (m_a.m14 * m_b.m42) 
    );

    out.m13 = (
        (m_a.m11 * m_b.m13) +
        (m_a.m12 * m_b.m23) +
        (m_a.m13 * m_b.m33) +
        (m_a.m14 * m_b.m43) 
    );

    out.m14 = (
       (m_a.m11 * m_b.m14) +
       (m_a.m12 * m_b.m24) +
       (m_a.m13 * m_b.m34) +
       (m_a.m14 * m_b.m44) 
    );

	out.m21 = (
		(m_a.m21 * m_b.m11) +
		(m_a.m22 * m_b.m21) +
		(m_a.m23 * m_b.m31) +
		(m_a.m24 * m_b.m41)
	);

	out.m22 = (
		(m_a.m21 * m_b.m12) +
		(m_a.m22 * m_b.m22) +
		(m_a.m23 * m_b.m32) +
		(m_a.m24 * m_b.m42)
	);

	out .m23 = (
		(m_a.m21 * m_b.m13) +
		(m_a.m22 * m_b.m23) +
		(m_a.m23 * m_b.m33) +
		(m_a.m24 * m_b.m43)
	);

	out.m24 = (
		(m_a.m21 * m_b.m14) +
		(m_a.m22 * m_b.m24) +
		(m_a.m23 * m_b.m34) +
		(m_a.m24 * m_b.m44)
	);

	out.m31 = (
		(m_a.m31 * m_b.m11) +
		(m_a.m32 * m_b.m21) +
		(m_a.m33 * m_b.m31) +
		(m_a.m34 * m_b.m41)
	);

	out.m32 = (
		(m_a.m31 * m_b.m12) +
		(m_a.m32 * m_b.m22) +
		(m_a.m33 * m_b.m32) +
		(m_a.m34 * m_b.m42)
	);

	out.m33 = (
		(m_a.m31 * m_b.m13) +
		(m_a.m32 * m_b.m23) +
		(m_a.m33 * m_b.m33) +
		(m_a.m34 * m_b.m43)
	);

	out.m34 = (
		(m_a.m31 * m_b.m14) +
		(m_a.m32 * m_b.m24) +
		(m_a.m33 * m_b.m34) +
		(m_a.m34 * m_b.m44)
	);

	out.m41 = (
		(m_a.m41 * m_b.m11) +
		(m_a.m42 * m_b.m21) +
		(m_a.m43 * m_b.m31) +
		(m_a.m44 * m_b.m41)
	);

	out.m42 = (
		(m_a.m41 * m_b.m12) +
		(m_a.m42 * m_b.m22) +
		(m_a.m43 * m_b.m32) +
		(m_a.m44 * m_b.m42)
	);
	out.m43 = (
		(m_a.m41 * m_b.m13) +
		(m_a.m42 * m_b.m23) +
		(m_a.m43 * m_b.m33) +
		(m_a.m44 * m_b.m43)
	);

	out.m44 = (
		(m_a.m41 * m_b.m14) +
		(m_a.m42 * m_b.m24) +
		(m_a.m43 * m_b.m34) +
		(m_a.m44 * m_b.m44)
	);

    return out;
}
