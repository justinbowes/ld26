//
//  rand.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-08.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include "xpl_rand.h"

float xpl_frand_pareto(const float shape) {
	// Verified and fixed.

	float u;
	do {
		u = xpl_frand();
	} while (u == 0.f || u == 1.f);

	return 1.f / powf(1.f - u, 1.f - shape);
}

float xpl_frand_gaussian() {
    float x1, x2, rad;
	do {
		x1 = 2.0f * xpl_frand() - 1.0f;
		x2 = 2.0f * xpl_frand() - 1.0f;
		rad = x1 * x1 + x2 * x2;
	} while(rad >= 1 || rad == 0);
    
	float c = sqrtf(-2.0f * logf(rad) / rad);
    
	return x1 * c;
}

// Creates a null-terminated string of (length - 1) random characters.
void xpl_rand_string(char *fill, size_t length) {
	assert(length);
	static const char select[] = "0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
	xpl_rand_bytes_from_set((uint8_t*)fill, length - 1, (uint8_t *)&select[0], sizeof(select));
	fill[length] = 0;
}

void xpl_rand_bytes_from_set(uint8_t *fill, size_t length, uint8_t *bytes, size_t set_len) {
	for (size_t i = 0; i < length; ++i) {
		float r = xpl_frand();
		size_t index = (size_t)(r * set_len);
		fill[i] = bytes[index];
	}
}

int xpl_rand_selection_from_bins(float *bins, size_t bin_count) {
    // get sum of bins
    float total = 0.f;
    for (int i = 0; i < bin_count; ++i) {
        total += bins[i];
    }
    float select = xpl_frand_range(0.f, total);
    float accum = 0.f;
    for (int i = 0; i < bin_count - 1; ++i) {
        accum += bins[i];
        if (accum > select) return i;
    }
    return (int)bin_count - 1;
}

void xpl_random_floats(float *floats, size_t count, float min, float max) {
    for (size_t i = 0; i < count; ++i) {
        floats[i] = xpl_frand_range(min, max);
    }
}

xvec2 xpl_rand_xvec2(float min, float max) {
	xvec2 r;
	xpl_random_floats(r.data, 2, min, max);
	return r;
}

xvec3 xpl_rand_xvec3(float min, float max) {
    xvec3 r;
    xpl_random_floats(r.data, 3, min, max);
    return r;
}

xquat xpl_rand_orientation_quat() {
    xvec3 orientation_axis = xpl_rand_xvec3(0.f, 1.f);
    float orientation_angle = xpl_frand_range(0.f, M_2PI);
    xquat orientation;
    xquat_from_axis_angle_rad(orientation_axis, orientation_angle, &orientation);
    return orientation;
}
