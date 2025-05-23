/* SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Copyright:
 *  2020      Evan Nemerson <evan@nemerson.com>
 */

#if !defined(SIMDE_X86_AVX512_TYPES_H)
#define SIMDE_X86_AVX512_TYPES_H

#include "../avx.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

/* The problem is that Microsoft doesn't support 64-byte aligned parameters, except for
 * __m512/__m512i/__m512d.  Since our private union has an __m512 member it will be 64-byte
 * aligned even if we reduce the alignment requirements of other members.
 *
 * Even if we're on x86 and use the native AVX-512 types for arguments/return values, the
 * to/from private functions will break, and I'm not willing to change their APIs to use
 * pointers (which would also require more verbose code on the caller side) just to make
 * MSVC happy.
 *
 * If you want to use AVX-512 in SIMDe, you'll need to either upgrade to MSVC 2017 or later,
 * or upgrade to a different compiler (clang-cl, perhaps?).  If you have an idea of how to
 * fix this without requiring API changes (except transparently through macros), patches
 * are welcome.
 */

#  if defined(HEDLEY_MSVC_VERSION) && !HEDLEY_MSVC_VERSION_CHECK(19,10,0)
#    if defined(SIMDE_X86_AVX512F_NATIVE)
#      undef SIMDE_X86_AVX512F_NATIVE
#      pragma message("Native AVX-512 support requires MSVC 2017 or later.  See comment above (in code) for details.")
#    endif
#    define SIMDE_AVX512_ALIGN SIMDE_ALIGN_TO_32
#  else
#    define SIMDE_AVX512_ALIGN SIMDE_ALIGN_TO_64
#  endif

typedef union {
  #if defined(SIMDE_VECTOR_SUBSCRIPT)
    SIMDE_AVX512_ALIGN int8_t          i8 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int16_t        i16 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int32_t        i32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int64_t        i64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint8_t         u8 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint16_t       u16 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint32_t       u32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint64_t       u64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    #if defined(SIMDE_HAVE_INT128_)
    SIMDE_AVX512_ALIGN simde_int128  i128 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN simde_uint128 u128 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    #endif
    SIMDE_AVX512_ALIGN simde_float32  f32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN simde_float64  f64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int_fast32_t  i32f SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint_fast32_t u32f SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
  #else
    SIMDE_AVX512_ALIGN int8_t          i8[64];
    SIMDE_AVX512_ALIGN int16_t        i16[32];
    SIMDE_AVX512_ALIGN int32_t        i32[16];
    SIMDE_AVX512_ALIGN int64_t        i64[8];
    SIMDE_AVX512_ALIGN uint8_t         u8[64];
    SIMDE_AVX512_ALIGN uint16_t       u16[32];
    SIMDE_AVX512_ALIGN uint32_t       u32[16];
    SIMDE_AVX512_ALIGN uint64_t       u64[8];
    SIMDE_AVX512_ALIGN int_fast32_t  i32f[64 / sizeof(int_fast32_t)];
    SIMDE_AVX512_ALIGN uint_fast32_t u32f[64 / sizeof(uint_fast32_t)];
    #if defined(SIMDE_HAVE_INT128_)
    SIMDE_AVX512_ALIGN simde_int128  i128[4];
    SIMDE_AVX512_ALIGN simde_uint128 u128[4];
    #endif
    SIMDE_AVX512_ALIGN simde_float32  f32[16];
    SIMDE_AVX512_ALIGN simde_float64  f64[8];
  #endif

    SIMDE_AVX512_ALIGN simde__m128_private m128_private[4];
    SIMDE_AVX512_ALIGN simde__m128         m128[4];
    SIMDE_AVX512_ALIGN simde__m256_private m256_private[2];
    SIMDE_AVX512_ALIGN simde__m256         m256[2];

  #if defined(SIMDE_X86_AVX512F_NATIVE)
    SIMDE_AVX512_ALIGN __m512         n;
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned char)      altivec_u8[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned short)     altivec_u16[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned int)       altivec_u32[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed char)        altivec_i8[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed short)       altivec_i16[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed int)         altivec_i32[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(float)              altivec_f32[4];
    #if defined(SIMDE_POWER_ALTIVEC_P7_NATIVE)
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned long long) altivec_u64[4];
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed long long)   altivec_i64[4];
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(double)             altivec_f64[4];
    #endif
  #endif
} simde__m512_private;

