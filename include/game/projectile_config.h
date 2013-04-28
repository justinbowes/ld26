//
//  projectile_config.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-28.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_projectile_config_h
#define ld26_projectile_config_h

#include "xpl_color.h"

typedef struct projectile_config {
	uint8_t	initial_health;
	int16_t velocity;
	int		size;
	uint32_t color;
	float	variance;
	int		trail_size;
	uint32_t trail_color;
	float	trail_variance;
	float	trail_life;
	uint8_t	trail_timeout;
	int		explode_particle_count;
	int		explode_particle_size;
	uint32_t explode_particle_color;
	float	explode_particle_color_variance;
	float	explode_particle_life;
	float	explode_particle_velocity;
	int		fire_cooldown;
	int		price;
} projectile_config_t;

static const projectile_config_t projectile_config[] = {
	{ // 0: pew
		20,
		256,
		3,
		RGBA(0xff, 0x00, 0x00, 0xff),
		0.1f,
		
		2,
		RGBA(0xff, 0x0, 0x0, 0x80),
		0.1f,
		0.4f,
		5,
		
		100,
		2,
		RGBA(0xff, 0x00, 0x00, 0xff),
		0.1f,
		0.5f,
		100.f,
		20,
		
		-1
	},
	{ // 1: heavy pew
		30,
		256,
		4,
		RGBA(0xff, 0x00, 0x00, 0xff),
		0.1f,
		
		2,
		RGBA(0xff, 0x0, 0x0, 0x80),
		0.1f,
		0.4f,
		5,
		
		150,
		3,
		RGBA(0xff, 0x00, 0x00, 0xff),
		0.1f,
		0.5f,
		100.f,
		25,
		
		2
	},
	{ // 2: missile
		80,
		80,
		4,
		RGBA(0xc0, 0xc0, 0xc0, 0xff),
		0.0f,
		
		2,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.0f,
		5,
		
		100,
		4,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		0.8f,
		150.f,
		50,
		
		5
	},
	{ // 3: heavy missile
		120,
		60,
		4,
		RGBA(0xc0, 0xc0, 0xc0, 0xff),
		0.0f,
		
		2,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.0f,
		5,
		
		100,
		4,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.2f,
		150.f,
		150,
		
		10
	},
	{ // 4: nuke
		255,
		30,
		8,
		RGBA(0xc0, 0xc0, 0xc0, 0xff),
		0.0f,
		
		2,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.0f,
		10,
		
		500,
		4,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		5.0f,
		150.f,
		180,
		
		250
	},
	{ // 5: mine
		200,
		0,
		6,
		RGBA(0xc0, 0xc0, 0xc0, 0xff),
		0.0f,
		
		0,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.f,
		0.f,
		255,
		
		400,
		4,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		5.0f,
		150.f,
		180,
		
		10
	},
	{ // 6: black hole
		120,
		32,
		16,
		RGBA(0x00, 0x00, 0x00, 0xff),
		0.0f,
		
		0,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.f,
		0.f,
		255,
		
		400,
		4,
		RGBA(0xff, 0x70, 0xff, 0xff),
		0.1f,
		5.0f,
		150.f,
		180,
		
		500
	},
	{ // 7: repair
		128,
		0,
		0,
		RGBA(0x00, 0x00, 0x00, 0xff),
		0.0f,
		
		0,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.f,
		0.f,
		255,
		
		100,
		2,
		RGBA(0x00, 0xff, 0x00, 0xff),
		0.1f,
		1.0f,
		120.f,
		360,
		
		250
	},
};

#endif
