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
#include "game/game.h"

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
	float	explode_radius;
	int		explode_particle_count;
	int		explode_particle_size;
	uint32_t explode_particle_color;
	float	explode_particle_color_variance;
	float	explode_particle_life;
	float	explode_particle_velocity;
	int		fire_cooldown;
	int		price;
	bool	can_hit_self;
	bool	is_mine;
	
	const char *identifier;
	const char *fire_effect;
	const char *explode_effect;
	
} projectile_config_t;

/*
typedef enum projectile_type {
	pt_pew			= 0,
	pt_heavypew		= 1,
	pt_missile		= 2,
	pt_heavymissile = 3,
	pt_nuke			= 4,
	pt_mine			= 5,
	pt_blackhole	= 6,
	pt_repair		= 7
} projectile_type_t;
 */

#define EXPL_VIS_FACTOR (128.0 / VELOCITY_SCALE)

static const projectile_config_t projectile_config[] = {
	{ // 0: pew
		20,
		2000,
		3,
		RGBA(0xff, 0x00, 0x00, 0xff),
		0.1f,
		
		2,
		RGBA(0xff, 0x0, 0x0, 0x80),
		0.1f,
		0.4f,
		5,
		
		20.f,
		100,
		2,
		RGBA(0xff, 0x00, 0x00, 0xff),
		0.1f,
		0.5f,
		100.f,
		
		20,
		-1,
		false,
		false,
		
		"pew",
		"weapon_0",
		"explode_4"
	},
	{ // 1: heavy pew
		30,
		1500,
		4,
		RGBA(0xff, 0x00, 0xff, 0xff),
		0.1f,
		
		2,
		RGBA(0xff, 0x0, 0x0, 0x80),
		0.1f,
		0.4f,
		5,
		
		40.f,
		150,
		3,
		RGBA(0xff, 0x00, 0x00, 0xff),
		0.1f,
		0.5f,
		100.f,
		
		16,
		1,
		false,
		false,
		
		"heavy_pew",
		"weapon_1",
		"explode_4"
	},
	{ // 2: missile
		45,
		900,
		4,
		RGBA(0xc0, 0xc0, 0xc0, 0xff),
		0.0f,
		
		2,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.0f,
		5,
		
		16.f,
		100,
		4,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		0.4f,
		16.f / 0.4f * EXPL_VIS_FACTOR,
		
		30,
		5,
		false,
		false,
		
		"missile",
		"weapon_2",
		"explode_2"
	},
	{ // 3: heavy missile
		90,
		650,
		4,
		RGBA(0xc0, 0xc0, 0xc0, 0xff),
		0.0f,
		
		2,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.0f,
		5,
		
		50.f,
		100,
		4,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.2f,
		50.f / 1.2f * EXPL_VIS_FACTOR,
		
		60,
		10,
		false,
		false,
		
		"heavy_missile",
		"weapon_3",
		"explode_2"
	},
	{ // 4: nuke
		255,
		350,
		8,
		RGBA(0xc0, 0xc0, 0xc0, 0xff),
		0.0f,
		
		2,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.0f,
		10,
		
		400.f,
		500,
		4,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		5.0f,
		400.f / 5.f * EXPL_VIS_FACTOR,
		
		180,
		250,
		true,
		false,
		
		"nuke",
		"weapon_4",
		"explode_0"
	},
	/*
	{ // 5: mine
		90,
		0,
		6,
		RGBA(0xc0, 0xc0, 0xc0, 0xff),
		0.0f,
		
		0,
		RGBA(0xff, 0x30, 0x30, 0xff),
		0.4f,
		0.f,
		255,
		
		50.f,
		200,
		4,
		RGBA(0xff, 0xff, 0x30, 0xc0),
		0.1f,
		1.2f,
		50.f / 1.2f * EXPL_VIS_FACTOR,
		
		30,
		10,
		true,
		true,
		
		"mine",
		"weapon_5",
		"explode_2"
	},
	{ // 6: black hole
		120,
		-50,
		16,
		RGBA(0xff, 0x70, 0xff, 0xff),
		0.0f,
		
		0,
		RGBA(0x40, 0x20, 0x40, 0xff),
		0.f,
		0.f,
		255,
		
		512.f,
		400,
		4,
		RGBA(0xff, 0x70, 0xff, 0xff),
		0.1f,
		5.0f,
		512.f / 5.0f * EXPL_VIS_FACTOR,
		
		180,
		500,
		true,
		true,
		
		"mine",
		"weapon_6",
		"explode_3"
	},
	 */
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
		
		0.f,
		100,
		2,
		RGBA(0x00, 0xff, 0x00, 0xff),
		0.1f,
		1.0f,
		120.f,
		
		360,
		250,
		true,
		false,
		
		"health_kit",
		"weapon_7",
		NULL
	},
};

static const size_t projectile_type_count = sizeof(projectile_config) / sizeof(projectile_config[0]);

#endif
