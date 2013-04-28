//
//  det_rng.c
//  p1
//
//  Created by Justin Bowes on 2013-02-15.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <math.h>

#include "xpl_hash.h"
#include "xpl_vec.h"

#include "random/det_rng.h"

void rng_seq_init_ints(int *ints, size_t size, rng_seq_t *seq_init) {
    union {
        uint64_t seed_ui64;
        int seed_int[2];
    } seed_data;
    seed_data.seed_int[0] = XPL_HASH_INIT;
    seed_data.seed_int[1] = 2 * XPL_HASH_INIT + 7;
    int index;
    for (size_t i = 0; i < size; ++i) {
        index = i % 2;
        seed_data.seed_int[index] = xpl_hashi(ints[i], seed_data.seed_int[index]);
    }
    seq_init->r = seed_data.seed_ui64;
    
    // warm up
    for (size_t i = 0; i < 3; ++i) {
        rng_ui64(seq_init);
    }
}

uint64_t rng_ui64(rng_seq_t *seq) {
    uint64_t r = seq->r;
	r ^= (r >> 17);
	r ^= (r << 31);
	r ^= (r >>  8);
    seq->r = r;
    return r;
}

double rng_double(rng_seq_t *seq) {
    uint64_t i = rng_ui64(seq);
    double d = ((double)i) / ((double)UINT64_MAX);
    return d;
}

float rng_float(rng_seq_t *seq) {
	float f = (float)rng_double(seq);
    return f;
}

double rng_range(rng_seq_t *seq, double x1, double x2) {
	double min = (x1 < x2 ? x1 : x2), max = (x2 > x1 ? x2 : x1);
	return (max - min) * rng_double(seq) + min;
}

double rng_pareto(rng_seq_t *seq, double shape) {
    double u;
    do {
        u = rng_double(seq);
    } while (u == 0. || u == 1.);
    
    return pow(u, -1. / shape) - 1.;
}

void rng_bytes_from_set(rng_seq_t *seq, uint8_t *fill, size_t length, uint8_t *bytes, size_t set_len) {
	for (size_t i = 0; i < length; ++i) {
		fill[i] = bytes[(size_t)(rng_double(seq) * set_len)];
	}
    
	fill[length] = 0;
}

void rng_string(rng_seq_t *seq, char *fill, size_t chars) {
	static const char select[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	return rng_bytes_from_set(seq, (uint8_t*)fill, chars, (uint8_t *)&select[0], sizeof(select));
}

size_t rng_select_from_bins(rng_seq_t *seq, float *bins, size_t bin_count) {
    // get sum of bins
    float total = 0.f;
    for (int i = 0; i < bin_count; ++i) {
        total += bins[i];
    }
    float select = rng_range(seq, 0.f, total);
    float accum = 0.f;
    for (int i = 0; i < bin_count - 1; ++i) {
        accum += bins[i];
        if (accum > select) return i;
    }
    return (int)bin_count - 1;
}

void rng_doubles_range(rng_seq_t *seq, double min, double max, double *doubles, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        doubles[i] = rng_range(seq, min, max);
    }
}

void rng_floats_range(rng_seq_t *seq, float min, float max, float *floats, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        floats[i] = (float)rng_range(seq, min, max);
    }
}

xvec3 rng_xvec3(rng_seq_t *seq, float min, float max) {
    xvec3 r;
    rng_floats_range(seq, min, max, r.data, 3);
    return r;
}

xquat rng_orientation_quat(rng_seq_t *seq) {
    xvec3 axis = rng_xvec3(seq, 0.f, 1.f);
    float angle = rng_float(seq);
    xquat orientation;
    xquat_from_axis_angle_rad(axis, angle, &orientation);
    return orientation;
}
