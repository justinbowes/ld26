//
//  util.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_util_h
#define ld26_util_h

#include "game/game.h"

XPLINLINE bool position_in_bounds(position_t position, int fudge, position_t min, position_t max) {
	long x = position.px;
	long y = position.py;
	
	if (x + fudge < min.px) return false;
	if (y + fudge < min.py) return false;
	
	if (x - fudge > max.px) return false;
	if (y - fudge > max.py) return false;
	
	return true;
}

XPLINLINE bool player_in_bounds(int i, int fudge, position_t min, position_t max) {
	position_t player_position = game.player[i].position;
	return position_in_bounds(player_position, fudge, min, max);
}

XPLINLINE float player_rotation_rads_get(int i) {
	float orientation = game.player[i].orientation;
	orientation /= (float)UINT8_MAX;
	orientation *= M_2PI;
	return orientation;
}

const char *random_word(const char *key_prefix);

#endif
