//
//  xpl_dynamic_buffer.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-09.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <string.h>
#include <assert.h>

#include "xpl.h"
#include "xpl_memory.h"
#include "xpl_dynamic_buffer.h"

xpl_dynamic_buffer_t *xpl_dynamic_buffer_new(void) {
    xpl_dynamic_buffer_t *buf = xpl_calloc_type(xpl_dynamic_buffer_t);
    return buf;
}

void xpl_dynamic_buffer_destroy(xpl_dynamic_buffer_t **ppbuffer) {
    xpl_dynamic_buffer_t *buf = *ppbuffer;
    
    if (buf->capacity) {
        xpl_free(buf->content);
    }
    
    xpl_free(buf);
    *ppbuffer = NULL;
}

void xpl_dynamic_buffer_alloc(xpl_dynamic_buffer_t *self, size_t data_len, int as_data) {
    assert(self);
    
    if (!data_len) return;
	
    if (self->content) {
        self->content = xpl_realloc(self->content, self->capacity + data_len);
    } else {
        self->content = xpl_alloc(data_len);
    }
    memset(self->content + self->capacity, 0, data_len);
    
    self->capacity += data_len;
    if (as_data) {
    	self->length += data_len;
    	self->dirty_range_max = xmax(self->dirty_range_max, self->length);
    }
}

void xpl_dynamic_buffer_append(xpl_dynamic_buffer_t *self, const unsigned char *data, size_t data_len) {
    
    assert(self);
    
    if (! data_len) return;
    
    if (self->length + data_len > self->capacity) {
    	xpl_dynamic_buffer_alloc(self, self->length + data_len - self->capacity, FALSE);
    }
    memmove(self->content + self->length,
            data,
            data_len);
    
    self->dirty_range_min = xmin(self->dirty_range_min, self->length);
    self->length += data_len;
    self->dirty_range_max = xmax(self->dirty_range_max, self->length);
}

void xpl_dynamic_buffer_insert(xpl_dynamic_buffer_t *self, size_t insert_offset, const unsigned char *data, size_t data_len) {
    
    assert(self);
    assert(insert_offset <= self->length);
    
    if (! data_len) return;
    
    // Use append for insert at end or empty buffer
    if (insert_offset == self->length) {
        xpl_dynamic_buffer_append(self, data, data_len);
        return;
    }
    
    // Otherwise, it's a three stage copy
    size_t newlen = self->length + data_len;
    if (newlen > self->capacity) {
    	self->content = xpl_realloc(self->content, newlen);
    }
    
    // Now we have the original copy with room for the insertion
    // Move the final chunk forward by data_len
    memmove(self->content + insert_offset + data_len,
            self->content + insert_offset,
            self->length - insert_offset);
    
    // Now move the data to the insert offset
    memmove(self->content + insert_offset,
            data,
            data_len);
    
    self->dirty_range_min = xmin(self->dirty_range_min, insert_offset);
    self->length = newlen;
    self->dirty_range_max = xmax(self->dirty_range_max, self->length);

}

void xpl_dynamic_buffer_delete(xpl_dynamic_buffer_t *self, size_t delete_offset, size_t data_len) {
    
    assert(self);
    assert(delete_offset + data_len <= self->capacity);
    
    if (! data_len) return;
    
    memmove(self->content, self->content, delete_offset);
    memmove(self->content + delete_offset,
            self->content + delete_offset + data_len,
            self->length - data_len);
    
    self->dirty_range_max = xmax(self->dirty_range_max, self->length);
    self->length -= data_len;
    self->dirty_range_min = xmin(self->dirty_range_min, delete_offset);
    
}

void xpl_dynamic_buffer_update(xpl_dynamic_buffer_t *self, size_t update_offset, const unsigned char *data, size_t data_len) {
    assert(self);
    assert(update_offset + data_len <= self->length);

    if (! data_len) return;
    
    memmove(self->content + update_offset, data, data_len);
    
    self->dirty_range_min = xmin(self->dirty_range_min, update_offset);
    self->dirty_range_max = xmax(self->dirty_range_max, update_offset + data_len);
}

void xpl_dynamic_buffer_clear(xpl_dynamic_buffer_t *self) {
	self->length = 0;
	xpl_dynamic_buffer_mark_clean(self);
}

int xpl_dynamic_buffer_is_clean(xpl_dynamic_buffer_t *self) {
    return (self->dirty_range_max == 0);
}

void xpl_dynamic_buffer_mark_clean(xpl_dynamic_buffer_t *self) {
    self->dirty_range_min = self->length;
    self->dirty_range_max = 0;
}
