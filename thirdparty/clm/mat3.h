#pragma once
#include "clm_defines.h"

typedef struct mat2 {
  clm_f32 elements[4];
} Mat2;

clm_inline Mat2 mat2_identity() {
  Mat2 out_mat = {0};
  out_mat.elements[0] = 1.0f;
  out_mat.elements[3] = 1.0f;
  return out_mat;
}

clm_inline Mat2 mat2_mul(const Mat2 mat_0, const Mat2 mat_1) {
  Mat2 out;

  const clm_f32* a = mat_0.elements;
  const clm_f32* b = mat_1.elements;
  clm_f32* d = out.elements;

  for (clm_u16 i = 0; i < 2; ++i) {
    const clm_f32 a0 = a[0];
    const clm_f32 a1 = a[1];

    d[0] = a0*b[0] + a1*b[2];
    d[1] = a0*b[1] + a1*b[3];

    a += 2;
    d += 2;
  }
  return out;
}
