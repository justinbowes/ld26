//
//  xpl_bo.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-09.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_bo_h
#define xpl_osx_xpl_bo_h

#include "xpl.h"
#include "xpl_gl.h"
#include "xpl_dynamic_buffer.h"

enum xpl_bo_dirty_state {

	xpl_bods_empty         = 0,
	xpl_bods_create        = 1,
	xpl_bods_clean         = 2,
	xpl_bods_update        = 3,
} ;

typedef struct xpl_bo {

	GLuint                      bo_id;
	GLenum                      target;
	GLenum                      usage;

	xpl_dynamic_buffer_t        *client_data;
	size_t						server_memory_size;
	enum xpl_bo_dirty_state    dirty_state;
} xpl_bo_t;

xpl_bo_t *xpl_bo_new(GLenum target, GLenum usage);
xpl_bo_t *xpl_bo_clone(const xpl_bo_t *source_bo);
void xpl_bo_destroy(xpl_bo_t **ppbo);

void xpl_bo_commit(xpl_bo_t *self);

void xpl_bo_alloc(xpl_bo_t *self, size_t data_len_bytes);
void xpl_bo_append(xpl_bo_t *self, const void *data, size_t data_len_bytes);
void xpl_bo_clear(xpl_bo_t *self);
void xpl_bo_insert(xpl_bo_t *self, size_t insert_offset, const void *data, size_t data_len_bytes);
void xpl_bo_delete(xpl_bo_t *self, size_t delete_offset, size_t data_len_bytes);
void xpl_bo_update(xpl_bo_t *self, size_t update_offset, const void *data, size_t data_len_bytes);

#endif
