//
//  xpl_instanced_geom.c
//  p1
//
//  Created by Justin Bowes on 2013-01-02.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <assert.h>

#include "xpl_gl_debug.h"
#include "xpl_memory.h"
#include "xpl.h"
#include "xpl_vao.h"
#include "xpl_bo.h"

#include "xpl_instanced_geom.h"

xpl_instanced_geom_t *xpl_instanced_geom_new(void) {
	xpl_instanced_geom_t *geom = xpl_calloc_type(xpl_instanced_geom_t);
	
	geom->vao = xpl_vao_new();
	geom->vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	geom->ibo = xpl_bo_new(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
	geom->ubo = xpl_bo_new(GL_UNIFORM_BUFFER, GL_STATIC_READ);
	
	return geom;
}

void xpl_instanced_geom_destroy(xpl_instanced_geom_t **ppgeom) {
	assert(ppgeom);
	xpl_instanced_geom_t *geom = *ppgeom;
	assert(geom);
	xpl_vao_destroy(&geom->vao);
	xpl_bo_destroy(&geom->vbo);
	xpl_bo_destroy(&geom->ibo);
	xpl_bo_destroy(&geom->ubo);
	
	xpl_free(geom);
	*ppgeom = NULL;
}