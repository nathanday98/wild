#ifndef STD_H
#define STD_H

#define Swap(X, Y) {auto SWAPTHINGY = X; X = Y; Y = SWAPTHINGY;}

#include <stdarg.h>
#include <stdio.h>

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;} else {}
#define ArrayCount(Array) (sizeof(Array) / sizeof(Array[0]))

inline char *formatString(char *Format, ...) {
	static char char_buffer[1024];
	va_list Va;
	va_start(Va, Format);
	vsprintf(char_buffer, Format, Va);
	va_end(Va);
	return char_buffer;
}

#define global_variable static
#define local_persist static
#define internal_func static

#include <stdint.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

#define RGBA(r, g, b, a) ((a << 24) | (b << 16) | (g << 8) | r)


#define GetByte(n, x) ((x >> (8*n)) & 0xff)
// #define SetBit(n, v, x) x |= v << n;
#define ContainsBits(n, b) ((n & b) == b)

#define DLL_EXPORT __declspec(dllexport)

#endif // STD_H