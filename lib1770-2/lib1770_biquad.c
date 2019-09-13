/*
 * lib1770_biquad.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
 * Copyright (C) 2019 Tilde Joy <tilde@ultros.pro>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

#include "lib1770.h"

void
lib1770_biquad_get_ps(const lib1770_biquad_t *biquad, lib1770_biquad_ps_t *ps) 
{
	double x11 = biquad->a1-2;
	double x12 = biquad->a1;
	double x1 = -biquad->a1-2;

	double x21 = biquad->a2-1;
	double x22 = biquad->a2+1;
	double x2 = -biquad->a2+1;

	double dx = x22 * x11 - x12 * x21;
	double k_sq = (x22 * x1 - x12 * x2) / dx;
	double k_by_q = (x11 * x2 - x21 * x1) / dx;
	double a0 = 1.0 + k_by_q + k_sq;

	ps->k = sqrt(k_sq);
	ps->q = ps->k / k_by_q;
	ps->vb = 0.5 * a0 * (biquad->b0 - biquad->b2) / k_by_q;
	ps->vl = 0.25 * a0 * (biquad->b0 + biquad->b1 + biquad->b2) /k_sq;
	ps->vh = 0.25 * a0 * (biquad->b0 - biquad->b1 + biquad->b2);
}

lib1770_biquad_t *
lib1770_biquad_requantize(lib1770_biquad_t *out, const lib1770_biquad_t *in)
{
	if (in->samplerate == out->samplerate)
		*out = *in;
	else {
		lib1770_biquad_ps_t ps;
		double k, k_sq, k_by_q, a0;
		double den_tmp;

		lib1770_biquad_get_ps(in, &ps);
 		k = tan((in->samplerate/out->samplerate) * atan(ps.k));
		k_sq = k * k;
		k_by_q= k / ps.q;
		a0= 1.0 + k_by_q + k_sq;

		out->a1 = LIB1770_DEN((2.0 * (k_sq-1.0)) / a0);
		out->a2 = LIB1770_DEN((1.0-k_by_q+k_sq)/a0);
 		out->b0 = LIB1770_DEN((ps.vh + ps.vb * k_by_q + ps.vl * k_sq) \
		    / a0);
		out->b1 = LIB1770_DEN((2.0 * (ps.vl * k_sq - ps.vh)) / a0);
		out->b2 = LIB1770_DEN((ps.vh - ps.vb * k_by_q + ps.vl * k_sq) \
		    / a0);
	}

	return out;
}
