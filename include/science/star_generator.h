//
//  star_generator.h
//  elsie
//
//  Created by Justin Bowes on 2012-09-08.
//  Copyright (c) 2012 Informi Software Inc. All rights reserved.
//

#ifndef elsie_star_generator_h
#define elsie_star_generator_h

#ifdef __cplusplus
#define __externcstart  extern "C" {
#define __externcend    }
__externcstart
#endif

#include "star_properties.h"

#include "random/det_rng.h"

void star_randomize(star_t *modify, rng_seq_t *rng);
star_type_t star_type_for_luminosity(const float luminosity, const int temp_k);

#ifdef __cplusplus
__externcend
#endif
    
#endif
