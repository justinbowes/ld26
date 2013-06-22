//
//  context_logo.c
//  p1
//
//  Created by Justin Bowes on 2013-04-04.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "xpl.h"
#include "xpl_l10n.h"
#include "xpl_vao_shader.h"
#include "xpl_text_cache.h"
#include "xpl_easing.h"
#include "xpl_rand.h"

#include "audio/audio.h"
#include "models/informi_brick.h"
#include "models/plane-xy.h"

#include "context/context_logo.h"
#include "context/context_menu.h"

#include "game/prefs.h"

static xpl_context_t *context_next = NULL;

// This carries through.
audio_t *title_bgm = NULL;

// The logo is synchronized to the audio.
static audio_t		*clip;
static bool         clip_started;

static xpl_vao_t    *brick_vao;
static xpl_bo_t     *brick_vbo;
static xpl_shader_t *brick_shader;
static GLsizei      brick_elements;

static xpl_vao_t    *quad_vao;
static xpl_bo_t     *quad_vbo;
static xpl_shader_t *quad_shader;
static GLsizei      quad_elements;
static xvec4        overlay_color;
static float        overlay_strength;

static xpl_text_buffer_t    *logo_text;
static xpl_markup_t         logo_markup;

static xpl_text_buffer_t    *website_text;
static xpl_markup_t         website_markup;

static xpl_text_buffer_t    *outline_text;
static xpl_markup_t         outline_markup;

static xpl_text_buffer_t    *copyright_text;
static xpl_markup_t         copyright_markup;

static xmat4        ortho_mvp;
static xmat4        lookat_vp;

static double total_time = 0.0;

static struct {
    xvec3 origin;
    xvec3 target;
    xvec3 position;
    xvec4 color;
} brick[3];

static void *init(xpl_context_t *self) {
    brick_vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_append(brick_vbo, &brick_vertex_data, sizeof(brick_vertex_data));
    xpl_bo_commit(brick_vbo);
    brick_elements = sizeof(brick_vertex_data) / sizeof(brick_vertex_data[0]);
    brick_vao = xpl_vao_new();
    xpl_vao_define_vertex_attrib(brick_vao, "position", brick_vbo,
                                 3, GL_FLOAT, GL_FALSE,
                                 sizeof(brick_vertex_data[0]), offsetof(vertex_normal_t, vertex));
    
    // Quad for effects
    quad_vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_append(quad_vbo, &plane_xy_vertex_data[0], sizeof(plane_xy_vertex_data));
    xpl_bo_commit(quad_vbo);
    quad_elements = sizeof(plane_xy_vertex_data) / sizeof(plane_xy_vertex_data[0]);
    quad_vao = xpl_vao_new();
    xpl_vao_define_vertex_attrib(quad_vao, "position", quad_vbo,
                                 3, GL_FLOAT, GL_FALSE,
                                 sizeof(plane_xy_vertex_data[0]), offsetof(vertex_normal_t, vertex));
    
    brick_shader = xpl_shader_get_prepared("LogoBackground", "Logo.Vertex", "Logo.Brick.Fragment");
    quad_shader = xpl_shader_get_prepared("LogoForeground", "Overlay.Vertex", "Overlay.Fragment");
    
	clip = audio_create("logo.ogg");
    clip->loop = false;
    clip_started = false;
	
	prefs_t prefs = prefs_get();
	title_bgm = audio_create("title.ogg");
	title_bgm->loop = true;
	if (prefs.bgm_on) {
		title_bgm->volume = 0.5f;
	} else {
		title_bgm->volume = 0.0f;
	}
	title_bgm->action = aa_play;
    
    logo_text = xpl_text_buffer_new(128, 128, 1);
    xpl_markup_clear(&logo_markup);
    xpl_markup_set(&logo_markup, "Candela", 60.f, TRUE, TRUE, xvec4_set(0.f, 0.f, 0.4f, 1.f), xvec4_all(0.0f));
    logo_markup.outline = xfo_none;
    
    website_text = xpl_text_buffer_new(128, 128, 1);
    xpl_markup_clear(&website_markup);
    xpl_markup_set(&website_markup, "Chicago", 24.f, FALSE, FALSE, xvec4_set(0.6f, 0.6f, 0.8f, 1.f), xvec4_all(0.0f));
    logo_markup.outline = xfo_none;
    
    outline_text = xpl_text_buffer_new(128, 128, 1);
    xpl_markup_clear(&outline_markup);
    xpl_markup_set(&outline_markup, "Candela", 60.f, TRUE, TRUE, xvec4_all(1.f), xvec4_all(0.0f));
    outline_markup.outline = xfo_line;
    outline_markup.outline_thickness = 6.f;
    
    copyright_text = xpl_text_buffer_new(128, 128, 1);
    xpl_markup_clear(&copyright_markup);
    xpl_markup_set(&copyright_markup, "CandelaBook", 12.f, FALSE, FALSE, xvec4_all(0.8f), xvec4_all(0.0f));
    logo_markup.outline = xfo_none;
    
    xvec2 pen;
    wchar_t buffer[1024];
    
    xpl_mbs_to_wcs("informi labs", buffer, 1024);
    pen = xvec2_set(360.f, 480.f);
    xpl_text_buffer_add_text(outline_text, &pen, &outline_markup, buffer, 0);
    xpl_text_buffer_commit(outline_text);
    
    pen = xvec2_set(360.f, 480.f);
    xpl_text_buffer_add_text(logo_text, &pen, &logo_markup, buffer, 0);
    xpl_text_buffer_commit(logo_text);

    xpl_mbs_to_wcs("informilabs.com\nultrapew.com", buffer, 1024);
    pen = xvec2_set(360.f, 400.f);
    xpl_text_buffer_add_text(website_text, &pen, &website_markup, buffer, 0);
    xpl_text_buffer_commit(website_text);
    
    char key[64];
    int line = 1;
    while (1) {
        snprintf(key, 64, "logo_copyright%d", line);
        const char *str = xl(key);
        if (strcmp(str, key) == 0) break;

        xpl_mbs_to_wcs(str, buffer, 1024);
        pen = xvec2_set(5.f, 160.f - (15.f * line));
        xpl_text_buffer_add_text(copyright_text, &pen, &copyright_markup, buffer, 0);
        ++line;
    }
    
    xmat4_ortho(0.f, ((float)self->size.height / self->size.width) * 720.f, 0.f, 720.f, 1.f, -1.f, &ortho_mvp);
    
    xvec3 origin = {{ 15.f, 12.f, 20.f }};
    xvec3 target = {{ 6.f, 0.8f, 0.f }};
    xmat4 lookat_v, lookat_p;
    xmat4_look_at(&origin, &target, &xvec3_y_axis, &lookat_v);
    xmat4_perspective(30.f, 16.f/9.f, 0.1f, 100.f, &lookat_p);
    xmat4_multiply(&lookat_p, &lookat_v, &lookat_vp);
    
    for (size_t i = 0; i < 3; ++i) {
        brick[i].origin = xvec3_set(xpl_frand_range(-5.f, 5.f), 1.8 * i + 10.f, xpl_frand_range(-5.f, 5.f));
        brick[i].target = xvec3_set(i % 2, 1.8f * i, 0.f);
        brick[i].position = brick[i].origin;
        brick[i].color = xvec4_set((i + 1) % 3 ? 0.f : 1.f, (i + 2) % 3 ? 0.f : 1.f, i % 3 ? 0.f : 1.f, 0.5f);
    }
    
    context_next = self;
    
    overlay_color = xvec4_all(0.f);
    overlay_strength = 0.f;
    
    GL_DEBUG();
    
    return NULL;
}

