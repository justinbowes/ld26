//
//  packet.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-26.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_packet_h
#define ld26_packet_h

#include "game/game.h"

#define CHAT_MAX 64

#define encode(x, type, ptr) \
	*((type *)ptr) = x; \
	ptr += sizeof(type);


#define decode(ptr, type, x) \
	x = *((type *)ptr); \
	ptr += sizeof(type);

typedef enum packet_type {
	pt_hello,
	pt_goodbye,
	pt_player,
	pt_projectile,
	pt_damage,
	pt_chat
} packet_type_t;

typedef struct packet {
	uint32_t seq;
	uint8_t type;
	
	union {
		player_t		player;
		player_id_t		hello;
		player_id_t		goodbye;
		projectile_t	projectile;
		damage_t		damage;
		char			chat[CHAT_MAX];
	};
	
} packet_t;

size_t packet_encode(packet_t *packet, uint16_t client_id, uint8_t *buffer);
bool packet_decode(packet_t *packet, uint16_t *client_source, uint8_t *buffer);

#endif
