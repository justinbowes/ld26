/*
 * xpl_hl.c
 *
 *  Created on: 2012-12-21
 *      Author: Justin
 */

#include <assert.h>
#include <stddef.h>

#include "utarray.h"
#include "uthash.h"

#include "xpl.h"
#include "xpl_gl_debug.h"
#include "xpl_vao.h"
#include "xpl_bo.h"
#include "xpl_memory.h"
#include "xpl_log.h"
#include "xpl_vec.h"
#include "xpl_color.h"
#include "xpl_texture.h"
#include "xpl_text_buffer.h"

#include "xpl_sprite.h"

#define MAX_TEXTURES 8

typedef struct sprite_vertex {
	float x;
	float y;
	float u;
	float v;
	xvec4 color;
} sprite_vertex_t;
static const size_t sprite_vertex_size = sizeof(sprite_vertex_t);

struct xpl_sprite {
	struct xpl_sprite_batch         *batch;
	struct texture_entry            *texture;
	const int                       *blend_funcs;
	xrect                           region;
};

typedef struct sprite_vao {
	xpl_vao_t                       *vao;
	xpl_bo_t                        *vbo;
	size_t                          vertices;
	size_t                          quads;
	size_t                          indexed_quads;
} sprite_vao_t;

typedef struct sprite_batch_command {
	struct xpl_sprite               *sprite;
	xvec2                           pos;
	xvec2                           origin;
	xvec2                           size;
	xvec2                           scale;
	float                           rot_radians;
    xmat4                           matrix;
    xvec4                           color;
	int                             executed;
} sprite_batch_command_t;

struct xpl_sprite_batch {

	UT_array                        *matrix_stack;

	struct texture_entry {
		char						resource[PATH_MAX];
		xpl_texture_t               *texture;
		UT_hash_handle              hh;
	}                               *texture_table;
	struct texture_entry            *current_texture;


	struct gl_state_info {
		int                         unknown;
		xpl_shader_t                *active_shader;
		const int                   *blend_funcs;
		int                         depth_mask;
        int                         cull_face;
		int                         active_texture;
		int                         bound_texture[MAX_TEXTURES];
        xvec4                       draw_color;
	} gl_state;

	UT_array                        *commands;
	UT_array                        *vaos;
	xpl_shader_t                    *current_shader;
	sprite_vao_t                    *current_vao;
	int                             started;

};


