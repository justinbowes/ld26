//
//  xpl_effect_buffer.c
//  p1
//
//  Created by Justin Bowes on 2013-01-22.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <assert.h>
#include <stdbool.h>

#include "uthash.h"

#include "xpl_gl.h"
#include "xpl_effect_buffer.h"
#include "xpl_app.h"
#include "xpl_model.h"

static const vertex_normal_t plane_xy_vertex_data[] = {
	{ /*v*/ {{-1.000000, -1.000000, 0.000000}}, 	/*n*/ {{0.000000, 0.000000, 1.000000}} },
	{ /*v*/ {{1.000000, -1.000000, 0.000000}}, 	/*n*/ {{0.000000, 0.000000, 1.000000}} },
	{ /*v*/ {{1.000000, 1.000000, 0.000000}}, 	/*n*/ {{0.000000, 0.000000, 1.000000}} },
	{ /*v*/ {{-1.000000, 1.000000, 0.000000}}, 	/*n*/ {{0.000000, 0.000000, 1.000000}} },
	{ /*v*/ {{-1.000000, -1.000000, 0.000000}}, 	/*n*/ {{0.000000, 0.000000, 1.000000}} },
	{ /*v*/ {{1.000000, 1.000000, 0.000000}}, 	/*n*/ {{0.000000, 0.000000, 1.000000}} },
};

#define PIXEL_STEP 2

struct xpl_effect {
    int priority;
    bool flip;
    xpl_effect_def_t funcs;
    void *userdata;
    const char *name;
    UT_hash_handle hh;
};

static bool isDepthAttachment(GLenum attachment) {
#ifdef XPL_PLATFORM_IOS
	return attachment == GL_DEPTH_ATTACHMENT;
#else
	return attachment == GL_DEPTH_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT;
#endif
}

static void create_buffer_texture(xpl_effect_buffer_t *self, size_t texture_index, xpl_buffer_attachment_t *attachment_info) {
    GLenum internal_format;
    GLenum format;
    GLenum type;
    switch (attachment_info->attach_type) {
#ifdef XPL_PLATFORM_IOS
		case xbat_rgba8:
		case xbat_srgba8:
		case xbat_rgb8:
			internal_format = GL_RGBA;
			format = GL_RGBA;
			type = GL_UNSIGNED_INT;
			break;
		case xbat_rgba16f:
		case xbat_rgba32f:
		case xbat_rg16f:
			internal_format = GL_RGBA;
			format = GL_RGBA;
			type = GL_FLOAT;
			break;
		case xbat_depth24:
		case xbat_depth32f:
			internal_format = GL_DEPTH_COMPONENT;
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
			break;
		case xbat_depth24_stencil8:
		case xbat_depth32f_stencil8:
			// OES_packed_depth_stencil somehow
			// Fall through to default
#else
        case xbat_rgba8:
            internal_format = GL_RGBA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
            
        case xbat_srgba8:
            internal_format = GL_SRGB8_ALPHA8;
            format = GL_BGRA;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
            
        case xbat_rgba16f:
            internal_format = GL_RGBA16F;
            format = GL_BGRA;
            type = GL_FLOAT;
            break;
            
        case xbat_rgba32f:
            internal_format = GL_RGBA32F;
            format = GL_RGBA;
            type = GL_FLOAT;
            break;
            
        case xbat_rgb8:
            internal_format = GL_RGB8;
            format = GL_BGR;
            type = GL_UNSIGNED_INT_8_8_8_8_REV;
            break;
            
        case xbat_rg16f:
            internal_format = GL_RG16F;
            format = GL_RG;
            type = GL_FLOAT;
            break;
			
		case xbat_r32f:
			internal_format = GL_R32F;
			format = GL_RED;
			type = GL_FLOAT;
			break;
            
		case xbat_r16ui:
			internal_format = GL_R16UI;
			format = GL_RED_INTEGER;
			type = GL_UNSIGNED_SHORT;
			break;

        case xbat_r32ui:
            internal_format = GL_R32UI;
            format = GL_RED_INTEGER;
            type = GL_UNSIGNED_INT;
            break;
        case xbat_depth24:
            internal_format = GL_DEPTH_COMPONENT24;
            format = GL_DEPTH_COMPONENT;
            type = GL_FLOAT;
            break;
            
        case xbat_depth32f:
            internal_format = GL_DEPTH_COMPONENT32F;
            format = GL_DEPTH_COMPONENT;
            type = GL_FLOAT;
            break;
            
        case xbat_depth24_stencil8:
            internal_format = GL_DEPTH24_STENCIL8;
            format = GL_DEPTH_STENCIL;
            type = GL_UNSIGNED_INT_24_8;
            break;
            
        case xbat_depth32f_stencil8:
            internal_format = GL_DEPTH32F_STENCIL8;
            format = GL_DEPTH_STENCIL;
            type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            break;
#endif

        default:
            LOG_ERROR("Unrecognized buffer attachment type");
            assert(0);
            break;
    }
    
    switch (attachment_info->chain_type) {
        case xect_initial_read:
            self->read_texture_index = (GLuint)texture_index;
            break;
            
        case xect_initial_write:
            self->write_texture_index = (GLuint)texture_index;
            break;
            
        default:
            break;
    }
    
    glActiveTexture(GL_TEXTURE0 + (GLenum)texture_index);
    glBindTexture(GL_TEXTURE_2D, self->fb_textures[texture_index]);
    GL_DEBUG();
	GL_DEBUG_THIS(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, self->size.width, self->size.height, 0, format, type, NULL));

    if (isDepthAttachment(attachment_info->attachment)) {
#ifndef XPL_PLATFORM_IOS
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
#endif
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        self->depth_texture_index = (GLuint)texture_index;
    } else {
        // color attachment case
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, attachment_info->min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, attachment_info->mag_filter);
        if (attachment_info->min_filter == GL_LINEAR_MIPMAP_LINEAR ||
        		attachment_info->min_filter == GL_LINEAR_MIPMAP_NEAREST ||
        		attachment_info->min_filter == GL_NEAREST_MIPMAP_LINEAR ||
        		attachment_info->min_filter == GL_NEAREST_MIPMAP_NEAREST) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }  
        self->color_attachment_count++;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_info->attachment, GL_TEXTURE_2D, self->fb_textures[texture_index], 0);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    self->fb_texture_attachments[texture_index] = attachment_info->attachment;
    
    GL_DEBUG();
    
}

