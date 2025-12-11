#pragma once
#include "../vendor/raylib/src/raylib.h"
#include <math.h>

inline Vector3 Vector3Subtract(Vector3 v1, Vector3 v2) {
  return (Vector3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

inline Vector3 Vector3Add(Vector3 v1, Vector3 v2) {
  return (Vector3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

inline float Vector3Length(Vector3 v) {
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vector3 Vector3Scale(Vector3 v, float scale) {
  return (Vector3){v.x * scale, v.y * scale, v.z * scale};
}

inline Vector3 Vector3Normalize(Vector3 v) {
  float length = Vector3Length(v);
  if (length == 0)
    length = 1.0f;
  return Vector3Scale(v, 1.0f / length);
}

inline Vector3 Vector3CrossProduct(Vector3 v1, Vector3 v2) {
  return (Vector3){v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
                   v1.x * v2.y - v1.y * v2.x};
}

// Rotate vector around an axis (angle in radians)
inline Vector3 Vector3RotateByAxisAngle(Vector3 v, Vector3 axis, float angle) {
  // Rodriguez rotation formula
  // v_rot = v * cos(theta) + (axis x v) * sin(theta) + axis * (axis . v) * (1 -
  // cos(theta))

  float cosAngle = cosf(angle);
  float sinAngle = sinf(angle);

  Vector3 cross = Vector3CrossProduct(axis, v);
  float dot = v.x * axis.x + v.y * axis.y + v.z * axis.z;

  return (Vector3){
      v.x * cosAngle + cross.x * sinAngle + axis.x * dot * (1.0f - cosAngle),
      v.y * cosAngle + cross.y * sinAngle + axis.y * dot * (1.0f - cosAngle),
      v.z * cosAngle + cross.z * sinAngle + axis.z * dot * (1.0f - cosAngle)};
}
