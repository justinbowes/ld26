//
//  xpl_font_manager.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-23.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_font_manager_h
#define xpl_osx_xpl_font_manager_h

#include "xpl_texture_atlas.h"
#include "xpl_markup.h"
#include "xpl_font.h"

typedef struct {
	xpl_texture_atlas_t				*atlas;
	struct fm_font_entry			*font_cache;
	wchar_t							*wchar_cache;
} xpl_font_manager_t;


xpl_font_manager_t *xpl_font_manager_new(size_t surface_width, size_t surface_height, size_t depth_bytes);

void xpl_font_manager_destroy(xpl_font_manager_t **ppmgr);

void xpl_font_manager_delete_font(xpl_font_manager_t *self, xpl_font_t **ppfont);

xpl_font_t *xpl_font_manager_get_from_resource_name(xpl_font_manager_t *self, const char *resource_name, const float size);
xpl_font_t *xpl_font_manager_get_from_description(xpl_font_manager_t *self, const char *family, const float size, const int bold, const int italic);
xpl_font_t *xpl_font_manager_get_from_markup(xpl_font_manager_t *self, const xpl_markup_t *markup);

#endif