static void draw_command(xpl_sprite_batch_t *self, sprite_batch_command_t *cmd) {
	// port from
	// https://github.com/libgdx/libgdx/blob/master/gdx/src/com/badlogic/gdx/graphics/g2d/SpriteBatch.java
	const xpl_sprite_t *sprite = cmd->sprite;

	sprite_vao_t *vao = self->current_vao;

	if (self->gl_state.depth_mask || self->gl_state.unknown) {
		self->gl_state.depth_mask = GL_FALSE;
		glDepthMask(GL_FALSE);
	}
    
    if (self->gl_state.cull_face || self->gl_state.unknown) {
        self->gl_state.cull_face = GL_FALSE;
        glDisable(GL_CULL_FACE);
    }

	xpl_shader_t *sprite_shader = self->current_shader;
	if (self->gl_state.active_shader != sprite_shader || self->gl_state.unknown) {
		self->gl_state.active_shader = sprite_shader;
		glUseProgram(sprite_shader->id);
		glUniformMatrix4fv(xpl_shader_get_uniform(sprite_shader, "mvp"), 1, GL_FALSE, &cmd->matrix.data[0]);
	}

	if (self->gl_state.blend_funcs != sprite->blend_funcs || self->gl_state.unknown) {
		const int *bf = self->gl_state.blend_funcs = sprite->blend_funcs;
        glEnable(GL_BLEND);
        glBlendFuncSeparate(bf[0], bf[1], bf[2], bf[3]);
	}

	if (self->gl_state.active_texture != GL_TEXTURE0 || self->gl_state.unknown) {
		self->gl_state.active_texture = GL_TEXTURE0;
		glActiveTexture(GL_TEXTURE0);
	}

	int texno = self->gl_state.active_texture - GL_TEXTURE0;
	GLuint ftid = cmd->sprite->texture->texture->texture_id;
	if (self->gl_state.bound_texture[texno] != ftid || self->gl_state.unknown) {
		self->gl_state.bound_texture[texno] = ftid;
		glBindTexture(GL_TEXTURE_2D, ftid);
		glUniform1i(xpl_shader_get_uniform(sprite_shader, "tex"), 0);
	}
    
	const float scale_x = cmd->scale.x;
	const float scale_y = cmd->scale.y;
	const float world_origin_x = cmd->pos.x + cmd->origin.x;
	const float world_origin_y = cmd->pos.y + cmd->origin.y;
	float fx = -cmd->origin.x;
	float fy = -cmd->origin.y;
	float fx2 = cmd->size.x - cmd->origin.x;
	float fy2 = cmd->size.y - cmd->origin.y;

	if (scale_x != 1.0f || scale_y != 1.0f) {
		fx *= scale_x;
		fy *= scale_y;
		fx2 *= scale_x;
		fy2 *= scale_y;
	}

	const float p1x = fx;
	const float p1y = fy;
	const float p2x = fx;
	const float p2y = fy2;
	const float p3x = fx2;
	const float p3y = fy2;
	const float p4x = fx2;
	const float p4y = fy;

	float x1;
	float y1;
	float x2;
	float y2;
	float x3;
	float y3;
	float x4;
	float y4;

	const float rot = cmd->rot_radians;
	if (rot != 0.0f) {
		const float cos = cosf(rot);
		const float sin = sinf(rot);

		x1 = cos * p1x - sin * p1y;
		y1 = sin * p1x + cos * p1y;
		x2 = cos * p2x - sin * p2y;
		y2 = sin * p2x + cos * p2y;
		x3 = cos * p3x - sin * p3y;
		y3 = sin * p3x + cos * p3y;
		x4 = x1 + (x3 - x2);
		y4 = y3 - (y2 - y1);
	} else {
		x1 = p1x;
		y1 = p1y;
		x2 = p2x;
		y2 = p2y;
		x3 = p3x;
		y3 = p3y;
		x4 = p4x;
		y4 = p4y;
	}

	x1 += world_origin_x;
	y1 += world_origin_y;
	x2 += world_origin_x;
	y2 += world_origin_y;
	x3 += world_origin_x;
	y3 += world_origin_y;
	x4 += world_origin_x;
	y4 += world_origin_y;

	const float u = sprite->region.x;
	const float v = sprite->region.y;
	const float u2 = sprite->region.x + sprite->region.width;
	const float v2 = sprite->region.y + sprite->region.height;

//	const uint32_t color = RGBA_F(cmd->color);
//	sprite_vertex_t vtx[4] = {
//			{ x1, y1, u, v, color },
//			{ x2, y2, u, v2, color },
//			{ x3, y3, u2, v2, color },
//			{ x4, y4, u2, v, color }
//	};
    sprite_vertex_t vtx[4] = {
        { x1, y1, u, v, cmd->color },
        { x2, y2, u, v2, cmd->color },
        { x3, y3, u2, v2, cmd->color },
        { x4, y4, u2, v, cmd->color }
	};


	xpl_bo_append(vao->vbo, vtx, sizeof(vtx));

	// Nothing changes about the indices. Reuse if possible.
	vao->quads++;
	if (vao->quads > vao->indexed_quads) {
		unsigned short i = vao->vertices;
		unsigned short indices[6] = { i, i+1, i+2, i+2, i+3, i };
		xpl_bo_append(vao->vao->index_bos[0], indices, sizeof(indices));
		vao->indexed_quads++;
	}
	vao->vertices += 4;

	self->gl_state.unknown = FALSE;
    
    GL_DEBUG();
}

XPLINLINE int command_similar(sprite_batch_command_t *a, sprite_batch_command_t *b) {
	if (a->sprite->texture != b->sprite->texture) return FALSE;
	// other checks? Custom shaders?
	return TRUE;
}

// ----------------------------------------------------------------------------------------------

static void xmat4_icd_init(void *_init) {
	xmat4 *init = (xmat4 *) _init;
	xmat4_identity(init);
}

static void sprite_vao_icd_init(void *_init) {
	sprite_vao_t *init = (sprite_vao_t *)_init;
	xpl_vao_t *vao = init->vao = xpl_vao_new();
	xpl_bo_t *vbo = init->vbo = xpl_bo_new(GL_ARRAY_BUFFER,
			GL_DYNAMIC_DRAW);
	xpl_vao_define_vertex_attrib(vao, "position", vbo, 2, GL_FLOAT, GL_FALSE,
			sprite_vertex_size, offsetof(sprite_vertex_t, x) );
	xpl_vao_define_vertex_attrib(vao, "uv", vbo, 2, GL_FLOAT, GL_FALSE,
			sprite_vertex_size, offsetof(sprite_vertex_t, u) );
	xpl_vao_define_vertex_attrib(vao, "color", vbo, 4, GL_FLOAT,
			GL_FALSE, sprite_vertex_size, offsetof(sprite_vertex_t, color) );
	xpl_vao_set_index_buffer(vao, 0, xpl_bo_new(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW));

	init->indexed_quads = 0;
	init->quads = 0;
	init->vertices = 0;


}

