//
//  packet.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-26.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xpl_platform.h>
#ifndef XPL_PLATFORM_WINDOWS
#	include <arpa/inet.h>
#	include <netinet/in.h>
#else
#	include <Winsock2.h>
#endif

#include "game/packet.h"
#include "game/game.h"


#define encode(x, type, ptr) \
	*((type *)ptr) = (sizeof(type) == 1 ? x : \
		sizeof(type) == 2 ? htons(x) : htonl(x)); \
	ptr += sizeof(type);


#define decode(ptr, type, x) \
	x = (sizeof(type) == 1 ? *((type *)ptr) : \
		sizeof(type) == 2 ? ntohs(*((type *)ptr)) : ntohl(*((type *)ptr))); \
	ptr += sizeof(type);

static const uint16_t ultrapew_magic = (uint16_t)0xff37;
static const uint8_t protocol_version = 0x03;

size_t packet_encode(packet_t *packet, uint16_t client_id, uint8_t *buffer) {
	uint8_t *p = buffer;
	
	encode(ultrapew_magic, uint16_t, p);
	encode(protocol_version, uint8_t, p);
	
	encode(client_id, uint16_t, p);
	encode(packet->seq, uint32_t, p);
	encode(packet->type, uint8_t, p);
	
	switch (packet->type) {
		case pt_hello:
		case pt_goodbye:
			encode(packet->hello.client_id, uint16_t, p);
			encode(packet->hello.nonce, uint16_t, p);
			memmove(p, packet->hello.name, NAME_SIZE);
			p += NAME_SIZE;
			break;
			
		case pt_player:
			encode(packet->player.position.px, uint16_t, p);
			encode(packet->player.position.py, uint16_t, p);
			encode(packet->player.velocity.dx, int16_t, p);
			encode(packet->player.velocity.dy, int16_t, p);
			encode(packet->player.score, uint32_t, p);
			encode(packet->player.orientation, uint8_t, p);
			encode(packet->player.health, uint8_t, p);
			encode(packet->player.is_thrust, bool, p);
			break;
			
		case pt_projectile:
			encode(packet->projectile.pid, uint16_t, p);
			encode(packet->projectile.position.px, uint16_t, p);
			encode(packet->projectile.position.py, uint16_t, p);
			encode(packet->projectile.velocity.dx, int16_t, p);
			encode(packet->projectile.velocity.dy, int16_t, p);
			encode(packet->projectile.orientation, uint8_t, p);
			encode(packet->projectile.health, uint8_t, p);
			encode(packet->projectile.type, uint8_t, p);
			break;
			
		case pt_damage:
			encode(packet->damage.player_id, uint16_t, p);
			encode(packet->damage.projectile_id, uint16_t, p);
			encode(packet->damage.amount, uint8_t, p);
			encode(packet->damage.flags, uint8_t, p);
			break;
			
		case pt_chat:
			memmove(p, packet->chat, CHAT_MAX);
			p += CHAT_MAX;
			break;
			
		default:
			break;
	}
	
	return p - buffer; // bytes
}

bool packet_decode(packet_t *packet, uint16_t *client_source, uint8_t *buffer) {
	uint8_t *p = buffer;
	
	uint16_t magic;
	decode(p, uint16_t, magic);
	if (magic != ultrapew_magic) return false;
	
	uint8_t protocol;
	decode(p, uint8_t, protocol);
	if (protocol != protocol_version) return false;
	
	decode(p, uint16_t, *client_source);
	decode(p, uint32_t, packet->seq);
	decode(p, uint8_t, packet->type);
	
	switch (packet->type) {
		case pt_hello:
		case pt_goodbye:
			decode(p, uint16_t, packet->hello.client_id);
			decode(p, uint16_t, packet->hello.nonce);
			memmove(packet->hello.name, p, NAME_SIZE);
			p += NAME_SIZE;
			break;
			
		case pt_player:
			decode(p, uint16_t, packet->player.position.px);
			decode(p, uint16_t, packet->player.position.py);
			decode(p, int16_t, packet->player.velocity.dx);
			decode(p, int16_t, packet->player.velocity.dy);
			decode(p, uint32_t, packet->player.score);
			decode(p, uint8_t, packet->player.orientation);
			decode(p, uint8_t, packet->player.health);
			decode(p, bool, packet->player.is_thrust);
			break;
			
		case pt_projectile:
			decode(p, uint16_t, packet->projectile.pid);
			decode(p, uint16_t, packet->projectile.position.px);
			decode(p, uint16_t, packet->projectile.position.py);
			decode(p, int16_t, packet->projectile.velocity.dx);
			decode(p, int16_t, packet->projectile.velocity.dy);
			decode(p, uint8_t, packet->projectile.orientation);
			decode(p, uint8_t, packet->projectile.health);
			decode(p, uint8_t, packet->projectile.type);
			break;
			
		case pt_damage:
			decode(p, uint16_t, packet->damage.player_id);
			decode(p, uint16_t, packet->damage.projectile_id);
			decode(p, uint8_t, packet->damage.amount);
			decode(p, uint8_t, packet->damage.flags);
			break;
			
		case pt_chat:
			memmove(packet->chat, p, CHAT_MAX);
			break;
			
		default:
			return false;
	}
	
	return true;
}
