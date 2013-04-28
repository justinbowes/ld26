//
//  xpl_effect_buffer.h
//  p1
//
//  Created by Justin Bowes on 2013-01-22.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_effect_buffer_h
#define p1_xpl_effect_buffer_h

#include "xpl.h"
#include "xpl_gl.h"
#include "xpl_vao.h"
#include "xpl_vec.h"
#include "xpl_effect.h"

enum xpl_buffer_attachment_type {
    xbat_rgba8,
    xbat_srgba8,
    xbat_rgba16f,
    xbat_rgba32f,
    xbat_rgb8,
    xbat_rg16f,
	xbat_r32f,
	xbat_r16ui,
    xbat_r32ui,
    xbat_shadow24,
    xbat_depth24,
    xbat_depth32f,
    xbat_depth24_stencil8,
    xbat_depth32f_stencil8,
};

enum xpl_effect_chain_type {
    xect_none,
    xect_initial_write,
    xect_initial_read
};

typedef struct xpl_buffer_attachment {
    GLenum attachment;
    enum xpl_buffer_attachment_type attach_type;
    GLenum min_filter;
    GLenum mag_filter;
    enum xpl_effect_chain_type chain_type;
} xpl_buffer_attachment_t;


typedef struct xpl_effect_render_state {
	GLint current_draw_fbo;
	GLenum current_draw_buffer;
	GLint current_read_fbo;
	GLenum current_read_buffer;
	GLint bound_textures[8];
} xpl_effect_render_state_t;

typedef struct xpl_effect_buffer {
    xivec2  size;
    GLuint  fbo;
    GLuint  *fb_textures;
    GLuint  *fb_texture_attachments;
    
    GLsizei texture_count;
    GLuint color_attachment_count;
    GLuint read_texture_index;
    GLuint write_texture_index;
    GLuint depth_texture_index;
    
    struct xpl_effect *effects;
    xpl_effect_render_state_t *state;
    int blit_last_effect;

    xpl_vao_t *vao;
    xpl_bo_t *vbo;
    GLuint elements;
} xpl_effect_buffer_t;

typedef struct xpl_effect xpl_effect_t;

xpl_effect_buffer_t *xpl_effect_buffer_new(xivec2 size, xpl_buffer_attachment_t *attachments, size_t num_attachments, int blit_last_effect);
void xpl_effect_buffer_destroy(xpl_effect_buffer_t **effect_buffer);

void xpl_effect_buffer_render_effects(xpl_effect_buffer_t *self, double time);
void xpl_effect_buffer_flip(xpl_effect_buffer_t *self);

struct xpl_effect_def;

#define xpl_effect_buffer_add_effect(b, p, def, data) xpl_effect_buffer_add_effect_named(b, p, #def, def, data)
xpl_effect_t *xpl_effect_buffer_add_effect_named(xpl_effect_buffer_t *self, int priority, const char *name, const struct xpl_effect_def effect_def, void *userdata);
void xpl_effect_buffer_remove_effect(xpl_effect_buffer_t *self, xpl_effect_t **effect);

#endif