static void sprite_vao_icd_destroy(void *_destroy) {
	sprite_vao_t *destroy = (sprite_vao_t *)_destroy;
	xpl_bo_destroy(&destroy->vbo);
	xpl_bo_destroy(&destroy->vao->index_bos[0]);
	xpl_vao_destroy(&destroy->vao);
}

UT_icd xmat4_icd = { sizeof(xmat4), xmat4_icd_init, NULL, NULL };
UT_icd command_icd = { sizeof(sprite_batch_command_t), NULL, NULL, NULL };
UT_icd sprite_vao_icd = { sizeof(sprite_vao_t), sprite_vao_icd_init, NULL, sprite_vao_icd_destroy };

xpl_sprite_batch_t * xpl_sprite_batch_new() {
	xpl_sprite_batch_t *self = xpl_calloc_type(xpl_sprite_batch_t);

	utarray_new(self->matrix_stack, &xmat4_icd);
	utarray_extend_back(self->matrix_stack);
	// 1 element: identity

	for (size_t i = 0; i < MAX_TEXTURES; ++i) {
		self->gl_state.bound_texture[i] = -1;
	}

	utarray_new(self->vaos, &sprite_vao_icd);

	xpl_shader_t *sprite_shader = self->current_shader = xpl_shader_get("Sprite");
	if (! sprite_shader->linked) {
		xpl_shader_add(sprite_shader, GL_VERTEX_SHADER, "Sprite.Vertex");
		xpl_shader_add(sprite_shader, GL_FRAGMENT_SHADER, "Sprite.Fragment");
		xpl_shader_link(sprite_shader);
	}

	utarray_new(self->commands, &command_icd);

	return self;
}

void xpl_sprite_batch_destroy(xpl_sprite_batch_t **ppbatch) {
	assert(ppbatch);
	xpl_sprite_batch_t *batch = *ppbatch;

	utarray_free(batch->matrix_stack);
	struct texture_entry *el, *tmp;
	HASH_ITER(hh, batch->texture_table, el, tmp)
	{
		HASH_DEL(batch->texture_table, el);
		xpl_texture_destroy(&el->texture);
		xpl_free(el);
	}

	utarray_free(batch->commands);
	utarray_free(batch->vaos);

	xpl_free(batch);
	*ppbatch = NULL;
}

void xpl_sprite_batch_begin(xpl_sprite_batch_t *self) {
	assert(! self->started);
	self->started = TRUE;
	self->gl_state.unknown = TRUE;
	self->current_vao = NULL;
	utarray_clear(self->commands);
	while(utarray_len(self->matrix_stack) > 1) {
		utarray_pop_back(self->matrix_stack);
	}
	sprite_vao_t *sv = NULL;
	while ((sv = (sprite_vao_t *)utarray_next(self->vaos, sv))) {
		xpl_bo_clear(sv->vbo);
		sv->quads = 0;
		sv->vertices = 0;
	}
}

void xpl_sprite_batch_end(xpl_sprite_batch_t *self) {
	assert(self->started);

	// Execute the drawing commands in order, sort of.
	UT_array *commands = self->commands;
	LOG_TRACE("Drawing %d commands", utarray_len(commands));
	sprite_batch_command_t *head_cmd = NULL;
	self->current_vao = NULL;
	int finished = FALSE;
	UT_array *vaos = self->vaos;

	while (!finished && ((head_cmd = (sprite_batch_command_t *)utarray_next(commands, head_cmd)) != NULL)) {
		if (head_cmd->executed) continue;

		self->current_vao = (sprite_vao_t *)utarray_next(vaos, self->current_vao);
		if (self->current_vao == NULL) {
			utarray_extend_back(self->vaos);
			self->current_vao = (sprite_vao_t *)utarray_back(vaos);
		}

		draw_command(self, head_cmd);
		head_cmd->executed = TRUE;

		finished = TRUE;
		sprite_batch_command_t *linked_cmd = head_cmd;
		while((linked_cmd = (sprite_batch_command_t *)utarray_next(commands, linked_cmd))) {
			if (linked_cmd->executed) continue;
			if (command_similar(head_cmd, linked_cmd)) {
				draw_command(self, linked_cmd);
				linked_cmd->executed = TRUE;
			} else {
				finished = FALSE;
			}
		}

		// When all the similar commands have executed, draw this VAO.
		sprite_vao_t *sprite_vao = self->current_vao;
		xpl_bo_commit(sprite_vao->vbo);
		xpl_bo_commit(sprite_vao->vao->index_bos[0]);
		xpl_vao_program_draw_elements_count_offset(sprite_vao->vao, self->current_shader, GL_TRIANGLES, 0, 6 * sprite_vao->quads, 0);
	}
	self->started = FALSE;
}

