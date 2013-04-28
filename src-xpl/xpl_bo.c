//
//  xpl_bo.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-09.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <assert.h>

#include "xpl_gl_debug.h"
#include "xpl_log.h"
#include "xpl_memory.h"

#include "xpl_bo.h"

xpl_bo_t *xpl_bo_new(GLenum target, GLenum usage) {
    xpl_bo_t *bo = xpl_calloc_type(xpl_bo_t);

    bo->bo_id = 0;
	bo->client_data = xpl_dynamic_buffer_new();
    bo->dirty_state = xpl_bods_empty;
    bo->usage = usage;
    bo->target = target;

    return bo;
}

xpl_bo_t *xpl_bo_clone(const xpl_bo_t *source_bo) {
    xpl_bo_t *new_bo = xpl_bo_new(source_bo->target, source_bo->usage);
    xpl_dynamic_buffer_append(new_bo->client_data, source_bo->client_data->content, source_bo->client_data->length);
    xpl_bo_commit(new_bo);

    return new_bo;
}

void xpl_bo_destroy(xpl_bo_t **ppbo) {
    xpl_bo_t *bo = *ppbo;

    if (bo->client_data) {
        xpl_dynamic_buffer_destroy(&bo->client_data);
    }

    if (bo->bo_id) {
        glDeleteBuffers(1, &bo->bo_id);
    }

    xpl_free(bo);
    *ppbo = NULL;
}

void xpl_bo_commit(xpl_bo_t *self) {

    if (! self->bo_id) {
        glGenBuffers(1, &self->bo_id);
    }

    switch (self->dirty_state) {
        case xpl_bods_empty:
            LOG_TRACE("Not committing unused buffer");
            break;

        case xpl_bods_create:
            glBindBuffer(self->target, self->bo_id);
            glBufferData(self->target, self->client_data->length, self->client_data->content, self->usage);
            GL_DEBUG();
            break;

        case xpl_bods_clean:
            LOG_TRACE("Buffer is clean; no update needed");
            break;

        case xpl_bods_update:
            glBindBuffer(self->target, self->bo_id);
            glBufferSubData(self->target,
                            self->client_data->dirty_range_min,
                            self->client_data->dirty_range_max - self->client_data->dirty_range_min,
                            self->client_data->content);
            GL_DEBUG();
            break;

        default:
            LOG_ERROR("Invalid buffer state");
            break;
    }

    GL_DEBUG();
    
    glBindBuffer(self->target, GL_NONE);
    
    self->dirty_state = xpl_bods_clean;
    xpl_dynamic_buffer_mark_clean(self->client_data);
}

static void buffer_mark_dirty(xpl_bo_t *self) {
    if (self->dirty_state == xpl_bods_empty) {
        // empty goes to create
        self->dirty_state = xpl_bods_create;
    } else if (self->dirty_state == xpl_bods_clean) {
        // clean goes to update
        self->dirty_state = xpl_bods_update;
    } // otherwise, it's already dirty
}

void xpl_bo_alloc(xpl_bo_t *self, size_t data_len) {
	buffer_mark_dirty(self);
	xpl_dynamic_buffer_alloc(self->client_data, data_len, TRUE);
}

void xpl_bo_append(xpl_bo_t *self, const void *data, size_t data_len) {
	assert(self->client_data);
    // Can't resize FBO
    self->dirty_state = xpl_bods_create;
    xpl_dynamic_buffer_append(self->client_data, data, data_len);
}

void xpl_bo_insert(xpl_bo_t *self, size_t insert_offset, const void *data, size_t data_len) {
	assert(self->client_data);
    buffer_mark_dirty(self);
    xpl_dynamic_buffer_insert(self->client_data, insert_offset, data, data_len);
}

void xpl_bo_clear(xpl_bo_t *self) {
    xpl_dynamic_buffer_clear(self->client_data);
    self->dirty_state = xpl_bods_create;
}

void xpl_bo_delete(xpl_bo_t *self, size_t delete_offset, size_t data_len) {
    // Can't resize FBO
    self->dirty_state = xpl_bods_create;
    xpl_dynamic_buffer_delete(self->client_data, delete_offset, data_len);
}

void xpl_bo_update(xpl_bo_t *self, size_t update_offset, const void *data, size_t data_len) {
    buffer_mark_dirty(self);
	xpl_dynamic_buffer_update(self->client_data, update_offset, data, data_len);
}
