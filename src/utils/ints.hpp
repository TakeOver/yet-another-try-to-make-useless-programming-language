#pragma once
#include <cstdint>
typedef int64_t i64;
typedef uint64_t u64;
typedef int32_t i32;
typedef uint32_t u32;
typedef int16_t i16;
typedef uint16_t u16;
typedef int8_t i8;
typedef uint8_t u8;
typedef float f32;
typedef double f64;
typedef long double f80;
#if WORDSIZE >=64
typedef __int128_t i128;
typedef __uint128_t u128;
#else // well, i know.
typedef int64_t i128;
typedef uint64_t u128;
#endif