//
//  xpl_texture_atlas.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-23.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_texture_atlas_h
#define xpl_osx_xpl_texture_atlas_h

#include "xpl.h"
#include "xpl_gl.h"

#include "utlist.h"

#include "xpl_dynamic_buffer.h"
#include "xpl_vec.h"

typedef struct xpl_texture_atlas_node {

	int                             left;
	int                             top;
	int                             size;

	// UTList ptrs
	struct xpl_texture_atlas_node   *prev;
	struct xpl_texture_atlas_node   *next;

} xpl_texture_atlas_node_t;

typedef struct xpl_texture_atlas {

	xpl_texture_atlas_node_t    *nodes;     // utlist
	int                         node_count;

	int							width;
	int							height;
#ifndef XPL_PLATFORM_IOS
	int							depth;      // buffer depth in bytes (4 = 32 bit)
#endif

	int							used;       // surface size used

	GLuint                      texture_id; // GL object id

	xpl_dynamic_buffer_t        *data;

} xpl_texture_atlas_t;

xpl_texture_atlas_t *xpl_texture_atlas_new(const int width, const int height, const int depth);
void xpl_texture_atlas_destroy(xpl_texture_atlas_t **ppatlas);

// Assemble texture atlas from multiple bitmaps.
void xpl_texture_atlas_commit(xpl_texture_atlas_t *self);
void xpl_texture_atlas_clear(xpl_texture_atlas_t *self);
xirect xpl_texture_atlas_get_region(xpl_texture_atlas_t *self, const int width, const int height);
void xpl_texture_atlas_set_region(xpl_texture_atlas_t *self, xirect region, const unsigned char *data, const int stride);

#endif
