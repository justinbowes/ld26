//
//  celestial_constants.h
//  elsie
//
//  Created by Justin Bowes on 2012-09-08.
//  Copyright (c) 2012 Informi Software Inc. All rights reserved.
//

#ifndef elsie_celestial_constants_h
#define elsie_celestial_constants_h


#ifdef __cplusplus
#define __externcstart  extern "C" {
#define __externcend    }
__externcstart
#endif


#define     EDDINGTON_CONSTANT          32000.f
#define 	G_CONST						6.67428E-11f
// units are J•s^-1•m^-2•K-4
#define 	STEFANS_CONSTANT			5.670400E-8f
/**
 * Nothing in the universe is naturally colder than this.
 */
#define 	MICROWAVE_BACKGROUND_TEMP	2.7255f


#define 	AU                          149.6E9f
#define 	M_PER_LY					9.4605284E15f
#define 	LY_PER_PARSEC				3.26163626f

// Galaxy constants
#define 	MILKY_WAY_DISC_RADIUS_LY	60000
#define 	MILKY_WAY_STAR_COUNT		200000000000L


#define     SOLAR_ABSOLUTE_MAGNITUDE    4.83f
#define 	SOLAR_RADIUS                6.955E8f
#define 	SOLAR_MASS					1.98892E30f
/**
 * Effective solar photosphere temperature, http://en.wikipedia.org/wiki/Sun
 */
#define 		SOLAR_TEMPERATURE			5778.f


#define 	EARTH_RADIUS                6371000.f
#define 	EARTH_MASS					5.9742E24f
/**
 * The earth has medium-low albedo. Very grounded.
 */
#define 	EARTH_ALBEDO				0.28f

/**
 * It's dark out there man
 */
#define 	VISIBILITY_THRESHOLD		10.5f


/**
 * X width of galactic disc in LY.
 */
#define 		GALAXY_DISC_SIZE_X			100000.f
/**
 * Height of galactic disc in LY.
 */
#define 		GALAXY_DISC_SIZE_Y			1000.f
/**
 * Z width of galactic disc in LY.
 */
#define 		GALAXY_DISC_SIZE_Z			100000.f

/**
 * X width of galactic core bulge
 */
#define 		GALAXY_CORE_SIZE_X			15000.f

/**
 * Height of galactic core bulge
 */
#define 		GALAXY_CORE_SIZE_Y			8000.f

/**
 * Z width of galactic core bulge
 */
#define 		GALAXY_CORE_SIZE_Z			15000.f

/**
 * The actual core mass is 25%, but the disc gaussian overlaps.
 */
#define 			GALAXY_CORE_PROBABILITY		0.15f

/**
 * The core has a bias towards cooler stars.
 */
#define 		GALAXY_LOW_TEMP				4000.f
    
#ifdef __cplusplus
__externcend
#endif

#endif