static void create_buffer_effect_quad(xpl_effect_buffer_t *self) {
    // Quad for effects
    GLsizei element_size = sizeof(plane_xy_vertex_data[0]);
    
    self->vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_append(self->vbo, &plane_xy_vertex_data[0], sizeof(plane_xy_vertex_data));
    xpl_bo_commit(self->vbo);
    self->elements = sizeof(plane_xy_vertex_data) / element_size;
    
    self->vao = xpl_vao_new();
    xpl_vao_define_vertex_attrib(self->vao, "position", self->vbo, 3, GL_FLOAT, GL_FALSE, element_size, offsetof(vertex_normal_t, vertex));
}

xpl_effect_buffer_t *xpl_effect_buffer_new(xivec2 size, xpl_buffer_attachment_t *attachments, size_t num_attachments, int blit_last_effect) {
    xpl_effect_buffer_t *buf = xpl_calloc_type(xpl_effect_buffer_t);

    int width_step = (size.width + PIXEL_STEP - 1) / PIXEL_STEP * PIXEL_STEP;
	int height_step = (size.height + PIXEL_STEP - 1) / PIXEL_STEP * PIXEL_STEP;
	buf->size = xivec2_set(width_step, height_step);
	LOG_DEBUG("framebuffer size: requested %dx%d, created %dx%d", size.width, size.height, width_step, height_step);
    
    buf->texture_count = (GLsizei)num_attachments;
    buf->fb_textures = xpl_calloc(buf->texture_count * sizeof(GLuint));
    buf->fb_texture_attachments = xpl_calloc(buf->texture_count * sizeof(GLuint));
    buf->blit_last_effect = blit_last_effect;
    buf->state = xpl_calloc_type(xpl_effect_render_state_t);
    
    GL_DEBUG();

    // Framebuffer
	glGenFramebuffers(1, &buf->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, buf->fbo);
	GL_DEBUG();

#ifndef XPL_PLATFORM_IOS
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	GL_DEBUG();
#endif

    // Create the textures
    glGenTextures(buf->texture_count, buf->fb_textures);
    for (size_t i = 0; i < num_attachments; ++i) {
        create_buffer_texture(buf, i, &attachments[i]);
    }
    
    //@ SRGB framebuffer?
#ifndef XPL_PLATFORM_IOS
    glEnable(GL_FRAMEBUFFER_SRGB);
#endif

    // Check whether the arguments create a valid buffer
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Error creating framebuffer, status: 0x%x", status);
        xpl_gl_breakpoint_func();
        return NULL;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    create_buffer_effect_quad(buf);
    
    GL_DEBUG();
    
    return buf;
}

void xpl_effect_buffer_destroy(xpl_effect_buffer_t **ppbuffer) {
    assert(ppbuffer);
    xpl_effect_buffer_t *buf = *ppbuffer;
    assert(buf);
    
    glDeleteTextures(buf->texture_count, buf->fb_textures);
    glDeleteFramebuffers(1, &buf->fbo);
    
    xpl_vao_destroy(&buf->vao);
    xpl_bo_destroy(&buf->vbo);
    
    while (buf->effects) {
        // Due to the nulled-yer-pointer semantics of XPL delete, we need
        // our own pointer to null, not the hash head.
        xpl_effect_t *effect = buf->effects;
        xpl_effect_buffer_remove_effect(buf, &effect);
    }
    
    xpl_free(buf);
    *ppbuffer = NULL;
}

