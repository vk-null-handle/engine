#pragma once
#include "clm_defines.h"

clm_inline clm_f32 clm_to_rad(const clm_f32 deg) {
  return deg * (CLM_PI / 180.0f);
}

clm_inline clm_f32 clm_to_deg(const clm_f32 rad) {
  return rad * (180.0f / CLM_PI);
}

clm_inline clm_f32 clm_lerp(const clm_f32 a, const clm_f32 b, const clm_f32 t) {
  return a + t * (b - a);
}
