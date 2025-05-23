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
 *   2020      Evan Nemerson <evan@nemerson.com>
 *   2020      Sean Maher <seanptmaher@gmail.com>
 */

#if !defined(SIMDE_ARM_NEON_ST3_H)
#define SIMDE_ARM_NEON_ST3_H

#include "types.h"
#include "st1.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

#if !defined(SIMDE_BUG_INTEL_857088)

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_f32(simde_float32_t *ptr, simde_float32x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_f32(ptr, val);
  #else
    simde_float32_t buf[6];
    simde_float32x2_private a_[3] = { simde_float32x2_to_private(val.val[0]), simde_float32x2_to_private(val.val[1]), simde_float32x2_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3_f32
  #define vst3_f32(a, b) simde_vst3_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_f64(simde_float64_t *ptr, simde_float64x1x3_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst3_f64(ptr, val);
  #else
    simde_float64_t buf[3];
    simde_float64x1_private a_[3] = { simde_float64x1_to_private(val.val[0]), simde_float64x1_to_private(val.val[1]), simde_float64x1_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst3_f64
  #define vst3_f64(a, b) simde_vst3_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_s8(int8_t *ptr, simde_int8x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_s8(ptr, val);
  #else
    int8_t buf[24];
    simde_int8x8_private a_[3] = { simde_int8x8_to_private(val.val[0]), simde_int8x8_to_private(val.val[1]), simde_int8x8_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3_s8
  #define vst3_s8(a, b) simde_vst3_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_s16(int16_t *ptr, simde_int16x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_s16(ptr, val);
  #else
    int16_t buf[12];
    simde_int16x4_private a_[3] = { simde_int16x4_to_private(val.val[0]), simde_int16x4_to_private(val.val[1]), simde_int16x4_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3_s16
  #define vst3_s16(a, b) simde_vst3_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_s32(int32_t *ptr, simde_int32x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_s32(ptr, val);
  #else
    int32_t buf[6];
    simde_int32x2_private a_[3] = { simde_int32x2_to_private(val.val[0]), simde_int32x2_to_private(val.val[1]), simde_int32x2_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3_s32
  #define vst3_s32(a, b) simde_vst3_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_s64(int64_t *ptr, simde_int64x1x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_s64(ptr, val);
  #else
    int64_t buf[3];
    simde_int64x1_private a_[3] = { simde_int64x1_to_private(val.val[0]), simde_int64x1_to_private(val.val[1]), simde_int64x1_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst3_s64
  #define vst3_s64(a, b) simde_vst3_s64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_u8(uint8_t *ptr, simde_uint8x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_u8(ptr, val);
  #else
    uint8_t buf[24];
    simde_uint8x8_private a_[3] = { simde_uint8x8_to_private(val.val[0]), simde_uint8x8_to_private(val.val[1]), simde_uint8x8_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3_u8
  #define vst3_u8(a, b) simde_vst3_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_u16(uint16_t *ptr, simde_uint16x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_u16(ptr, val);
  #else
    uint16_t buf[12];
    simde_uint16x4_private a_[3] = { simde_uint16x4_to_private(val.val[0]), simde_uint16x4_to_private(val.val[1]), simde_uint16x4_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3_u16
  #define vst3_u16(a, b) simde_vst3_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_u32(uint32_t *ptr, simde_uint32x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_u32(ptr, val);
  #else
    uint32_t buf[6];
    simde_uint32x2_private a_[3] = { simde_uint32x2_to_private(val.val[0]), simde_uint32x2_to_private(val.val[1]), simde_uint32x2_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3_u32
  #define vst3_u32(a, b) simde_vst3_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3_u64(uint64_t *ptr, simde_uint64x1x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3_u64(ptr, val);
  #else
    uint64_t buf[3];
    simde_uint64x1_private a_[3] = { simde_uint64x1_to_private(val.val[0]), simde_uint64x1_to_private(val.val[1]), simde_uint64x1_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst3_u64
  #define vst3_u64(a, b) simde_vst3_u64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_f32(simde_float32_t *ptr, simde_float32x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3q_f32(ptr, val);
  #else
    simde_float32_t buf[12];
    simde_float32x4_private a_[3] = { simde_float32x4_to_private(val.val[0]), simde_float32x4_to_private(val.val[1]), simde_float32x4_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3q_f32
  #define vst3q_f32(a, b) simde_vst3q_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_f64(simde_float64_t *ptr, simde_float64x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst3q_f64(ptr, val);
  #else
    simde_float64_t buf[6];
    simde_float64x2_private a_[3] = { simde_float64x2_to_private(val.val[0]), simde_float64x2_to_private(val.val[1]), simde_float64x2_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst3q_f64
  #define vst3q_f64(a, b) simde_vst3q_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_s8(int8_t *ptr, simde_int8x16x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3q_s8(ptr, val);
  #else
    int8_t buf[48];
    simde_int8x16_private a_[3] = { simde_int8x16_to_private(val.val[0]), simde_int8x16_to_private(val.val[1]), simde_int8x16_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3q_s8
  #define vst3q_s8(a, b) simde_vst3q_s8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_s16(int16_t *ptr, simde_int16x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3q_s16(ptr, val);
  #else
    int16_t buf[24];
    simde_int16x8_private a_[3] = { simde_int16x8_to_private(val.val[0]), simde_int16x8_to_private(val.val[1]), simde_int16x8_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3q_s16
  #define vst3q_s16(a, b) simde_vst3q_s16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_s32(int32_t *ptr, simde_int32x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3q_s32(ptr, val);
  #else
    int32_t buf[12];
    simde_int32x4_private a_[3] = { simde_int32x4_to_private(val.val[0]), simde_int32x4_to_private(val.val[1]), simde_int32x4_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3q_s32
  #define vst3q_s32(a, b) simde_vst3q_s32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_s64(int64_t *ptr, simde_int64x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst3q_s64(ptr, val);
  #else
    int64_t buf[6];
    simde_int64x2_private a_[3] = { simde_int64x2_to_private(val.val[0]), simde_int64x2_to_private(val.val[1]), simde_int64x2_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst3q_s64
  #define vst3q_s64(a, b) simde_vst3q_s64((a), (b))
#endif


SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_u8(uint8_t *ptr, simde_uint8x16x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3q_u8(ptr, val);
  #else
    uint8_t buf[48];
    simde_uint8x16_private a_[3] = { simde_uint8x16_to_private(val.val[0]), simde_uint8x16_to_private(val.val[1]), simde_uint8x16_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3q_u8
  #define vst3q_u8(a, b) simde_vst3q_u8((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_u16(uint16_t *ptr, simde_uint16x8x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3q_u16(ptr, val);
  #else
    uint16_t buf[24];
    simde_uint16x8_private a_[3] = { simde_uint16x8_to_private(val.val[0]), simde_uint16x8_to_private(val.val[1]), simde_uint16x8_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3q_u16
  #define vst3q_u16(a, b) simde_vst3q_u16((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_u32(uint32_t *ptr, simde_uint32x4x3_t val) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    vst3q_u32(ptr, val);
  #else
    uint32_t buf[12];
    simde_uint32x4_private a_[3] = { simde_uint32x4_to_private(val.val[0]), simde_uint32x4_to_private(val.val[1]), simde_uint32x4_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vst3q_u32
  #define vst3q_u32(a, b) simde_vst3q_u32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
void
simde_vst3q_u64(uint64_t *ptr, simde_uint64x2x3_t val) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    vst3q_u64(ptr, val);
  #else
    uint64_t buf[6];
    simde_uint64x2_private a_[3] = { simde_uint64x2_to_private(val.val[0]), simde_uint64x2_to_private(val.val[1]), simde_uint64x2_to_private(val.val[2]) };
    for (size_t i = 0; i < (sizeof(val.val[0]) / sizeof(*ptr)) * 3 ; i++) {
      buf[i] = a_[i % 3].values[i / 3];
    }
    simde_memcpy(ptr, buf, sizeof(buf));
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vst3q_u64
  #define vst3q_u64(a, b) simde_vst3q_u64((a), (b))
#endif

#endif /* !defined(SIMDE_BUG_INTEL_857088) */

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_ST3_H) */