static void engine(xpl_context_t *self, double time, void *data) {
    const float delay = 2.5f;
    
    total_time += time; // done after x seconds.
    
    if (total_time < delay) return; // initialize time
    
	if (! clip_started) {
		clip->action = aa_play;
		clip_started = true;
	} else {
		if (total_time >= 4.7) {
			title_bgm->volume *= 2.0f;
		}
		if (! audio_is_playing(clip)) {
			if (total_time < 8.5) return;
			context_next = xpl_context_new(self->app, &menu_context_def);
		}
	}
	
    const double rate = 9.0;
    for (size_t i = 0; i < 3; ++i) {
        float active_brick = rate * (total_time - delay);
        active_brick -= i;
        float this_brick_position = xclamp(active_brick, 0.0, 1.0);
        brick[i].position = xvec3_mix(brick[i].origin, brick[i].target, xpl_ease_bounce_out(this_brick_position));
    }
}

static void render(xpl_context_t *self, double time, void *data) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(brick_shader->id);
    glBlendFunc(GL_ONE, GL_ONE);
    GL_DEBUG();
    for (size_t i = 0; i < 3; ++i) {
        xmat4 m, mvp;
        xmat4_identity(&m);
        xmat4_translate(&m, &brick[i].position, NULL);
        xmat4_multiply(&lookat_vp, &m, &mvp);
        glUniform4fv(xpl_shader_get_uniform(brick_shader, "color"), 1, brick[i].color.data);
        glUniformMatrix4fv(xpl_shader_get_uniform(brick_shader, "mvp"), 1, GL_FALSE, mvp.data);
        GL_DEBUG();
        xpl_vao_program_draw_arrays(brick_vao, brick_shader, GL_TRIANGLES, 0, brick_elements);
    }
    glUseProgram(GL_NONE);

    if (total_time > 4.5) {
        xpl_text_buffer_render(outline_text, ortho_mvp.data);
		xpl_text_buffer_render(website_text, ortho_mvp.data);
        xpl_text_buffer_render(logo_text, ortho_mvp.data);
        GL_DEBUG();
    }
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(quad_shader->id);
    glUniform1f(xpl_shader_get_uniform(quad_shader, "scanline_amount"), total_time * 0.33f);
    glUniform1f(xpl_shader_get_uniform(quad_shader, "strength"), overlay_strength);
    glUniform4fv(xpl_shader_get_uniform(quad_shader, "color"), 1, overlay_color.data);
    xpl_vao_program_draw_arrays(quad_vao, quad_shader, GL_TRIANGLES, 0, (GLsizei)quad_elements);
    glUseProgram(GL_NONE);

	if (total_time > 4.5) {
		xpl_text_buffer_render(website_text, ortho_mvp.data);
        GL_DEBUG();
    }

    
    xpl_text_buffer_render(copyright_text, ortho_mvp.data);
}

static void destroy(xpl_context_t *self, void *vdata) {
	audio_destroy(&clip);
}

static xpl_context_t *handoff(xpl_context_t *self, void *vdata) {
    return context_next;
}

xpl_context_def_t logo_context_def = {
    init,
    engine,
    render,
    destroy,
    handoff
};
