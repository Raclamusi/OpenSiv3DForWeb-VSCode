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
 *   2020      Sean Maher <seanptmaher@gmail.com> (Copyright owned by Google, LLC)
 */

#if !defined(SIMDE_ARM_NEON_CAGT_H)
#define SIMDE_ARM_NEON_CAGT_H

#include "types.h"

HEDLEY_DIAGNOSTIC_PUSH
SIMDE_DISABLE_UNWANTED_DIAGNOSTICS
SIMDE_BEGIN_DECLS_

SIMDE_FUNCTION_ATTRIBUTES
uint32_t
simde_vcagts_f32(simde_float32_t a, simde_float32_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcagts_f32(a, b);
  #else
    return (simde_math_fabsf(a) > simde_math_fabsf(b)) ? ~UINT32_C(0) : UINT32_C(0);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcagts_f32
  #define vcagts_f32(a, b) simde_vcagts_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
uint64_t
simde_vcagtd_f64(simde_float64_t a, simde_float64_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcagtd_f64(a, b);
  #else
    return (simde_math_fabs(a) > simde_math_fabs(b)) ? ~UINT64_C(0) : UINT64_C(0);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcagtd_f64
  #define vcagtd_f64(a, b) simde_vcagtd_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x2_t
simde_vcagt_f32(simde_float32x2_t a, simde_float32x2_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcagt_f32(a, b);
  #else
    simde_uint32x2_private r_;
    simde_float32x2_private
      a_ = simde_float32x2_to_private(a),
      b_ = simde_float32x2_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcagts_f32(a_.values[i], b_.values[i]);
    }

    return simde_uint32x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcagt_f32
  #define vcagt_f32(a, b) simde_vcagt_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x1_t
simde_vcagt_f64(simde_float64x1_t a, simde_float64x1_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcagt_f64(a, b);
  #else
    simde_uint64x1_private r_;
    simde_float64x1_private
      a_ = simde_float64x1_to_private(a),
      b_ = simde_float64x1_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcagtd_f64(a_.values[i], b_.values[i]);
    }

    return simde_uint64x1_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcagt_f64
  #define vcagt_f64(a, b) simde_vcagt_f64((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint32x4_t
simde_vcagtq_f32(simde_float32x4_t a, simde_float32x4_t b) {
  #if defined(SIMDE_ARM_NEON_A32V7_NATIVE)
    return vcagtq_f32(a, b);
  #elif defined(SIMDE_WASM_SIMD128_NATIVE)
    return wasm_f32x4_gt(wasm_f32x4_abs(a), wasm_f32x4_abs(b));
  #else
    simde_uint32x4_private r_;
    simde_float32x4_private
      a_ = simde_float32x4_to_private(a),
      b_ = simde_float32x4_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcagts_f32(a_.values[i], b_.values[i]);
    }

    return simde_uint32x4_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A32V7_ENABLE_NATIVE_ALIASES)
  #undef vcagtq_f32
  #define vcagtq_f32(a, b) simde_vcagtq_f32((a), (b))
#endif

SIMDE_FUNCTION_ATTRIBUTES
simde_uint64x2_t
simde_vcagtq_f64(simde_float64x2_t a, simde_float64x2_t b) {
  #if defined(SIMDE_ARM_NEON_A64V8_NATIVE)
    return vcagtq_f64(a, b);
  #elif defined(SIMDE_WASM_SIMD128_NATIVE)
    return wasm_f64x2_gt(wasm_f64x2_abs(a), wasm_f64x2_abs(b));
  #else
    simde_uint64x2_private r_;
    simde_float64x2_private
      a_ = simde_float64x2_to_private(a),
      b_ = simde_float64x2_to_private(b);

    SIMDE_VECTORIZE
    for (size_t i = 0 ; i < (sizeof(r_.values) / sizeof(r_.values[0])) ; i++) {
      r_.values[i] = simde_vcagtd_f64(a_.values[i], b_.values[i]);
    }

    return simde_uint64x2_from_private(r_);
  #endif
}
#if defined(SIMDE_ARM_NEON_A64V8_ENABLE_NATIVE_ALIASES)
  #undef vcagtq_f64
  #define vcagtq_f64(a, b) simde_vcagtq_f64((a), (b))
#endif

SIMDE_END_DECLS_
HEDLEY_DIAGNOSTIC_POP

#endif /* !defined(SIMDE_ARM_NEON_CAGT_H) */
