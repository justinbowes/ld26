//
//  xpl_texture.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-10-07.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_texture_h
#define xpl_osx_xpl_texture_h

#include "xpl_gl.h"
#include "xpl_vec.h"

typedef struct xpl_texture {
	xivec2					size;
	int						channels;
	GLuint                  texture_id;

} xpl_texture_t;

typedef struct xpl_texture_region {
	xirect					region_i;
	xrect					region_f;
	GLuint					texture_id;
} xpl_texture_region_t;

xpl_texture_t *xpl_texture_new(void);
void xpl_texture_destroy(xpl_texture_t **pptexture);

GLuint xpl_texture_load(xpl_texture_t *self, const char *resource_name);
// Loads a null-terminated list of texture resource names into a GL_TEXTURE_2D_ARRAY.
GLuint xpl_texture_load_array(xpl_texture_t *self, ...);

// Create a texture region from a texture. If the region parameter is null, the region
// will be the entire area of the texture.
xpl_texture_region_t *xpl_texture_region_new(xpl_texture_t *texture, xirect *region);
void xpl_texture_region_destroy(xpl_texture_region_t **ppregion);

#endif
