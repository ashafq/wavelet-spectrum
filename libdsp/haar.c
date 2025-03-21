/**
 * Copyright (c) 2025 Ayan Shafqat <ayan.x.shafqat@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#define SIZE_C(x) ((size_t)(x))

// Define a macro to swap two values
#define SWAP(a, b)                                                                       \
  do {                                                                                   \
    __typeof__(a) temp = a;                                                              \
    (a) = (b);                                                                           \
    (b) = temp;                                                                          \
  } while (0)

// Macro to get the minimum of two values
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Macro to get the maximum of two values
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Macro to clamp to a range
#define CLAMP(A, MIN_VAL, MAX_VAL) (MAX(MIN(MAX_VAL, A), MIN_VAL))

// Haar transform scale
#define HAAR_SCALE (M_SQRT1_2)

#define BITREV_RESHUFFLE(DATA, RADIX, LEN)                                               \
  do {                                                                                   \
    size_t shift = SIZE_C(32) - (RADIX);                                                 \
    for (size_t i = 0; i < (LEN); ++i) {                                                 \
      size_t rev = revbin32(i) >> shift;                                                 \
                                                                                         \
      if (i < rev) {                                                                     \
        SWAP(DATA[i], DATA[rev]);                                                        \
      }                                                                                  \
    }                                                                                    \
  } while (0);

#define SWAP_BITS(b, m, n) ((((b) >> (n)) & (m)) | (((b) & (m)) << (n)))

static inline uint32_t revbin32(uint32_t x) {
#if defined(__clang__)
  x = __builtin_bitreverse32(x);
#else
  {
    const uint32_t BIT = UINT32_C(0x55555555);
    const uint32_t PAIR = UINT32_C(0x33333333);
    const uint32_t NIBBLE = UINT32_C(0x0f0f0f0f);

    x = SWAP_BITS(x, BIT, 1);
    x = SWAP_BITS(x, PAIR, 2);
    x = SWAP_BITS(x, NIBBLE, 4);

#if defined(__GNUC__)
    x = __builtin_bswap32(x);
#else
    {
      const uint32_t BYTE = UINT32_C(0x00ff00ff);
      x = SWAP_BITS(x, BYTE, 8);
      x = (x >> 16) | (x << 16);
    }
#endif /* defined(__GNUC__) */
  }
#endif /* defined (__clang__) */
  return x;
}

#undef SWAP_BITS

void haar_forward_q15(int16_t *data, size_t radix) {
  size_t len = SIZE_C(1) << radix;
  size_t hlen = len >> SIZE_C(1);
  size_t skip = SIZE_C(1);

  for (size_t i = 0; i < radix; ++i) {
    for (size_t j = 0; j < hlen; ++j) {
      size_t odd, even;
      int32_t a, b, x, y;

      even = ((2 * j) + 0) * skip;
      odd = ((2 * j) + 1) * skip;

      a = data[even];
      b = data[odd];

      x = ((a + b) + 1) >> 1;
      y = (a - b);

      data[even] = (int16_t)x;
      data[odd] = (int16_t)y;
    }

    skip <<= 1;
    hlen >>= 1;
  }

  BITREV_RESHUFFLE(data, radix, len);
}

void haar_inverse_q15(int16_t *data, size_t radix) {
  size_t len = SIZE_C(1) << radix;
  size_t hlen = SIZE_C(1);
  size_t skip = len >> SIZE_C(1);

  BITREV_RESHUFFLE(data, radix, len);

  for (size_t i = 0; i < radix; ++i) {
    for (size_t j = 0; j < hlen; ++j) {
      size_t odd, even;
      int32_t a, b, x, y;

      even = ((2 * j) + 0) * skip;
      odd = ((2 * j) + 1) * skip;

      a = data[even];
      b = data[odd];

      x = ((a << 1) + b) >> 1;
      y = ((a << 1) - b) >> 1;

      data[even] = (int16_t)x;
      data[odd] = (int16_t)y;
    }

    skip >>= 1;
    hlen <<= 1;
  }
}

void haar_forward_f32(float *data, size_t radix) {
  size_t len = SIZE_C(1) << radix;
  size_t hlen = len >> SIZE_C(1);
  size_t skip = SIZE_C(1);

  for (size_t i = 0; i < radix; ++i) {
    for (size_t j = 0; j < hlen; ++j) {
      size_t even = ((2 * j) + 0) * skip;
      size_t odd = ((2 * j) + 1) * skip;

      float a = data[even];
      float b = data[odd];

      float x = (a + b) * HAAR_SCALE;
      float y = (a - b) * HAAR_SCALE;

      data[even] = x;
      data[odd] = y;
    }

    skip <<= 1;
    hlen >>= 1;
  }

  BITREV_RESHUFFLE(data, radix, len);
}

void haar_inverse_f32(float *data, size_t radix) {
  size_t len = SIZE_C(1) << radix;
  size_t hlen = SIZE_C(1);
  size_t skip = len >> SIZE_C(1);

  BITREV_RESHUFFLE(data, radix, len);

  for (size_t i = 0; i < radix; ++i) {
    for (size_t j = 0; j < hlen; ++j) {
      size_t even = ((2 * j) + 0) * skip;
      size_t odd = ((2 * j) + 1) * skip;

      float a = data[even];
      float b = data[odd];

      float x = (a + b) * HAAR_SCALE;
      float y = (a - b) * HAAR_SCALE;

      data[even] = x;
      data[odd] = y;
    }

    skip >>= 1;
    hlen <<= 1;
  }
}
