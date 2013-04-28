//
//  xpl_rand.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-23.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_rand_h
#define xpl_osx_xpl_rand_h

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "xpl_platform.h"
#include "xpl_vec.h"

// ------------- mathy --------------------

XPLINLINE float xpl_frand(void) {
	return ((float)rand() / RAND_MAX);
};

XPLINLINE float xpl_frand_range(float x1, float x2) {
	float min = (x1 < x2 ? x1 : x2), max = (x2 > x1 ? x2 : x1);
	return (max - min) * xpl_frand() + min;
};

XPLINLINE int xpl_irand_range(int min, int max) {
	return (rand() % (max - min)) + min;
}

float xpl_frand_pareto(float shape);
float xpl_frand_gaussian(void);

void xpl_rand_string(char *fill, size_t chars);
void xpl_rand_bytes_from_set(uint8_t *fill, size_t fill_bytes, uint8_t *select_bytes, size_t select_len);
int xpl_rand_selection_from_bins(float *bins, size_t bin_count);
void xpl_random_floats(float *floats, size_t count, float min, float max);

xvec2 xpl_rand_xvec2(float min, float max);
xvec3 xpl_rand_xvec3(float min, float max);
xquat xpl_rand_orientation_quat(void);

float xpl_perlin_noise2(xvec2 vec);
float xpl_perlin_noise3(xvec3 vec);

#endif
