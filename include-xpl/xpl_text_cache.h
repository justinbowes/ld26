//
//  xpl_font_cache.h
//  p1
//
//  Created by Justin Bowes on 2013-03-18.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_font_cache_h
#define p1_xpl_font_cache_h

#include "xpl_text_buffer.h"
#include "xpl_font.h"
#include "xpl_markup.h"

typedef struct xpl_text_cache xpl_text_cache_t;

typedef struct xpl_cached_text {
    xpl_text_buffer_t *buffer;
	xpl_font_t *managed_font;
} xpl_cached_text_t;

struct xpl_text_cache * xpl_text_cache_new(size_t size);
void xpl_text_cache_destroy(struct xpl_text_cache **ppcache);

void xpl_text_cache_advance_frame(struct xpl_text_cache *self);
xpl_cached_text_t * xpl_text_cache_get(struct xpl_text_cache *self, xpl_markup_t *markup, const char *text);

#endif
