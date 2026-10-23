#pragma once
#include "clm_defines.h"

typedef struct vec2 {
  clm_f32 elements[2];
} Vec2;

clm_inline Vec2 clm_vec2_create(const clm_f32 x, const clm_f32 y) {
  Vec2 out_vec;
  out_vec.elements[0] = x;
  out_vec.elements[1] = y;
  return out_vec;
}

clm_inline Vec2 clm_vec2_add(const Vec2 vec_0, const Vec2 vec_1) {
  return (Vec2){
    vec_0.elements[0] + vec_1.elements[0],
    vec_0.elements[1] + vec_1.elements[1]};
}

clm_inline Vec2 clm_vec2_sub(const Vec2 vec_0, const Vec2 vec_1) {
  return (Vec2){
    vec_0.elements[0] - vec_1.elements[0],
    vec_0.elements[1] - vec_1.elements[1]};
}

clm_inline Vec2 clm_vec2_mul(const Vec2 vec_0, const Vec2 vec_1) {
  return (Vec2){
    vec_0.elements[0] * vec_1.elements[0],
    vec_0.elements[1] * vec_1.elements[1]};
}

clm_inline Vec2 clm_vec2_div(const Vec2 vec_0, const Vec2 vec_1) {
  return (Vec2){
    vec_0.elements[0] / vec_1.elements[0],
    vec_0.elements[1] / vec_1.elements[1]};
}


