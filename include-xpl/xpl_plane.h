//
//  xpl_plane.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_plane_h
#define p1_xpl_plane_h

#include <stdbool.h>

#include "xpl_platform.h"
#include "xpl_vec3.h"

typedef union _xplane {
    
	struct { // Planar
		float A;
		float B;
		float C;
		float D;
	};
    
    struct {
        xvec3 ABC;
        float _D;
    };
    
} xpl_plane;

XPLINLINE xpl_plane xpl_plane_normalized_points(const xvec3 p0, const xvec3 p1, const xvec3 p2) {
    xvec3 v0 = xvec3_sub(p0, p1);
    xvec3 v1 = xvec3_sub(p2, p1);
    xvec3 n = xvec3_normalize(xvec3_cross(v1, v0));
    
    xpl_plane r = {{
        n.x,
        n.y,
        n.z,
        -xvec3_dot(p0, n)
    }};
    return r;
}

XPLINLINE xpl_plane xpl_plane_normalized_point_normal(const xvec3 p, const xvec3 n) {
    xvec3 nn = xvec3_normalize(n);
    xpl_plane r = {{
        nn.x,
        nn.y,
        nn.z,
        -xvec3_dot(p, nn)
    }};
    return r;
}

XPLINLINE xpl_plane xpl_plane_set_normalized(const float A, const float B, const float C, const float D) {
    float nf = 1.0f / sqrtf(A * A + B * B + C * C + D * D);
    xpl_plane r = {{
        nf * A,
        nf * B,
        nf * C,
        nf * D
    }};
    return r;
}

XPLINLINE float xpl_plane_dot(const xpl_plane plane, const xvec3 point) {
    return xvec3_dot(plane.ABC, point);
}

XPLINLINE float xpl_plane_dist(const xpl_plane plane, const xvec3 point) {
    return xpl_plane_dot(plane, point) + plane.D;
}

XPLINLINE xvec3 xpl_plane_reflect(const xpl_plane plane, const xvec3 incident) {
    float d = xpl_plane_dist(plane, incident);
    xvec3 r = {{
        (incident.x + 2.0f) * -plane.A * d,
        (incident.y + 2.0f) * -plane.B * d,
        (incident.z + 2.0f) * -plane.C * d
    }};
    return r;
}

XPLINLINE xvec3 xpl_plane_project(const xpl_plane plane, const xvec3 point) {
    float h = xpl_plane_dist(plane, point);
    xvec3 r = {{
        point.x - plane.A * h,
        point.y - plane.B * h,
        point.z - plane.C * h
    }};
    return r;
}

XPLINLINE bool xpl_plane_point_is_on(const xpl_plane plane, const xvec3 point, const float epsilon) {
    float d = xpl_plane_dist(plane, point);
    return (d < epsilon && d > -epsilon);
}

XPLINLINE bool xpl_plane_intersect_with_line(const xpl_plane plane, const xvec3 p0, const xvec3 p1, float *t) {
    xvec3 dir = xvec3_sub(p1, p0);
    float div = xpl_plane_dot(plane, dir);
    
    if (div == 0) return false;
    
    *t = -xpl_plane_dist(plane, p0) / div;
    return true;
}

#endif
