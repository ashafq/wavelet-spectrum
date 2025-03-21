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

#include "haar.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static float db20(float x);

void haar_spectrum_l1(float *spectrum, const float *data, size_t radix) {
  size_t len = 1LL << radix;

  memcpy(spectrum, data, len * sizeof(float));
  haar_forward_f32(spectrum, radix);

  /* Averaging bins: 1, 2, 4, 8, ... to radix */
  size_t start = 0;
  size_t band_size = 1;

  for (size_t bin = 0; bin < radix; ++bin) {

    float sum = 0.0F;
    for (size_t i = 0; i < band_size; ++i) {
      sum += fabsf(spectrum[start + i]);
    }

    /*
     * In fixed point, this would be
     *
     * sum >>= bin;
     */
    spectrum[bin] = db20(ldexpf(sum, -bin));

    /* Next bin has twice as many bands */
    start += band_size;
    band_size <<= 1;
  }
}

static float db20(float x) {
  x = fabsf(x);
  if (x < 3.981071705534969e-07F) {
    return -128.0F;
  } else {
    return 20.0F * log10(x);
  }
}
