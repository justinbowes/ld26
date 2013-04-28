//
//  xpl_frustum.h
//  p1
//
//  Created by Justin Bowes on 2013-02-20.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_frustum_h
#define p1_xpl_frustum_h

#include "xpl.h"
#include "xpl_vec.h"

typedef struct xpl_frustum {
    float fov;
    float aspect_ratio;
    float near_plane;
    float far_plane;
    
    union {
    	struct {
            // ordered by clip likelihood, assuming arbitrarily
            // large scene with aspect ratio > 1.0
    		xpl_plane cnear;
    		xpl_plane cfar;
    		xpl_plane cbottom;
    		xpl_plane ctop;
    		xpl_plane cleft;
    		xpl_plane cright;
    	};
    	xpl_plane p[6];
    } clip_planes;
    
    xmat4 view_matrix;
    xmat4 projection_matrix;
    xmat4 view_projection_matrix;
    
} xpl_frustum_t;

XPLINLINE void xpl_frustum_update_clip_planes(xpl_frustum_t *frustum) {
    float *m = frustum->projection_matrix.data;
    
    frustum->clip_planes.cleft      = xpl_plane_set_normalized( m[0] + m[3],  m[4] + m[7],  m[8] + m[11],  m[12] + m[15]);
	frustum->clip_planes.cright 	= xpl_plane_set_normalized(-m[0] + m[3], -m[4] + m[7], -m[8] + m[11], -m[12] + m[15]);
	frustum->clip_planes.cbottom	= xpl_plane_set_normalized( m[1] + m[3],  m[5] + m[7],  m[9] + m[11],  m[13] + m[15]);
	frustum->clip_planes.ctop       = xpl_plane_set_normalized(-m[1] + m[3], -m[5] + m[7], -m[9] + m[11], -m[13] + m[15]);
	frustum->clip_planes.cnear      = xpl_plane_set_normalized( m[2] + m[3],  m[6] + m[7],  m[10] + m[11],  m[14] + m[15]);
	frustum->clip_planes.cfar       = xpl_plane_set_normalized(-m[2] + m[3], -m[6] + m[7], -m[10] + m[11], -m[14] + m[15]);
}

XPLINLINE void xpl_frustum_update(xpl_frustum_t *frustum) {
    xmat4_perspective(frustum->fov, frustum->aspect_ratio,
                      frustum->near_plane, frustum->far_plane,
                      &frustum->projection_matrix);
    
    xpl_frustum_update_clip_planes(frustum);
}


#endif
