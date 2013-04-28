//
//  det_rng.h
//  p1
//
//  Created by Justin Bowes on 2013-02-15.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_det_rng_h
#define p1_det_rng_h

#include "xpl.h"

typedef struct rng_seq {
    uint64_t r;
} rng_seq_t;

// Initialize the RNG sequence with a series of 32-bit integers.
void rng_seq_init_ints(int *ints, size_t size, rng_seq_t *seq_init);

uint64_t rng_ui64(rng_seq_t *seq);
double rng_double(rng_seq_t *seq);
float rng_float(rng_seq_t *seq);
double rng_range(rng_seq_t *seq, double x1, double x2);
double rng_pareto(rng_seq_t *seq, double shape);
void rng_bytes_from_set(rng_seq_t *seq, uint8_t *fill, size_t length, uint8_t *bytes, size_t set_len);
void rng_string(rng_seq_t *seq, char *fill, size_t chars);
size_t rng_select_from_bins(rng_seq_t *seq, float *bins, size_t bin_count);
void rng_doubles_range(rng_seq_t *seq, double min, double max, double *doubles, size_t count);
void rng_floats_range(rng_seq_t *seq, float min, float max, float *floats, size_t count);
xvec3 rng_xvec3(rng_seq_t *seq, float min, float max);
xquat rng_orientation_quat(rng_seq_t *seq);
#endif