typedef union {
  #if defined(SIMDE_VECTOR_SUBSCRIPT)
    SIMDE_AVX512_ALIGN int8_t          i8 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int16_t        i16 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int32_t        i32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int64_t        i64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint8_t         u8 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint16_t       u16 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint32_t       u32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint64_t       u64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    #if defined(SIMDE_HAVE_INT128_)
    SIMDE_AVX512_ALIGN simde_int128  i128 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN simde_uint128 u128 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    #endif
    SIMDE_AVX512_ALIGN simde_float32  f32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN simde_float64  f64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int_fast32_t  i32f SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint_fast32_t u32f SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
  #else
    SIMDE_AVX512_ALIGN int8_t          i8[64];
    SIMDE_AVX512_ALIGN int16_t        i16[32];
    SIMDE_AVX512_ALIGN int32_t        i32[16];
    SIMDE_AVX512_ALIGN int64_t        i64[8];
    SIMDE_AVX512_ALIGN uint8_t         u8[64];
    SIMDE_AVX512_ALIGN uint16_t       u16[32];
    SIMDE_AVX512_ALIGN uint32_t       u32[16];
    SIMDE_AVX512_ALIGN uint64_t       u64[8];
    #if defined(SIMDE_HAVE_INT128_)
    SIMDE_AVX512_ALIGN simde_int128  i128[4];
    SIMDE_AVX512_ALIGN simde_uint128 u128[4];
    #endif
    SIMDE_AVX512_ALIGN simde_float32  f32[16];
    SIMDE_AVX512_ALIGN simde_float64  f64[8];
    SIMDE_AVX512_ALIGN int_fast32_t  i32f[64 / sizeof(int_fast32_t)];
    SIMDE_AVX512_ALIGN uint_fast32_t u32f[64 / sizeof(uint_fast32_t)];
  #endif

    SIMDE_AVX512_ALIGN simde__m128d_private m128d_private[4];
    SIMDE_AVX512_ALIGN simde__m128d         m128d[4];
    SIMDE_AVX512_ALIGN simde__m256d_private m256d_private[2];
    SIMDE_AVX512_ALIGN simde__m256d         m256d[2];

  #if defined(SIMDE_X86_AVX512F_NATIVE)
    SIMDE_AVX512_ALIGN __m512d        n;
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned char)      altivec_u8[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned short)     altivec_u16[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned int)       altivec_u32[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed char)        altivec_i8[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed short)       altivec_i16[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed int)         altivec_i32[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(float)              altivec_f32[4];
    #if defined(SIMDE_POWER_ALTIVEC_P7_NATIVE)
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned long long) altivec_u64[4];
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed long long)   altivec_i64[4];
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(double)             altivec_f64[4];
    #endif
  #endif
} simde__m512d_private;

