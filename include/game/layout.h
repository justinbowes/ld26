//
//  layout.h
//  app
//
//  Created by Justin Bowes on 2013-06-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef app_layout_h
#define app_layout_h

#include <xpl_platform.h>
#include "game/sprites.h"

XPLINLINE int weapon_buttons_left(xivec2 screen) {
	if (screen.width <= 480) return 140;
	return 160;
}

XPLINLINE int weapon_buttons_spacing(xivec2 screen) {
	if (screen.width <= 480) return 0;
	return 8;
}

XPLINLINE int weapon_button_left(xivec2 screen, int index) {
	return weapon_buttons_left(screen) + ((TILE_SIZE + weapon_buttons_spacing(screen)) * index);
}

XPLINLINE float weapon_price_size(xivec2 screen) {
	if (screen.width <= 480) return 12.f;
	return 16.f;
}

XPLINLINE int health_left(xivec2 screen) {
	return weapon_button_left(screen, 8);
}

XPLINLINE int weapon_cooldown_left(xivec2 screen) {
	return weapon_buttons_left(screen) - (8 + (weapon_buttons_spacing(screen)));
}

XPLINLINE int fire_button_left(xivec2 screen) {
	return screen.width - 72;
}

#endif
