//
//  star_properties.c
//  elsie
//
//  Created by Justin Bowes on 2012-09-08.
//  Copyright (c) 2012 Informi Software Inc. All rights reserved.
//

#include <math.h>
#include <stdlib.h>

#include "science/star_properties.h"
#include "science/celestial_constants.h"

float absolute_magnitude_for_solar_luminosity(const float solar_luminosity) {
    return SOLAR_ABSOLUTE_MAGNITUDE - (2.5f * log10f(solar_luminosity));
}

float apparent_magnitude_for_solar_luminosity_distance(const float solar_luminosity, const float distance_ly) {
    return absolute_magnitude_for_solar_luminosity(solar_luminosity) - (5.f * (1.f - log10f(distance_ly / LY_PER_PARSEC)));
}

float eddington_luminosity(const float solar_masses) {
    return EDDINGTON_CONSTANT * solar_masses;
}

float solar_luminosity_for_solar_masses(const float solar_masses) {
    float p = logf(solar_masses);
    float q;

    if (solar_masses <= 0.2) {
        q =                      (-0.15f * p * p)         + (4.6f * p)             + 3.6f;
    } else if (solar_masses <= 0.4) {
        q =                      (0.21853f * p * p)       + (2.8385f * p)          - 0.781f;
    } else if (solar_masses <= 1.42) {
        q =                      (-0.11746f * p * p)      + (3.9784f * p);
    } else {
        q = (0.4f * p * p * p) + (-2.24f * p * p)         + (6.2316f * p)          - 0.2894f;
    }

    float l = powf(10.f, q);
    float m = eddington_luminosity(solar_masses);
    return l > m ? m : l;
}

float solar_radii_for_solar_luminosity_and_temperature(const float solar_luminosity, const float kelvin) {
    const float t_sun = 5800.0f; // approx
    const float t = kelvin;
    const float ts_t = t_sun / t;

    const float mag_sun = absolute_magnitude_for_solar_luminosity(1.0f);
    const float mag = absolute_magnitude_for_solar_luminosity(solar_luminosity);
    const float d_mag = mag_sun - mag;
    
    float r = powf(ts_t, 2.0) * sqrt(powf(2.512, d_mag));
    return r;
}
