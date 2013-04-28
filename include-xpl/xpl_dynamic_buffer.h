//
//  xpl_dynamic_buffer.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-09.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_dynamic_buffer_h
#define xpl_osx_xpl_dynamic_buffer_h

#include <stdlib.h>

typedef struct xpl_dynamic_buffer {

	size_t      capacity;
	unsigned char *content;
	size_t		length;

	size_t      dirty_range_min;
	size_t      dirty_range_max;

} xpl_dynamic_buffer_t;


xpl_dynamic_buffer_t *xpl_dynamic_buffer_new(void);
void xpl_dynamic_buffer_destroy(xpl_dynamic_buffer_t **ppbuffer);

void xpl_dynamic_buffer_alloc(xpl_dynamic_buffer_t *self, size_t alloc_len, int as_data);
void xpl_dynamic_buffer_append(xpl_dynamic_buffer_t *self, const unsigned char *data, size_t data_len);
void xpl_dynamic_buffer_insert(xpl_dynamic_buffer_t *self, size_t insert_offset, const unsigned char *data, size_t data_len);
void xpl_dynamic_buffer_delete(xpl_dynamic_buffer_t *self, size_t delete_offset, size_t data_len);
void xpl_dynamic_buffer_update(xpl_dynamic_buffer_t *self, size_t update_offset, const unsigned char *data, size_t data_len);

void xpl_dynamic_buffer_clear(xpl_dynamic_buffer_t *self);

int xpl_dynamic_buffer_is_clean(xpl_dynamic_buffer_t *self);
void xpl_dynamic_buffer_mark_clean(xpl_dynamic_buffer_t *self);

#endif
