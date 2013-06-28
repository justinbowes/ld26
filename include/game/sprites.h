//
//  sprites.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_sprites_h
#define ld26_sprites_h

#include "xpl_sprite.h"
#include "xpl_context.h"

#include "game/game.h"

typedef struct sprites {
	xpl_sprite_batch_t					*playfield_batch;
	xpl_sprite_batch_t					*ui_batch;
	xpl_sprite_t						*panel_background_sprite;
	xpl_sprite_t						*grid8_sprite;
	xpl_sprite_t						*ship_sprite;
	xpl_sprite_t						*solid_sprite;
	xpl_sprite_t						*star_sprite;
	xpl_sprite_t						*indicator_sprite;
	xpl_sprite_t						*playfield_coin_sprite;
	xpl_sprite_t						*particle_sprite;
	xpl_sprite_t						*weapon_key_sprites[8];
	xpl_sprite_t						*control_key_sprites[3];
	xpl_sprite_t						*ui_coin_sprite;
	xpl_sprite_t						*fire_button_lit;
	xpl_sprite_t						*fire_button_dark;
	
	xpl_sprite_t						*ui_tutorial_pages[TUTORIAL_PAGES];
	
} sprites_t;

extern sprites_t						sprites;


#define STAR_LAYERS		3
#define STARS_PER_LAYER	256
#define STAR_LAYER_SIZE	4096

#define TILE_SIZE		32

extern xivec3							star_layers[STAR_LAYERS][STARS_PER_LAYER];

void sprites_init(void);
void sprites_playfield_render(xpl_context_t *self, xmat4 *ortho);
void sprites_ui_render(xpl_context_t *self, xmat4 *ortho);

#endif
