//
//  star_properties.h
//  elsie
//
//  Created by Justin Bowes on 2012-09-08.
//  Copyright (c) 2012 Informi Software Inc. All rights reserved.
//

#ifndef elsie_star_properties_h
#define elsie_star_properties_h

#ifdef __cplusplus
#define __externcstart  extern "C" {
#define __externcend    }
__externcstart
#endif

#include <xpl_vec.h>

typedef enum star_type {
    st_subdwarf        = 6,
    st_main_sequence   = 5,
    st_subgiant        = 4,
    st_giant           = 3,
    st_bright_giant    = 2,
    st_supergiant_ib   = 1,
    st_supergiant_ia   = 0, // fucking harvard
    st_white_dwarf     = 10,
    st_pulsar          = 11
} star_type_t;

typedef struct star {
    float       smasses;
    float       sradii;
    int         temperature_kelvin;
    xvec3       color;
    float       abs_mag;
    float       apparent_mag;
    float       solar_luminosity;
    star_type_t type;
} star_t;

float absolute_magnitude_for_solar_luminosity(const float solar_luminosity);

float apparent_magnitude_for_solar_luminosity_distance(const float solar_luminosity, const float distance_ly);

float solar_luminosity_for_solar_masses(const float solar_masses);

float solar_radii_for_solar_luminosity_and_temperature(const float solar_luminosity, const float kelvin);

float eddington_luminosity(const float solar_masses);

#ifdef __cplusplus
__externcend
#endif
    
#endif
