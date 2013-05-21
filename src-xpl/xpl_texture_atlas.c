//
//  xpl_texture_atlas.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-23.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <limits.h>

#include "xpl.h"
#include "xpl_gl.h"
#include "xpl_math.h"
#include "xpl_memory.h"
#include "xpl_vec.h"
#include "xpl_texture_atlas.h"
#include "SOIL.h"

static void texture_atlas_add_initial_node(xpl_texture_atlas_t *self) {
	// Put a one-px border around the whole atlas to prevent sampling artifacts.
	xpl_texture_atlas_node_t *node = xpl_alloc_type(xpl_texture_atlas_node_t);
	node->left = 1;
	node->top = 1;
	node->size = (int)self->width - 2;
	node->prev = NULL;
	node->next = NULL;
	DL_APPEND(self->nodes, node);
	self->node_count++;
}

static void texture_atlas_create_initial_buffer(xpl_texture_atlas_t *self) {
	xpl_dynamic_buffer_t *buffer = xpl_dynamic_buffer_new();
#ifdef XPL_PLATFORM_IOS
	int depth = 1;
#else
	int depth = self->depth;
#endif
	xpl_dynamic_buffer_alloc(buffer, self->width * self->height * depth * sizeof (unsigned char), TRUE);
	self->data = buffer;
}

static xpl_texture_atlas_node_t *texture_atlas_get_node_at_index(const xpl_texture_atlas_t *self, const int index) {
	xpl_texture_atlas_node_t *node = self->nodes;
	for (int j = 0; j < index; j++) {
		assert(node);
		node = node->next;
	}
	return node;
}

static int texture_atlas_fit(xpl_texture_atlas_t *self, const int index, const int width, const int height) {
	assert(self);

	xpl_texture_atlas_node_t *node = texture_atlas_get_node_at_index(self, index);
	if (! node) {
		LOG_ERROR("No node found at index %i", index);
		return -1;
	}

	int x = node->left;
	int y = node->top;
	int width_left = (int)width;
	int i = index;

	if ((x + width) > (self->width - 1)) {
		return -1; // Too wide to fit in the atlas
	}

	while (width_left > 0) {
		node = texture_atlas_get_node_at_index(self, i);
		y = xmax(y, node->top);
		if ((y + height) > (self->height - 1)) {
			// Too tall and out of room
			return -1;
		}

		width_left -= node->size;
		++i;
		assert(i < self->node_count || width_left <= 0);
	}
	return y;
}

static void texture_atlas_merge_nodes(xpl_texture_atlas_t *self) {
	assert(self);

	xpl_texture_atlas_node_t *node = self->nodes;
	xpl_texture_atlas_node_t *next;
	while (node->next) {
		next = node->next;
		if (node->top == next->top) {
			node->size += next->size;
			DL_DELETE(self->nodes, next);
			xpl_free(next);
			self->node_count--;
		} else {
			node = node->next;
		}
	}
}

xpl_texture_atlas_t *xpl_texture_atlas_new(const int width, const int height, const int depth) {
	assert((depth == 1) ||
		   (depth == 3) ||
		   (depth == 4)
		   );

	xpl_texture_atlas_t *self = xpl_alloc_type(xpl_texture_atlas_t);

	self->nodes         = NULL;
	self->node_count    = 0;
	self->used          = 0;
	self->width         = width;
	self->height        = height;
#ifndef XPL_PLATFORM_IOS
	self->depth         = depth;
#endif
	self->texture_id    = 0;

	texture_atlas_add_initial_node(self);
	texture_atlas_create_initial_buffer(self);

	if (! self->data->capacity) {
		LOG_ERROR("Dynamic buffer length is not correct: %ud", (unsigned int)self->data->capacity);
		return NULL;
	}

	return self;
}

void xpl_texture_atlas_destroy(xpl_texture_atlas_t **ppatlas) {
	xpl_texture_atlas_t *atlas = *ppatlas;
	assert(atlas);

	xpl_texture_atlas_node_t *node, *tmp;

	DL_FOREACH_SAFE(atlas->nodes, node, tmp) {
		DL_DELETE(atlas->nodes, node);
		xpl_free(node);
	}

	if (atlas->data) {
		xpl_dynamic_buffer_destroy(&atlas->data);
	}

	if (atlas->texture_id) {
		glDeleteTextures(1, &atlas->texture_id);
	}

	xpl_free(atlas);
	*ppatlas = NULL;
}

