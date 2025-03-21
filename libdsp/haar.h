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

#ifndef A5CODEC_HAAR_H_
#define A5CODEC_HAAR_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void haar_forward_f32(float *data, size_t radix);
void haar_inverse_f32(float *data, size_t radix);
void haar_forward_q15(int16_t *data, size_t radix);
void haar_inverse_q15(int16_t *data, size_t radix);

void haar_spectrum_l1(float *spectrum, const float *data, size_t radix);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* A5CODEC_HAAR_H_ */
