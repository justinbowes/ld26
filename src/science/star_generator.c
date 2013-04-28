//
//  star_generator.c
//  elsie
//
//  Created by Justin Bowes on 2012-09-08.
//  Copyright (c) 2012 Informi Software Inc. All rights reserved.
//

#include <math.h>

#include "xpl.h"
#include "xpl_memory.h"
#include "xpl_rand.h"

#include "science/celestial_constants.h"
#include "science/blackbody_srgb.h"
#include "science/star_properties.h"
#include "science/star_generator.h"

#define PARETO_ALPHA 2.45f

void star_randomize(star_t *star, rng_seq_t *rng) {
    float smass;
    do {
        smass = rng_pareto(rng, PARETO_ALPHA);
    } while ((smass < 0.08f) || (smass > 70.f));
    int kelvin = (int)(powf(powf(smass, 2.5f), 0.25f) * SOLAR_TEMPERATURE);
    
    float luminosity = solar_luminosity_for_solar_masses(smass);
    
    star_type_t type = star_type_for_luminosity(luminosity, kelvin);
    
    xvec4 rgbi = blackbody_rgbi(kelvin);
    
    // Assign props
    star->smasses = smass;
    star->sradii = solar_radii_for_solar_luminosity_and_temperature(luminosity, kelvin);
    star->abs_mag = absolute_magnitude_for_solar_luminosity(luminosity);
    star->color = rgbi.rgb;
    star->solar_luminosity = luminosity;
    star->temperature_kelvin = kelvin;
    star->type = type;
}

/**
 * H-R mapping basically
 */
star_type_t star_type_for_luminosity(const float luminosity, const int temp_k) {
    if (luminosity >= 16000.) return st_supergiant_ia;
    if (luminosity >= 5000.) return st_supergiant_ib;
    if (luminosity >= 1000.) return st_bright_giant;
    
    if ((luminosity >= 500.) && (temp_k < 8000)) return st_bright_giant;
    if ((luminosity >= 300.) && (temp_k < 6000)) return st_bright_giant;
    
    if (luminosity > 80.) {
        if (temp_k < 8200)          return st_giant;
        else if (temp_k < 12000)    return st_subgiant;
        else                        return st_main_sequence;
    }
    
    if (luminosity > 60.) {
        if (temp_k < 6200)          return st_giant;
        else if (temp_k < 8000)     return st_subgiant;
        else                        return st_main_sequence;
    }
    
    if (luminosity > 30.) {
        if (temp_k < 6600)          return st_subgiant;
        else                        return st_main_sequence;
    }
    
    if (luminosity > 10.) {
        if (temp_k < 6000)          return st_subgiant;
        else                        return st_main_sequence;
    }
    
    if (luminosity > 0.01) {
        return st_main_sequence;
    }
    
    if (temp_k > 4500) return st_white_dwarf;
    return st_subdwarf;
}