typedef union {
  #if defined(SIMDE_VECTOR_SUBSCRIPT)
    SIMDE_AVX512_ALIGN int8_t          i8 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int16_t        i16 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int32_t        i32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int64_t        i64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint8_t         u8 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint16_t       u16 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint32_t       u32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint64_t       u64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    #if defined(SIMDE_HAVE_INT128_)
    SIMDE_AVX512_ALIGN simde_int128  i128 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN simde_uint128 u128 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    #endif
    SIMDE_AVX512_ALIGN simde_float32  f32 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN simde_float64  f64 SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN int_fast32_t  i32f SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
    SIMDE_AVX512_ALIGN uint_fast32_t u32f SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
  #else
    SIMDE_AVX512_ALIGN int8_t          i8[64];
    SIMDE_AVX512_ALIGN int16_t        i16[32];
    SIMDE_AVX512_ALIGN int32_t        i32[16];
    SIMDE_AVX512_ALIGN int64_t        i64[8];
    SIMDE_AVX512_ALIGN uint8_t         u8[64];
    SIMDE_AVX512_ALIGN uint16_t       u16[32];
    SIMDE_AVX512_ALIGN uint32_t       u32[16];
    SIMDE_AVX512_ALIGN uint64_t       u64[8];
    SIMDE_AVX512_ALIGN int_fast32_t  i32f[64 / sizeof(int_fast32_t)];
    SIMDE_AVX512_ALIGN uint_fast32_t u32f[64 / sizeof(uint_fast32_t)];
    #if defined(SIMDE_HAVE_INT128_)
    SIMDE_AVX512_ALIGN simde_int128  i128[4];
    SIMDE_AVX512_ALIGN simde_uint128 u128[4];
    #endif
    SIMDE_AVX512_ALIGN simde_float32  f32[16];
    SIMDE_AVX512_ALIGN simde_float64  f64[8];
  #endif

    SIMDE_AVX512_ALIGN simde__m128i_private m128i_private[4];
    SIMDE_AVX512_ALIGN simde__m128i         m128i[4];
    SIMDE_AVX512_ALIGN simde__m256i_private m256i_private[2];
    SIMDE_AVX512_ALIGN simde__m256i         m256i[2];

  #if defined(SIMDE_X86_AVX512F_NATIVE)
    SIMDE_AVX512_ALIGN __m512i        n;
  #elif defined(SIMDE_POWER_ALTIVEC_P6_NATIVE)
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned char)      altivec_u8[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned short)     altivec_u16[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned int)       altivec_u32[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed char)        altivec_i8[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed short)       altivec_i16[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed int)         altivec_i32[4];
    SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(float)              altivec_f32[4];
    #if defined(SIMDE_POWER_ALTIVEC_P7_NATIVE)
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(unsigned long long) altivec_u64[4];
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(signed long long)   altivec_i64[4];
      SIMDE_ALIGN_TO_16 SIMDE_POWER_ALTIVEC_VECTOR(double)             altivec_f64[4];
    #endif
  #endif
} simde__m512i_private;

/* Intel uses the same header (immintrin.h) for everything AVX and
 * later.  If native aliases are enabled, and the machine has native
 * support for AVX imintrin.h will already have been included, which
 * means simde__m512* will already have been defined.  So, even
 * if the machine doesn't support AVX512F we need to use the native
 * type; it has already been defined.
 *
 * However, we also can't just assume that including immintrin.h does
 * actually define these.  It could be a compiler which supports AVX
 * but not AVX512F, such as GCC < 4.9 or VS < 2017.  That's why we
 * check to see if _MM_CMPINT_GE is defined; it's part of AVX512F,
 * so we assume that if it's present AVX-512F has already been
 * declared.
 *
 * Note that the choice of _MM_CMPINT_GE is deliberate; while GCC
 * uses the preprocessor to define all the _MM_CMPINT_* members,
 * in most compilers they are simply normal enum members.  However,
 * all compilers I've looked at use an object-like macro for
 * _MM_CMPINT_GE, which is defined to _MM_CMPINT_NLT.  _MM_CMPINT_NLT
 * is included in case a compiler does the reverse, though I haven't
 * run into one which does.
 *
 * As for the ICC check, unlike other compilers, merely using the
 * AVX-512 types causes ICC to generate AVX-512 instructions. */
#if (defined(_MM_CMPINT_GE) || defined(_MM_CMPINT_NLT)) && (defined(SIMDE_X86_AVX512F_NATIVE) || !defined(HEDLEY_INTEL_VERSION))
  typedef __m512 simde__m512;
  typedef __m512i simde__m512i;
  typedef __m512d simde__m512d;

  typedef __mmask8 simde__mmask8;
  typedef __mmask16 simde__mmask16;
#else
 #if defined(SIMDE_VECTOR_SUBSCRIPT)
   typedef simde_float32 simde__m512  SIMDE_AVX512_ALIGN SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
   typedef int_fast32_t  simde__m512i SIMDE_AVX512_ALIGN SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
   typedef simde_float64 simde__m512d SIMDE_AVX512_ALIGN SIMDE_VECTOR(64) SIMDE_MAY_ALIAS;
  #else
    typedef simde__m512_private  simde__m512;
    typedef simde__m512i_private simde__m512i;
    typedef simde__m512d_private simde__m512d;
  #endif

  typedef uint8_t simde__mmask8;
  typedef uint16_t simde__mmask16;
#endif