xmat4 *xpl_sprite_batch_matrix_push(xpl_sprite_batch_t *self) {
	assert(self);
	UT_array *a = self->matrix_stack;
	utarray_push_back(a, (xmat4 *) utarray_back(a)); // copy last element
	return (xmat4 *) utarray_back(a);
}

void xpl_sprite_batch_matrix_pop(xpl_sprite_batch_t *self) {
	utarray_pop_back(self->matrix_stack);
}

struct xpl_sprite *xpl_sprite_new(xpl_sprite_batch_t *batch, const char *resource, const xirect *region) {
	xpl_sprite_t *sprite = xpl_calloc_type(xpl_sprite_t);
	sprite->batch = batch;
	sprite->blend_funcs = BLEND_FUNCS_PREMULT;

	struct texture_entry *entry;
	HASH_FIND_STR(batch->texture_table, resource, entry);
	if (!entry) {
		char resource_path[PATH_MAX];
		snprintf(resource_path, PATH_MAX, "bitmaps/%s", resource);
		xpl_texture_t *texture = xpl_texture_new();
		assert(xpl_texture_load(texture, resource_path, false));
		entry = xpl_alloc_type(struct texture_entry);
		strncpy(entry->resource, resource, PATH_MAX);
		entry->texture = texture;
		HASH_ADD_STR(batch->texture_table, resource, entry);
	}

	sprite->texture = entry;

	// transform region in pixels into 0..1 UV
	if (region == NULL) {
		sprite->region = xrect_set(0.f, 0.f, 1.f, 1.f);
	} else {
		xvec2 size = xvec2_set(entry->texture->size.x, entry->texture->size.y);
		sprite->region = xrect_set(region->x / size.x,
								   region->y / size.y,
								   region->width / size.x,
								   region->height / size.y);
//		This half-pixel fudge is suspiciously unnecessary
//		sprite->region = xrect_set((region->x + 0.5f) / size.x,
//								   (region->y + 0.5f) / size.y,
//								   (region->width - 1.0f) / size.x,
//								   (region->height - 1.0f) / size.y);
	}

	return sprite;
}

void xpl_sprite_destroy(struct xpl_sprite **ppsprite) {
	assert(ppsprite);
	xpl_sprite_t *sprite = *ppsprite;
	assert(sprite);
	xpl_free(sprite);
	*ppsprite = NULL;
}

void xpl_sprite_set_blend_funcs(struct xpl_sprite *sprite, const int *blend_funcs) {
	sprite->blend_funcs = blend_funcs;
}

void xpl_sprite_draw(struct xpl_sprite *sprite, float x, float y, float width, float height) {
	xpl_sprite_draw_transformed(sprite, x, y, 0.f, 0.f, width, height, 1.f,	1.f, 0.f, NULL);
}

void xpl_sprite_draw_colored(struct xpl_sprite *sprite, float x, float y, float width, float height, xvec4 color) {
	xpl_sprite_draw_transformed(sprite, x, y, 0.f, 0.f, width, height, 1.f,	1.f, 0.f, &color);
}

void xpl_sprite_draw_transformed(struct xpl_sprite *sprite,
                                 float x, float y, float origin_x, float origin_y,
                                 float width, float height, float scale_x, float scale_y,
                                 float rotation_rads,
                                 xvec4 *color) {
    
	assert(sprite->batch->started);
	assert(sprite);
    
    static xvec4 white = {{ 1.f, 1.f, 1.f, 1.f }};
    
	utarray_extend_back(sprite->batch->commands);
	sprite_batch_command_t *cmd = (sprite_batch_command_t *) utarray_back(sprite->batch->commands);
	cmd->pos.x = x;
	cmd->pos.y = y;
	cmd->origin.x = origin_x;
	cmd->origin.y = origin_y;
	cmd->size.x = width;
	cmd->size.y = height;
	cmd->scale.x = scale_x;
	cmd->scale.y = scale_y;
	cmd->rot_radians = rotation_rads;
    cmd->matrix = *((xmat4 *)utarray_back(sprite->batch->matrix_stack));
    cmd->color = (color ? *color : white);
	cmd->sprite = sprite;
}
