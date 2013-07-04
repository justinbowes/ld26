//
//  sprites.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "xpl_rand.h"
#include "xpl_color.h"
#include "xpl_sprite_sheet.h"

#include "game/game.h"

#include "game/camera.h"
#include "game/palette.h"
#include "game/sprites.h"
#include "game/util.h"
#include "game/projectile_config.h"
#include "game/hotspots.h"
#include "game/layout.h"

#include "science/star_generator.h"
#include "random/det_rng.h"

sprites_t						sprites;
xivec3							star_layers[STAR_LAYERS][STARS_PER_LAYER];
xivec3							debris_layer[DEBRIS_PER_LAYER];

void sprites_init(void) {
	
	rng_seq_t rng;
	double time = xpl_get_time();
	union {
		double dv;
		int iv[2];
	} init_v;
	init_v.dv = time;
	rng_seq_init_ints(init_v.iv, 2, &rng);
	
	sprites.playfield_batch = xpl_sprite_batch_new();
	sprites.ui_batch = xpl_sprite_batch_new();

	xpl_sprite_sheet_t *playfield_sheet = xpl_sprite_sheet_new(sprites.playfield_batch, "bitmaps/playfield.json");
	sprites.ship_sprite = xpl_sprite_get(playfield_sheet, "ship.png");
	sprites.indicator_sprite = xpl_sprite_get(playfield_sheet, "indicator.png");
	sprites.particle_sprite = xpl_sprite_get(playfield_sheet, "particle.png");
	sprites.playfield_coin_sprite = xpl_sprite_get(playfield_sheet, "coin.png");
	sprites.star_sprite = xpl_sprite_get(playfield_sheet, "star.png");
	
	xpl_sprite_sheet_t *ui_sheet = xpl_sprite_sheet_new(sprites.ui_batch, "bitmaps/ui.json");
	sprites.panel_background_sprite = xpl_sprite_get(ui_sheet, "panel_background.png");
	sprites.solid_sprite = xpl_sprite_get(ui_sheet, "tile_solid.png");
	sprites.grid8_sprite = xpl_sprite_get(ui_sheet, "tile_grid.png");
	
	for (int i = 0; i < projectile_type_count; ++i) {
		char resource[PATH_MAX];
		snprintf(resource, PATH_MAX, "%s.png", projectile_config[i].fire_effect);
		sprites.weapon_key_sprites[i] = xpl_sprite_get(ui_sheet, resource);
	}
	const char *key_sprites[3] = { "key_thrust.png", "key_left.png", "key_right.png" };
	for (int i = 0; i < 3; ++i) {
		sprites.control_key_sprites[i] = xpl_sprite_get(ui_sheet, key_sprites[i]);
	}
	
	sprites.joystick_pen_sprite = xpl_sprite_get(ui_sheet, "thrust_stick_pen.png");
	sprites.joystick_sprite = xpl_sprite_get(ui_sheet, "thrust_stick.png");
	
	sprites.ui_coin_sprite = xpl_sprite_get(ui_sheet, "coin.png");
	sprites.fire_button_lit = xpl_sprite_get(ui_sheet, "fire_button_lit.png");
	sprites.fire_button_dark = xpl_sprite_get(ui_sheet, "fire_button_dark.png");
	
	for (int i = 0; i < STAR_LAYERS; ++i) {
		for (int j = 0; j < STARS_PER_LAYER; ++j) {
			star_t star;
			star_randomize(&star, &rng);
			star_layers[i][j].x = xpl_irand_range(0, STAR_LAYER_SIZE);
			star_layers[i][j].y = xpl_irand_range(0, STAR_LAYER_SIZE);
			
			star_layers[i][j].z = (int)RGBA((int)(star.color.r * 0xff),
											(int)(star.color.g * 0xff),
											(int)(star.color.b * 0xff),
											(int)(star.sradii * 0x40 + 0x40));
		}
	}
	for (int j = 0; j < DEBRIS_PER_LAYER; ++j) {
		debris_layer[j].x = xpl_irand_range(0, STAR_LAYER_SIZE);
		debris_layer[j].y = xpl_irand_range(0, STAR_LAYER_SIZE);
		debris_layer[j].z = (int)RGBA(0xff, 0xff, 0xff, 0x80);
	}
	
	for (int i = 0; i < TUTORIAL_PAGES; ++i) {
		char resource[PATH_MAX];
		snprintf(resource, PATH_MAX, "tutorial_%d.png", i);
		sprites.ui_tutorial_pages[i] = xpl_sprite_new(sprites.ui_batch, resource, NULL);
		// xpl_sprite_set_blend_funcs(sprites.ui_tutorial_pages[i], BLEND_FUNCS_REPLACE);
	}

}