void xpl_effect_buffer_flip(xpl_effect_buffer_t *self) {
    GLuint read_tex = self->read_texture_index;
    self->read_texture_index = self->write_texture_index;
    self->write_texture_index = read_tex;
}

static void render_quad(xpl_effect_buffer_t *self, xpl_shader_t *shader) {
    // After repeated fuckups, we have to make sure we're actually using the shader we pass here.
    // glUseProgram(shader->program_handle); fuckups ahoy!
    xpl_vao_program_draw_arrays(self->vao, shader, GL_TRIANGLES, 0, self->elements);
}

static void set_current_draw_fbo(xpl_effect_buffer_t *self, GLint fbo) {
	if (fbo == self->state->current_draw_fbo) return;
	self->state->current_draw_fbo = fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    GL_DEBUG();
}

static void set_current_read_fbo(xpl_effect_buffer_t *self, GLint fbo) {
	if (fbo == self->state->current_read_fbo) return;
	self->state->current_read_fbo = fbo;
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	GL_DEBUG();
}

static void set_current_draw_buffer(xpl_effect_buffer_t *self, GLenum attachment) {
	if (attachment == self->state->current_draw_buffer) return;
	self->state->current_draw_buffer = attachment;
	glDrawBuffer(attachment);
}

static void set_current_read_buffer(xpl_effect_buffer_t *self, GLenum attachment) {
	if (attachment == self->state->current_read_buffer) return;
	self->state->current_read_buffer = attachment;
	glReadBuffer(attachment);
}

static void clear_assumed_state(xpl_effect_buffer_t *self) {
	self->state->current_draw_fbo = GL_DONT_CARE;
	self->state->current_draw_buffer = GL_DONT_CARE;
	self->state->current_read_fbo = GL_DONT_CARE;
	self->state->current_read_buffer = GL_DONT_CARE;
}

static struct xpl_effect *effect_tmp[128];
void xpl_effect_buffer_render_effects(xpl_effect_buffer_t *self, double time) {

    glViewport(0, 0, self->size.width, self->size.height);
	clear_assumed_state(self);

    size_t effect_count = HASH_COUNT(self->effects);
    if (effect_count) {
        // Clear the effect buffer
    	set_current_draw_fbo(self, self->fbo);
    	set_current_read_fbo(self, GL_NONE);
        glDepthMask(GL_TRUE);

        if (self->write_texture_index) {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);
            // This is a flip chain. Clear everything but the second color buffer.
        	set_current_draw_buffer(self, self->fb_texture_attachments[self->write_texture_index]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            xpl_effect_buffer_flip(self);
            // Clear the second color buffer.
        	set_current_draw_buffer(self, self->fb_texture_attachments[self->write_texture_index]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDisable(GL_STENCIL_TEST);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
        }
        // Otherwise, this is not a flippable chain, so it has unknown semantics.

        
        xpl_effect_t *effect, *tmp;
        size_t index = 0;
        size_t last_flip = 0;
        HASH_ITER(hh, self->effects, effect, tmp) {
            effect_tmp[index] = effect;
            if (effect->flip) {
                last_flip = index;
            }
            index++;
        }

        int last_effect_drew = FALSE;
        for (size_t i = 0; i < effect_count; ++i) {
            if (effect_tmp[i]->flip) {
                if (last_effect_drew) {
                    xpl_effect_buffer_flip(self);
                	set_current_draw_buffer(self, self->fb_texture_attachments[self->write_texture_index]);
                }
            }
            if (i >= last_flip && self->blit_last_effect) {
            	set_current_draw_fbo(self, GL_NONE);
            	set_current_draw_buffer(self, GL_BACK);
            }
            last_effect_drew = effect_tmp[i]->funcs.effect(self, time, render_quad, effect_tmp[i]->userdata);
            GL_DEBUG();
        }

    	set_current_draw_fbo(self, GL_NONE);

        glBindTexture(GL_TEXTURE_2D, GL_NONE);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        GL_DEBUG();
    }
}

static int priority_desc_sort(xpl_effect_t *a, xpl_effect_t *b) {
    return (a->priority - b->priority);
}

xpl_effect_t *xpl_effect_buffer_add_effect_named(xpl_effect_buffer_t *self, int priority, const char *name, const xpl_effect_def_t def, void *userdata) {
    xpl_effect_t *effect = xpl_calloc_type(xpl_effect_t);
    effect->priority = priority;
    effect->funcs = def;
    effect->userdata = userdata;
    effect->name = name;
    
    HASH_ADD_INT(self->effects, priority, effect);
    HASH_SORT(self->effects, priority_desc_sort);
    
    effect->flip = effect->funcs.init(self, effect->userdata);
    
    return effect;
}

void xpl_effect_buffer_remove_effect(xpl_effect_buffer_t *self, xpl_effect_t **ppeffect) {
    xpl_effect_t *effect = *ppeffect;
    HASH_DEL(self->effects, effect);
    effect->funcs.destroy(self, effect->userdata);
    xpl_free(effect);
    *ppeffect = NULL;
}
