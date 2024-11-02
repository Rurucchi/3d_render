/*  ----------------------------------- TYPES
	Basic types used throughout the program. For the math library; check mathlib.h
	
*/

#ifndef _TYPESH_
#define _TYPESH_

#define Assert(expression)                                                     \
  if(!(expression)) {                                                          \
    *(int *)0 = 0;                                                             \
}

// standard types

typedef unsigned int uint;

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

// math types

typedef Vector2 v2;
typedef Vector3 v3;
typedef Vector4 v4;
typedef Matrix mx;

ui32 SafeTruncateUInt64(ui64 value){
  Assert(value <= 0xFFFFFFFF);
  ui32 result = (ui32)value;
  return result;
}

#endif /* _TYPESH_ */