void sprites_playfield_render(xpl_context_t *self, xmat4 *ortho) {
	xpl_sprite_batch_begin(sprites.playfield_batch);
	{
		xmat4 *sprite_ortho = xpl_sprite_batch_matrix_push(sprites.playfield_batch);
		*sprite_ortho = *ortho;
		
		// Background stars
		// Need to draw back to front
		for (int i = STAR_LAYERS - 1; i >= 0; --i) {
			for (int j = 0; j < STARS_PER_LAYER; ++j) {
				int64_t px = (star_layers[i][j].x - (camera.center.px >> (2 * i + 2))) % STAR_LAYER_SIZE;
				int64_t py = (star_layers[i][j].y - (camera.center.py >> (2 * i + 2))) % STAR_LAYER_SIZE;
				if (px > camera.draw_area.x &&
					py > camera.draw_area.y &&
					px < camera.draw_area.x + camera.draw_area.width &&
					py < camera.draw_area.y + camera.draw_area.height) {
					xvec4 color = RGBA_F((uint32_t)star_layers[i][j].z);
					float k = 50.f * color.a;
					color.a = 0.5f;
					float size = k / (i + 1);
					float hsize = size / 2;
					xpl_sprite_draw_transformed(sprites.star_sprite,
												px - hsize, py - hsize, hsize, hsize,
												size, size,
												1.f, 1.f,
												0.f,
												&color);
					color.a = 1.0f;
					float rsize = k / (i + 2);
					float hrsize = rsize / 2;
					xpl_sprite_draw_transformed(sprites.star_sprite,
												px - hrsize, py - hrsize, hsize, hsize,
												rsize, rsize,
												1.f, 1.f,
												0.f,
												&color);
				}
			}
		}
		
		// Particles
		for (int i = 0; i < MAX_PARTICLES; ++i) {
			if (game.particle[i].life >= 0.f) {
				if (position_in_bounds(game.particle[i].position, game.particle[i].size, camera.min, camera.max)) {
					xvec2 v = camera_get_draw_position(game.particle[i].position);
					xvec2 s = xvec2_set(game.particle[i].size, game.particle[i].size);
					xvec2 half_s = xvec2_scale(s, 0.5f);
					v = xvec2_sub(v, half_s);
					xpl_sprite_draw_transformed(sprites.particle_sprite,
												v.x, v.y,
												half_s.x, half_s.y,
												s.x, s.y,
												1.f, 1.f,
												game.particle[i].orientation,
												&game.particle[i].color);
				}
			}
		}
		
		for (int i = 0; i < MAX_PROJECTILES; ++i) {
			if (game.projectile[i].health) {
				int pt = game.projectile[i].type;
				if (position_in_bounds(game.projectile[i].position, projectile_config[pt].size, camera.min, camera.max)) {
					xvec2 v = camera_get_draw_position(game.projectile[i].position);
					xvec2 s = xvec2_set(projectile_config[pt].size, projectile_config[pt].size);
					xvec2 half_s = xvec2_scale(s, 0.5f);
					v = xvec2_sub(v, half_s);
					xpl_sprite_draw_transformed(sprites.particle_sprite,
												v.x, v.y,
												half_s.x, half_s.y,
												s.x, s.y,
												1.f, 1.f,
												game.projectile[i].orientation,
												&game.projectile_local[i].color);
				}
			}
		}
		
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (! game.player_connected[i]) continue;
			if (! game.player_local[i].visible) continue;
			
			if (player_in_bounds(i, PLAYER_SIZE, camera.min, camera.max)) {
				// Draw player
				xvec2 v = camera_get_draw_position(game.player[i].position);
				xvec2 s = xvec2_set(PLAYER_SIZE, PLAYER_SIZE);
				xvec2 half_s = xvec2_scale(s, 0.5f);
				v = xvec2_sub(v, half_s);
				float rot_rad = player_rotation_rads_get(i);
				xvec4 color = (i == 0 ? xvec4_set(0.f, 1.f, 0.f, 1.f) : xvec4_set(0.8f, 0.8f, 0.8f, 1.f));
				xpl_sprite_draw_transformed(sprites.ship_sprite,
											v.x, v.y,
											half_s.x, half_s.y,
											s.x, s.y,
											1.f, 1.f,
											rot_rad,
											&color);
				
				if (i > 0) {
					int coin_symbols = xclamp(1 + game.player[i].score / 250, 1, 4);
					v.y -= (PLAYER_SIZE + 16);
					v.x += -4 * (coin_symbols - 2);
					for (int j = 0; j < coin_symbols; ++j) {
						xpl_sprite_draw_colored(sprites.playfield_coin_sprite, v.x, v.y, 8, 8, coin_color);
						v.x += 8;
					}
				}
			}
		}
		
		// Debris
		for (int j = 0; j < DEBRIS_PER_LAYER; ++j) {
			const size_t i = 0; // as though in front of stars
			int64_t px = ((debris_layer[j].x - camera.center.px) << (i + 1)) % STAR_LAYER_SIZE;
			int64_t py = ((debris_layer[j].y - camera.center.py) << (i + 1)) % STAR_LAYER_SIZE;
			if (px > camera.draw_area.x &&
				py > camera.draw_area.y &&
				px < camera.draw_area.x + camera.draw_area.width &&
				py < camera.draw_area.y + camera.draw_area.height) {
				xvec4 color = RGBA_F((uint32_t)debris_layer[j].z);
				xpl_sprite_draw_transformed(sprites.star_sprite,
											px, py, 0.f, 0.f,
											8 * (i + 1), 8 * (i + 1),
											1.f, 1.f,
											0.f,
											&color);
			}
		}
		
		if (game.indicators_on) {
			for (int i = 0; i < MAX_PLAYERS; ++i) {
				if (! game.player_connected[i]) continue;
				if (! player_in_bounds(i, PLAYER_SIZE, camera.min, camera.max)) {
					// Draw indicator
					int64_t dlx = (int64_t)game.player[i].position.px - (int64_t)game.player[0].position.px;
					int64_t dly = (int64_t)game.player[i].position.py - (int64_t)game.player[0].position.py;
					xvec2 d = {{ (float)dlx, (float)dly }};
					float angle = atan2f(d.y, d.x);
					d = xvec2_add(d, xvec2_set(camera.draw_area.x + (camera.draw_area.width >> 1),
											   camera.draw_area.y + (camera.draw_area.height >> 1)));
					d.x = xclamp(d.x, camera.draw_area.x, camera.draw_area.x + camera.draw_area.width - INDICATOR_SIZE);
					d.y = xclamp(d.y, camera.draw_area.y, camera.draw_area.y + camera.draw_area.height - INDICATOR_SIZE);
					xvec4 color = xvec4_set(1.f, 1.f, 0.f, 0.6f);
					xpl_sprite_draw_transformed(sprites.indicator_sprite,
												d.x, d.y,
												INDICATOR_SIZE * 0.5f, INDICATOR_SIZE * 0.5f,
												INDICATOR_SIZE, INDICATOR_SIZE,
												1.0f, 1.0f,
												angle,
												&color);
				}
			}
		}
		
		xpl_sprite_batch_matrix_pop(sprites.playfield_batch);
	}
	xpl_sprite_batch_end(sprites.playfield_batch);
}