void xpl_texture_atlas_commit(xpl_texture_atlas_t *self) {
	assert(self);
	assert(self->data);

	// If we have a texture ID and the buffer is clean, it's up to date.
	if (self->texture_id && xpl_dynamic_buffer_is_clean(self->data)) {
		LOG_TRACE("Texture atlas commit ignored; buffer clean and texture ID is assigned");
		return;
	}

	if (! self->texture_id) {
		glGenTextures(1, &self->texture_id);
	}

	glBindTexture(GL_TEXTURE_2D, self->texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // @todo: customize?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#ifndef XPL_PLATFORM_IOS
	switch (self->depth) {
		case 4:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                         (GLsizei)self->width, (GLsizei)self->height, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, self->data->content);
			break;

		case 3:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                         (GLsizei)self->width, (GLsizei)self->height, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, self->data->content);
			break;

		case 1:
			glTexImage2D(GL_TEXTURE_2D, 0, XPL_GL_SINGLE_CHANNEL,
                         (GLsizei)self->width, (GLsizei)self->height, 0, XPL_GL_SINGLE_CHANNEL,
                         GL_UNSIGNED_BYTE, self->data->content);
			break;

		default:
			break;
	}
#else
	glTexImage2D(GL_TEXTURE_2D, 0, XPL_GL_SINGLE_CHANNEL,
				 (GLsizei)self->width, (GLsizei)self->height, 0, XPL_GL_SINGLE_CHANNEL,
				 GL_UNSIGNED_BYTE, self->data->content);
#endif
    
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    
    GL_DEBUG();

	xpl_dynamic_buffer_mark_clean(self->data);
}

xirect xpl_texture_atlas_get_region(xpl_texture_atlas_t *self, const int width, const int height) {
	assert(self);

	xirect region = {
		{ 0, 0, (int)width, (int)height }};

	xpl_texture_atlas_node_t *best_node, *candidate_node;
	int best_width = INT_MAX;
	int best_height = INT_MAX;
	int best_index = -1;

	for (int i = 0; i < self->node_count; ++i) {

		int y = texture_atlas_fit(self, i, width, height);
		if (y < 0) continue;

		candidate_node = texture_atlas_get_node_at_index(self, i);
		if (((y + height) < best_height) ||
			(((y + height) == best_height) && (candidate_node->size < best_width))) {
			best_index = i;
			best_node = candidate_node;
			best_height = y + (int)height;
			best_width = candidate_node->size;
			region.origin.x = candidate_node->left;
			region.origin.y = y;
		}
	}

	if (best_index == -1) {
		region.origin.x = -1;
		region.origin.y = -1;
		region.size.width = 0;
		region.size.height = 0;
		return region;
	}

	xpl_texture_atlas_node_t *new_node = xpl_alloc_type(xpl_texture_atlas_node_t);
	new_node->left = region.origin.x;
	new_node->top = region.origin.y + (int)height;
	new_node->size = (int)width;
	new_node->prev = NULL;
	new_node->next = NULL;

	// Splice the node into the list BEFORE best_node.
	DL_PREPEND_ELEM(self->nodes, best_node, new_node);
	self->node_count++;

	xpl_texture_atlas_node_t *tmp;

	DL_FOREACH_SAFE(new_node->next, candidate_node, tmp) {
		if (candidate_node->left < (candidate_node->prev->left + candidate_node->prev->size)) {
			int shrink = candidate_node->prev->left + candidate_node->prev->size - candidate_node->left;
			candidate_node->left += shrink;
			candidate_node->size -= shrink;

			if (candidate_node->size <= 0) {
				DL_DELETE(self->nodes, candidate_node);
				self->node_count--;
				xpl_texture_atlas_node_t *prev = candidate_node->prev;
				xpl_free(candidate_node);
				candidate_node = prev;
			}
		}
	}

	texture_atlas_merge_nodes(self);
	self->used += width * height;
	return region;
}

void xpl_texture_atlas_set_region(xpl_texture_atlas_t *self, xirect region, const unsigned char *data, const int stride) {
	assert(self);
	assert(region.x >= 0);
	assert(region.y >= 0);
	assert(region.x < (self->width - 1));
	assert(region.y < (self->height - 1));
	assert((region.x + region.width) <= (self->width - 1));
	assert((region.y + region.height) <= (self->height - 1));

#ifndef XPL_PLATFORM_IOS
	int adepth = self->depth;
#else
	int adepth = 1;
#endif
	int awidth = self->width;

	int swidth = region.width;
	int sheight = region.height;
	int sx = region.x;
	int sy = region.y;
	int charsize = sizeof (unsigned char);

	// http://code.google.com/p/freetype-gl/source/browse/trunk/texture-atlas.c line 126
	for (int i = 0; i < sheight; ++i) {
		xpl_dynamic_buffer_update(self->data, ((sy + i) * awidth + sx) * charsize * adepth, (void *)(data + (i * stride * charsize)), adepth * swidth * charsize);
	}
}

void xpl_texture_atlas_clear(xpl_texture_atlas_t *self) {
	assert(self);
	assert(self->data);

	while (self->nodes) {
		xpl_free(self->nodes);
		DL_DELETE(self->nodes, self->nodes);
	}
	self->node_count = 0;

	self->used = 0;

	xpl_dynamic_buffer_destroy(&self->data);

	texture_atlas_add_initial_node(self);
	texture_atlas_create_initial_buffer(self);
}

