#pragma once
#include "clm_defines.h"

typedef struct mat3 {
  clm_f32 elements[9];
} Mat3;

clm_inline Mat3 mat3_identity() {
  Mat3 out_mat = {0};
  out_mat.elements[0] = 1.0f;
  out_mat.elements[4] = 1.0f;
  out_mat.elements[8] = 1.0f;
  return out_mat;
}

clm_inline Mat3 mat3_mul(const Mat3 mat_0, const Mat3 mat_1) {
  Mat3 out;

  const clm_f32* a = mat_0.elements;
  const clm_f32* b = mat_1.elements;
  clm_f32* d = out.elements;

  for (clm_u16 i = 0; i < 3; ++i) {
    const clm_f32 a0 = a[0];
    const clm_f32 a1 = a[1];
    const clm_f32 a2 = a[2];

    d[0] = a0*b[0] + a1*b[3] + a2*b[6];
    d[1] = a0*b[1] + a1*b[4] + a2*b[7];
    d[2] = a0*b[2] + a1*b[5] + a2*b[8];

    a += 3;
    d += 3;
  }
  return out;
}
