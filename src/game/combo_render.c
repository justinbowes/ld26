//
//  combo_render.c
//  app
//
//  Created by Justin Bowes on 2013-07-05.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>

#include <xpl_vec2.h>
#include <xpl_rand.h>
#include <xpl_text_cache.h>

#include "game/game.h"
#include "game/camera.h"

static xpl_text_cache_t *combo_cache;
static xpl_markup_t		*combo_markup;

void combo_init(void) {
	combo_cache = xpl_text_cache_new(128);
	combo_markup = xpl_markup_new();
	xpl_markup_set(combo_markup, "Chicago", 24.f, FALSE, FALSE, xvec4_all(1.f), xvec4_all(0.f));
}

void combo_render(int count, bool start_audio, xivec2 screen) {
	xmat4 ortho, translated_ortho;
	char key[64];
	char message[64];
	snprintf(key, 64, "combo_%d", count);
	bool template = false;
	if (! xl_exists(key)) {
		template = true;
		strncpy(key, "combo_n", 63);
	}
	
	const char *localized = xl(key);
	if (template) {
		snprintf(message, 63, localized, count);
	} else {
		strncpy(message, localized, 63);
	}
	if (start_audio) {
		audio_quickplay_pan(key, 1.0f, 0.5f);
	}
	
	xpl_cached_text_t *text = xpl_text_cache_get(combo_cache, combo_markup, message);
	float text_length = xpl_font_get_text_length(text->managed_font, message, -1);
	xmat4_ortho(0.f, screen.width, 0.f, screen.height, -1.f, 1.f, &ortho);

	xvec4 color = xvec4_set(0.4f, 0.0f, 0.0f, xclamp(game.combo_timeout - 10.f, 0.f, 1.f));
	color.a *= 0.5;
	if (color.a > 0.f) {
		for (int i = 0; i < 2; ++i) {
			xvec3 v = {{
				(screen.x  - text_length ) / 2 + xpl_frand() * 4,
				(screen.y - combo_markup->size) * 0.75 + xpl_frand() * 4,
				0.f
			}};
			xmat4_translate(&ortho, &v, &translated_ortho);
			xpl_text_buffer_render_tinted(text->buffer, translated_ortho.data, color);
			color.a += 2.0;
		}
	}
}