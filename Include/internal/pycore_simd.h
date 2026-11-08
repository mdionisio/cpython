/* Portable SIMD helpers based on GCC/Clang vector extensions.

   They are only available if _Py_HAVE_EFFICIENT_VECTOR_EXTENSIONS is
   defined: the compiler supports __attribute__((vector_size(16))) and the
   target architecture has native 128-bit SIMD (SSE2 on x86-64, NEON on
   ARM64), see configure.ac.  Callers must provide a scalar fallback.
*/

#ifndef Py_INTERNAL_SIMD_H
#define Py_INTERNAL_SIMD_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef Py_BUILD_CORE
#  error "this header requires Py_BUILD_CORE define"
#endif

#ifdef _Py_HAVE_EFFICIENT_VECTOR_EXTENSIONS

#include <stdint.h>               // uint64_t
#include <string.h>               // memcpy()

typedef unsigned char _Py_v16u8 __attribute__((vector_size(16)));
typedef uint64_t _Py_v2u64 __attribute__((vector_size(16)));
// Signed bytes, for comparisons: SSE2 has no unsigned byte comparison.
typedef signed char _Py_v16i8 __attribute__((vector_size(16)));

// Load 16 bytes from a possibly unaligned address.
static inline _Py_v16u8
_Py_v16u8_load(const unsigned char *p)
{
    _Py_v16u8 v;
    memcpy(&v, p, sizeof(v));
    return v;
}

// Return a vector with all 16 bytes set to x.
static inline _Py_v16u8
_Py_v16u8_splat(unsigned char x)
{
    return (_Py_v16u8){x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x};
}

// Return non-zero if any byte of v is non-ASCII (has its high bit set).
static inline int
_Py_v16u8_has_nonascii(_Py_v16u8 v)
{
    _Py_v2u64 x = (_Py_v2u64)v;
    return ((x[0] | x[1]) & 0x8080808080808080ULL) != 0;
}

// Return non-zero if any byte of v is non-zero.  Use it on the result of
// vector comparisons (lanes are 0 or 0xFF) to test if any lane matched.
static inline int
_Py_v16u8_any(_Py_v16u8 v)
{
    _Py_v2u64 x = (_Py_v2u64)v;
    return (x[0] | x[1]) != 0;
}

// Skip ASCII-only blocks of 64 bytes, then 16 bytes.  Return a pointer to
// the first 16-byte block which contains a non-ASCII byte, or to the last
// 0..15 bytes which have not been checked.
static inline const unsigned char *
_Py_simd_skip_ascii(const unsigned char *p, const unsigned char *end)
{
    while (end - p >= 64) {
        _Py_v16u8 o = (_Py_v16u8_load(p) | _Py_v16u8_load(p + 16))
                      | (_Py_v16u8_load(p + 32) | _Py_v16u8_load(p + 48));
        if (_Py_v16u8_has_nonascii(o)) {
            break;
        }
        p += 64;
    }
    while (end - p >= 16) {
        if (_Py_v16u8_has_nonascii(_Py_v16u8_load(p))) {
            break;
        }
        p += 16;
    }
    return p;
}

#endif  // _Py_HAVE_EFFICIENT_VECTOR_EXTENSIONS

#ifdef __cplusplus
}
#endif
#endif  // !Py_INTERNAL_SIMD_H
