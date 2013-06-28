//
//  game.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-26.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_game_h
#define ld26_game_h

#include <stdint.h>
#include <stdbool.h>

#include "xpl_vec2.h"
#include "xpl_vec4.h"
#include "xpl_irect.h"

#include "audio/audio.h"

#define PLAYFIELD_MAX	4096

#define MAX_PLAYERS		128
#define MAX_PROJECTILES	4096
#define MAX_PARTICLES	4096
#define MAX_TEXT_PARTICLES 128
#define MAX_TEXT_PARTICLE_CHARS 16
#define NAME_SIZE		16

#define VELOCITY_SCALE	128.0f

#define INDICATOR_SIZE	16.f
#define PLAYER_SIZE		24.f

#define TUTORIAL_PAGES	6

typedef struct position {
	uint16_t px;
	uint16_t py;
} position_t;

typedef struct velocity {
	int16_t dx;
	int16_t dy;
} velocity_t;

typedef struct player {
	position_t	position;
	velocity_t	velocity;
	uint32_t	score;
	uint8_t		orientation;
	uint8_t		health;
	bool		is_thrust;
} player_t;

typedef struct player_local {
	xvec2		position_buffer;
	bool		visible;
	audio_t		*thrust_audio;
	audio_t		*rotate_audio;
} player_local_t;

typedef struct player_id {
	uint16_t	client_id;
	uint16_t	nonce;
	char		name[NAME_SIZE];
} player_id_t;

typedef struct projectile {
	uint16_t	pid;
	position_t	position;
	velocity_t	velocity;
	uint8_t		orientation;
	uint8_t		health;
	uint8_t		type;	
} projectile_t;

typedef struct projectile_local {
	uint8_t		trail_timeout;
	uint16_t	owner;
	bool		force_detonate;
	bool		exploded;
	xvec4		color;
} projectile_local_t;

typedef struct particle {
	position_t	position;
	xvec2		fposition;
	xvec2		velocity;
	float		size;
	float		orientation;
	float		rotation;
	float		initial_life;
	float		life;
	bool		decay;
	xvec4		initial_color;
	xvec4		color;
} particle_t;

typedef struct text_particle {
	char		text[MAX_TEXT_PARTICLE_CHARS];
	position_t	position;
	xvec2		fposition;
	xvec2		velocity;
	xvec4		initial_color;
	xvec4		color;
	float		orientation;
	float		life;
	float		initial_life;
} text_particle_t;

#define DAMAGE_FLAG_EXPLODES	0x01
#define DAMAGE_FLAG_REPAIRS		0x02

typedef struct damage {
	uint16_t	player_id;
	uint16_t	projectile_id;
	uint8_t		amount;
	uint8_t		flags;
} damage_t;

typedef struct game {
	
	xvec2		player_position_buffer[MAX_PLAYERS];
	player_t	player[MAX_PLAYERS];
	player_id_t	player_id[MAX_PLAYERS];
	bool		player_connected[MAX_PLAYERS];
	player_local_t player_local[MAX_PLAYERS];
	
	xvec2		projectile_position_buffer[MAX_PROJECTILES];
	projectile_t projectile[MAX_PROJECTILES];
	projectile_local_t projectile_local[MAX_PROJECTILES];
	
	int			fire_cooldown;
	float		respawn_cooldown;
	
	uint8_t		indicators_cooldown;
	bool		indicators_on;
	
	bool		control_indicator_on[3];
	int			active_weapon;
	
	particle_t	particle[MAX_PARTICLES];
	text_particle_t text_particle[MAX_TEXT_PARTICLES];
	
} game_t;

extern game_t game;

#define SERVER_SIZE		128

typedef struct error {
	char		msg[256];
	float		timeout;
} error_t;

extern error_t error;

typedef struct network {
	
	char		server_host[SERVER_SIZE];
	uint16_t	server_port;
	float		hello_timeout;
	
	float		position_timeout;
	float		receive_timeout;
	
	double		latency_time;
	double		latency_timestamp;
	double		latency;
	
} network_t;

extern network_t network;

#endif
