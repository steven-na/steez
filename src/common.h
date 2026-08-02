#pragma once

#include <stdint.h>
#include <stddef.h>
#include <math.h> // IWYU pragma: keep

#define KiB(n) ((u64)n<<10)
#define MiB(n) ((u64)n<<20)
#define GiB(n) ((u64)n<<30)

#ifndef PI
#define PI 3.14159265358979323846264338327950
#endif // !PI

#ifndef F64_EPSILON
#define F64_EPSILON = 1e-9
#endif // !F64_EPSILON

#define F64_EQ(x, y, eps) (fabs((x) - (y)) <= (eps))
#define MAX(n, m) ((n > m) ? (n) : (m))
#define MIN(n, m) ((n < m) ? (n) : (m))
#define ALIGN_UP_POW2(n, m) (((u64)(n) + (u64)(m) - 1) & (~((u64)(m) - 1)))

typedef   int8_t  i8;
typedef  int16_t i16;
typedef  int32_t i32;
typedef  int64_t i64;
typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef       i8  b8;
typedef      i32 b32;

typedef    float f32;
typedef   double f64;

#define ARENA_ALIGN (sizeof(void*))
