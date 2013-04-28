//
//  xpl_instanced_geom.h
//  p1
//
//  Created by Justin Bowes on 2013-01-02.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_instanced_geom_h
#define p1_xpl_instanced_geom_h

#include "xpl.h"

typedef struct xpl_instanced_geom {
	xpl_vao_t *vao;
	xpl_bo_t *vbo;
	xpl_bo_t *ibo;
	xpl_bo_t *ubo;
} xpl_instanced_geom_t;

xpl_instanced_geom_t *xpl_instanced_geom_new(void);
void xpl_instanced_geom_destroy(xpl_instanced_geom_t **ppgeom);

#endif
