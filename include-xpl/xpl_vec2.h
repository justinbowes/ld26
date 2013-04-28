//
//  xpl_vec2.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_vec2_h
#define p1_xpl_vec2_h

#include <stdbool.h>

#include "xpl_platform.h"

/**
 * Tuple of 2 floats
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b> or <b>s</b>
 *  - Second component: <b>y</b> or <b>t</b>
 */
typedef union {
	float data[2]; /**< All components at once     */
    
	struct {
		float x;   /**< Alias for first component  */
		float y;   /**< Alias for second component */
	};
    
	struct {
		float s;   /**< Alias for first component  */
		float t;   /**< Alias for second component */
	};
	
	struct {
		float width;
		float height;
	};
} xvec2;

XPLINLINE xvec2 xvec2_set(float x, float y) {
	xvec2 r = {{ x, y }};
	return r;
}

XPLINLINE xvec2 xvec2_all(float v) {
	xvec2 r = {{ v, v }};
	return r;
}

XPLINLINE xvec2 xvec2_add(xvec2 a, xvec2 b) {
	xvec2 r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	return r;
}

XPLINLINE xvec2 xvec2_mix(xvec2 a, xvec2 b, float f) {
    xvec2 r = {{
        (1.0f - f) * a.x + f * b.x,
        (1.0f - f) * a.y + f * b.y
    }};
    return r;
}

XPLINLINE xvec2 xvec2_sub(xvec2 a, xvec2 b) {
	xvec2 r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	return r;
}

XPLINLINE xvec2 xvec2_from_polar(float radius, float theta_rad) {
    xvec2 r = {{
        radius * cosf(theta_rad),
        radius * sinf(theta_rad)
    }};
    return r;
}

XPLINLINE float xvec2_length_sq(xvec2 v) {
	return v.x * v.x + v.y * v.y;
}

XPLINLINE float xvec2_length(xvec2 v) {
	return sqrtf(xvec2_length_sq(v));
}

XPLINLINE xvec2 xvec2_scale(xvec2 v, float s) {
	xvec2 r = v;
	r.x *= s;
	r.y *= s;
	return r;
}

XPLINLINE xvec2 xvec2_normalize(xvec2 v) {
	return xvec2_scale(v, 1.f / xvec2_length(v));
}

XPLINLINE xvec2 xvec2_rotate_rad(xvec2 v, float radians) {
	xvec2 r = v;
	r.x = cosf(radians);
	r.y = sinf(radians);
	return r;
}

XPLINLINE xvec2 xvec2_transform_xy(float x, float y, float scale_x, float scale_y, float rotate_radians) {
	xvec2 r;
	r.x = x * scale_x * cosf(rotate_radians);
	r.y = y * scale_y * sinf(rotate_radians);
	return r;
}

XPLINLINE bool xvec2_intersect_line_line(const xvec2 l1p1, const xvec2 l1p2, const xvec2 l2p1, const xvec2 l2p2, float *t) {
    xvec2 d1 = xvec2_sub(l1p2, l1p1);
    xvec2 d2 = xvec2_sub(l2p2, l2p1);
    
    float denom = d2.y * d1.x - d2.x * d1.y;
    if (denom == 0.0f) return false;
    
    if (t) {
        float dist = d2.x * (l1p1.y - l2p1.y) - d2.y * (l1p1.x - l2p1.x);
        dist /= denom;
        *t = dist;
    }
    return true;
}




#endif