void sprites_ui_render(xpl_context_t *self, xmat4 *ortho) {
	xpl_sprite_batch_begin(sprites.ui_batch);
	{
		xmat4 *sprite_ortho = xpl_sprite_batch_matrix_push(sprites.ui_batch);
		*sprite_ortho = *ortho;
		
		// Bottom and top panels
		xpl_sprite_draw(sprites.panel_background_sprite, 0.f, 0.f, self->size.width, camera.dc.y);
		xpl_sprite_draw(sprites.panel_background_sprite, 0.f, camera.dc.y + camera.dc.height, self->size.width, camera.dc.y);
		
		// Control hotspots
#ifdef XPL_PLATFORM_IOS
		{
			joystick.bounds = xirect_set(0, 0, joystick_pen_width(self->size), joystick_pen_height(self->size));
			joystick.neutral.x = joystick.bounds.width / 2 + joystick.bounds.x;
			joystick.neutral.y = joystick.bounds.y;
			joystick.stick.width = joystick.stick.height = joystick_stick_size(self->size);
			xpl_sprite_draw_colored(sprites.joystick_pen_sprite, joystick.bounds.x, joystick.bounds.y, joystick.bounds.width, joystick.bounds.height, inactive_color);
			xpl_sprite_draw(sprites.joystick_sprite,
							joystick.stick.x - (joystick.stick.width >> 1),
							joystick.stick.y - (joystick.stick.height >> 1),
							joystick.stick.width, joystick.stick.height);
			hotspot_set("joystick", 0, joystick.bounds, self->size);
		}
#else
		for (int i = 0; i < 3; ++i) {
			xirect area = {{  8 + (TILE_SIZE + 8) * i, 24, TILE_SIZE, TILE_SIZE }};
			xpl_sprite_draw_colored(sprites.control_key_sprites[i], area.x, area.y, area.width, area.height,
									game.control_indicator_on[i] ? active_color : inactive_color);
			hotspot_set("thrust", i, area, self->size);
		}
#endif
		
		xpl_sprite_draw_colored(sprites.ui_coin_sprite, weapon_buttons_left(self->size), 4, 16, 16, coin_color);
		for (int i = 0; i < projectile_type_count; ++i) {
			xirect area = {{
				weapon_button_left(self->size, i), 24, TILE_SIZE, TILE_SIZE
			}};
			xpl_sprite_draw_colored(sprites.weapon_key_sprites[i], area.x, area.y, area.width, area.height,
									i == game.active_weapon ? active_color : inactive_color);
			hotspot_set("weapon", i, area, self->size);
		}
		
		
		// Render weapon cooldown
		const float wcx = weapon_cooldown_left(self->size);
		for (int i = 0; i < 6; ++i) {
			float y = 8 + 8 * i;
			if (game.fire_cooldown < ((int)1 << (8 - i))) {
				xvec4 health_color = xvec4_mix(healthy_color, unhealthy_color, (6.f - i) / 6.f);
				xpl_sprite_draw_colored(sprites.grid8_sprite, wcx, y, 8, 8, health_color);
			} else {
				xpl_sprite_draw_colored(sprites.solid_sprite, wcx, y, 8, 8, solid_black);
			}
		}
		
		// Fire button
		{
			xirect area = {{ fire_button_left(self->size), 0, 64, 64 }};
			xpl_sprite_draw_colored(game.fire_cooldown ? sprites.fire_button_dark : sprites.fire_button_lit, area.x, area.y, area.width, area.height, game.fire_cooldown ? inactive_color : active_color);
			hotspot_set("fire", 0, area, self->size);

			// Coin symbols on fire button
			int coin_symbols = xclamp(1 + game.player[0].score / 250, 1, 4);
			for (int i = 0; i < coin_symbols; ++i) {
				xpl_sprite_draw_colored(sprites.ui_coin_sprite, area.x + ((area.width - 16 * coin_symbols) / 2) + 16 * i, 20, 16, 16, coin_color);
			}

		}
		


		// Render health
		const float hx = health_left(self->size);
		for (int i = 0; i < 256; i += 48) {
			float y = 8 + 8 * (i / 48);
			if (i <= game.player[0].health) {
				xvec4 health_color = xvec4_mix(healthy_color, unhealthy_color, (255.f - i) / 255.f);
				xpl_sprite_draw_colored(sprites.grid8_sprite, hx, y, 8, 8, health_color);
			} else {
				xpl_sprite_draw_colored(sprites.solid_sprite, hx, y, 8, 8, solid_black);
			}
		}
		
		xpl_sprite_batch_matrix_pop(sprites.ui_batch);
	}
	xpl_sprite_batch_end(sprites.ui_batch);
}
