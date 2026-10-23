#pragma once
#include "clm_defines.h"

typedef struct mat4 {
  clm_f32 elements[16];
} Mat4;

clm_inline Mat4 mat4_identity() {
  Mat4 out_mat = {0};
  out_mat.elements[0] = 1.0f;
  out_mat.elements[5] = 1.0f;
  out_mat.elements[10] = 1.0f;
  out_mat.elements[15] = 1.0f;
  return out_mat;
}

clm_inline Mat4 mat4_mul(const Mat4 mat_0, const Mat4 mat_1) {
  Mat4 out;

  const clm_f32* a = mat_0.elements;
  const clm_f32* b = mat_1.elements;
  clm_f32* d = out.elements;

  for (clm_u16 i = 0; i < 4; ++i) {
    const clm_f32 a0 = a[0];
    const clm_f32 a1 = a[1];
    const clm_f32 a2 = a[2];
    const clm_f32 a3 = a[3];

    d[0] = a0*b[0]  + a1*b[4]  + a2*b[8]  + a3*b[12];
    d[1] = a0*b[1]  + a1*b[5]  + a2*b[9]  + a3*b[13];
    d[2] = a0*b[2]  + a1*b[6]  + a2*b[10] + a3*b[14];
    d[3] = a0*b[3]  + a1*b[7]  + a2*b[11] + a3*b[15];

    a += 4;
    d += 4;
  }
  return out;
}