/* These are really part of AVX-512VL / AVX-512BW (in GCC __mmask32 is
 * in avx512vlintrin.h and __mmask64 is in avx512bwintrin.h, in clang
 * both are in avx512bwintrin.h), not AVX-512F.  However, we don't have
 * a good (not-compiler-specific) way to detect if these headers have
 * been included.  In compilers which support AVX-512F but not
 * AVX-512BW/VL (e.g., GCC 4.9) we need typedefs since __mmask{32,64)
 * won't exist.
 *
 * AFAICT __mmask{32,64} are always just typedefs to uint{32,64}_t
 * in all compilers, so it's safe to use these instead of typedefs to
 * __mmask{16,32}. If you run into a problem with this please file an
 * issue and we'll try to figure out a work-around. */
typedef uint32_t simde__mmask32;
typedef uint64_t simde__mmask64;

#if !defined(SIMDE_X86_AVX512F_NATIVE) && defined(SIMDE_ENABLE_NATIVE_ALIASES)
  #if !defined(HEDLEY_INTEL_VERSION)
    typedef simde__m512 __m512;
    typedef simde__m512i __m512i;
    typedef simde__m512d __m512d;
  #else
    #define __m512 simde__m512
    #define __m512i simde__m512i
    #define __m512d simde__m512d
  #endif
#endif

HEDLEY_STATIC_ASSERT(64 == sizeof(simde__m512), "simde__m512 size incorrect");
HEDLEY_STATIC_ASSERT(64 == sizeof(simde__m512_private), "simde__m512_private size incorrect");
HEDLEY_STATIC_ASSERT(64 == sizeof(simde__m512i), "simde__m512i size incorrect");
HEDLEY_STATIC_ASSERT(64 == sizeof(simde__m512i_private), "simde__m512i_private size incorrect");
HEDLEY_STATIC_ASSERT(64 == sizeof(simde__m512d), "simde__m512d size incorrect");
HEDLEY_STATIC_ASSERT(64 == sizeof(simde__m512d_private), "simde__m512d_private size incorrect");
#if defined(SIMDE_CHECK_ALIGNMENT) && defined(SIMDE_ALIGN_OF)
HEDLEY_STATIC_ASSERT(SIMDE_ALIGN_OF(simde__m512) == 32, "simde__m512 is not 32-byte aligned");
HEDLEY_STATIC_ASSERT(SIMDE_ALIGN_OF(simde__m512_private) == 32, "simde__m512_private is not 32-byte aligned");
HEDLEY_STATIC_ASSERT(SIMDE_ALIGN_OF(simde__m512i) == 32, "simde__m512i is not 32-byte aligned");
HEDLEY_STATIC_ASSERT(SIMDE_ALIGN_OF(simde__m512i_private) == 32, "simde__m512i_private is not 32-byte aligned");
HEDLEY_STATIC_ASSERT(SIMDE_ALIGN_OF(simde__m512d) == 32, "simde__m512d is not 32-byte aligned");
HEDLEY_STATIC_ASSERT(SIMDE_ALIGN_OF(simde__m512d_private) == 32, "simde__m512d_private is not 32-byte aligned");
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde__m512
simde__m512_from_private(simde__m512_private v) {
  simde__m512 r;
  simde_memcpy(&r, &v, sizeof(r));
  return r;
}

SIMDE_FUNCTION_ATTRIBUTES
simde__m512_private
simde__m512_to_private(simde__m512 v) {
  simde__m512_private r;
  simde_memcpy(&r, &v, sizeof(r));
  return r;
}

SIMDE_FUNCTION_ATTRIBUTES
simde__m512i
simde__m512i_from_private(simde__m512i_private v) {
  simde__m512i r;
  simde_memcpy(&r, &v, sizeof(r));
  return r;
}

SIMDE_FUNCTION_ATTRIBUTES
simde__m512i_private
simde__m512i_to_private(simde__m512i v) {
  simde__m512i_private r;
  simde_memcpy(&r, &v, sizeof(r));
  return r;
}

SIMDE_FUNCTION_ATTRIBUTES
simde__m512d
simde__m512d_from_private(simde__m512d_private v) {
  simde__m512d r;
  simde_memcpy(&r, &v, sizeof(r));
  return r;
}

SIMDE_FUNCTION_ATTRIBUTES
simde__m512d_private
simde__m512d_to_private(simde__m512d v) {
  simde__m512d_private r;
  simde_memcpy(&r, &v, sizeof(r));
  return r;
}

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_X86_AVX512_TYPES_H) */
