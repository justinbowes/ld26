//
//  xpl_geometry.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-14.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_geometry_h
#define xpl_osx_xpl_geometry_h

#include "uthash.h"
#include "xpl_vao.h"
#include "xpl_bo.h"

typedef struct xpl_geometry {

	int             instance_id;
	int             refcount;
	xpl_vao_t       *vao;
	xpl_bo_t        *vertices;

	UT_hash_handle  hh;
} xpl_geometry_t;

typedef struct xpl_geom_slab_info {

	float           width;
	float           height;
	float           depth;
} xpl_geom_slab_info_t;

typedef struct xpl_geom_ellipsoid_info {

	float           rx;
	float           ry;
	float           rz;
	int             lat_slices;
	int             long_slices;
} xpl_geom_ellipsoid_info_t;

typedef struct xpl_geom_plane_info {

	float           sx;
	float           sz;
} xpl_geom_plane_info_t;


xpl_geometry_t *xpl_geometry_get_slab(xpl_geom_slab_info_t *slab_info);
xpl_geometry_t *xpl_geometry_get_ellipsoid(xpl_geom_ellipsoid_info_t *ellipsoid_info);
xpl_geometry_t *xpl_geometry_get_plane(xpl_geom_plane_info_t *plane_info);

xpl_geometry_t *xpl_geometry_fork(const xpl_geometry_t *geometry);

void xpl_geometry_release(xpl_geometry_t **geometry);

#endif
