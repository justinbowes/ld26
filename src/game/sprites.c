//
//  sprites.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "xpl_rand.h"
#include "xpl_color.h"

#include "game/game.h"

#include "game/camera.h"
#include "game/palette.h"
#include "game/sprites.h"
#include "game/util.h"
#include "game/projectile_config.h"

sprites_t						sprites;
xivec3							star_layers[STAR_LAYERS][STARS_PER_LAYER];

void sprites_init(void) {
	sprites.playfield_batch = xpl_sprite_batch_new();
	sprites.ui_batch = xpl_sprite_batch_new();
	
	sprites.ship_sprite = xpl_sprite_new(sprites.playfield_batch, "ship.png", NULL);
	sprites.indicator_sprite = xpl_sprite_new(sprites.playfield_batch, "indicator.png", NULL);
	sprites.particle_sprite = xpl_sprite_new(sprites.playfield_batch, "particle.png", NULL);
	sprites.playfield_coin_sprite = xpl_sprite_new(sprites.playfield_batch, "coin.png", NULL);
	sprites.star_sprite = xpl_sprite_new(sprites.playfield_batch, "star.png", NULL);
	
	sprites.panel_background_sprite = xpl_sprite_new(sprites.ui_batch, "panel_background.png", NULL);
	sprites.solid_sprite = xpl_sprite_new(sprites.ui_batch, "tile_solid.png", NULL);
	sprites.grid8_sprite = xpl_sprite_new(sprites.ui_batch, "tile_grid.png", NULL);
	
	for (int i = 0; i < 8; ++i) {
		char resource[PATH_MAX];
		snprintf(resource, PATH_MAX, "weapon_%d.png", i);
		sprites.weapon_key_sprites[i] = xpl_sprite_new(sprites.ui_batch, resource, NULL);
	}
	const char *key_sprites[3] = { "key_thrust.png", "key_left.png", "key_right.png" };
	for (int i = 0; i < 3; ++i) {
		sprites.control_key_sprites[i] = xpl_sprite_new(sprites.ui_batch, key_sprites[i], NULL);
	}
	sprites.ui_coin_sprite = xpl_sprite_new(sprites.ui_batch, "coin.png", NULL);
	
	for (int i = 0; i < STAR_LAYERS; ++i) {
		for (int j = 0; j < STARS_PER_LAYER; ++j) {
			star_layers[i][j].x = xpl_irand_range(0, STAR_LAYER_SIZE);
			star_layers[i][j].y = xpl_irand_range(0, STAR_LAYER_SIZE);
			star_layers[i][j].z = (int)RGBA(0xff, 0xff, 0xff, 0x80);
		}
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
		for (int i = 0; i < STAR_LAYERS; ++i) {
			for (int j = 0; j < STARS_PER_LAYER; ++j) {
				long px = (star_layers[i][j].x - (camera.center.px >> (2 * i + 2))) % STAR_LAYER_SIZE;
				long py = (star_layers[i][j].y - (camera.center.py >> (2 * i + 2))) % STAR_LAYER_SIZE;
				if (px > camera.draw_area.x &&
					py > camera.draw_area.y &&
					px < camera.draw_area.x + camera.draw_area.width &&
					py < camera.draw_area.y + camera.draw_area.height) {
					xvec4 color = RGBA_F((uint32_t)star_layers[i][j].z);
					xpl_sprite_draw_transformed(sprites.star_sprite,
												px, py, 0.f, 0.f,
												10 / (i + 1), 10 / (i + 1),
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
			} else if (game.indicators_on) {
				// Draw indicator
				long dlx = (long)game.player[i].position.px - (long)game.player[0].position.px;
				long dly = (long)game.player[i].position.py - (long)game.player[0].position.py;
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
		xpl_sprite_draw(sprites.panel_background_sprite, 0.f, 0.f, self->size.width, camera.draw_area.y);
		xpl_sprite_draw(sprites.panel_background_sprite, 0.f, camera.draw_area.y + camera.draw_area.height, self->size.width, camera.draw_area.y);
		
		for (int i = 0; i < 3; ++i) {
			xpl_sprite_draw_colored(sprites.control_key_sprites[i], 8 + (TILE_SIZE + 8) * i, 24, TILE_SIZE, TILE_SIZE,
									game.control_indicator_on[i] ? active_color : inactive_color);
		}
		
		xpl_sprite_draw_colored(sprites.ui_coin_sprite, 192, 4, 16, 16, coin_color);
		for (int i = 0; i < 8; ++i) {
			xpl_sprite_draw_colored(sprites.weapon_key_sprites[i], 192 + (TILE_SIZE + 8) * i, 24, TILE_SIZE, TILE_SIZE,
									i == game.active_weapon ? active_color : inactive_color);
		}
		
		int coin_symbols = xclamp(1 + game.player[0].score / 250, 1, 4);
		for (int i = 0; i < coin_symbols; ++i) {
			xpl_sprite_draw_colored(sprites.ui_coin_sprite, self->size.width - 16 - (coin_symbols - i) * 16, 12, 16, 16, coin_color);
		}
		
		// Render health
		for (int i = 0; i < 256; i += 48) {
			float y = 8 + 8 * (i / 48);
			if (i <= game.player[0].health) {
				xvec4 health_color = xvec4_mix(healthy_color, unhealthy_color, (255.f - i) / 255.f);
				xpl_sprite_draw_colored(sprites.grid8_sprite, 512, y, 8, 8, health_color);
			} else {
				xpl_sprite_draw_colored(sprites.solid_sprite, 512, y, 8, 8, solid_black);
			}
		}
		
		// Render weapon cooldown
		for (int i = 0; i < 6; ++i) {
			float y = 8 + 8 * i;
			if (game.fire_cooldown < ((int)1 << (8 - i))) {
				xvec4 health_color = xvec4_mix(healthy_color, unhealthy_color, (6.f - i) / 6.f);
				xpl_sprite_draw_colored(sprites.grid8_sprite, 176, y, 8, 8, health_color);
			} else {
				xpl_sprite_draw_colored(sprites.solid_sprite, 176, y, 8, 8, solid_black);
			}
		}
		
		xpl_sprite_batch_matrix_pop(sprites.ui_batch);
	}
	xpl_sprite_batch_end(sprites.ui_batch);
}
