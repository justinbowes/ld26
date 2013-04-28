//
//  xpl_vec3.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_vec3_h
#define p1_xpl_vec3_h

#include <stdbool.h>

#include "xpl.h"
#include "xpl_vec2.h"
#include "xpl_ivec2.h"

/**
 * Tuple of 3 floats
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b>, <b>r</b> or <b>red</b>
 *  - Second component: <b>y</b>, <b>g</b> or <b>green</b>
 *  - Third component:  <b>z</b>, <b>b</b> or <b>blue</b>
 */
typedef union _xv3 {
	float data[3];   /**< All components at once    */
    
	struct {
		float x;     /**< Alias for first component */
		float y;     /**< Alias for second component */
		float z;     /**< Alias for third component  */
	};
    
	struct {
		float s;      /**< Alias for first component */
		float t;      /**< Alias for second component */
		float p;      /**< Alias for third component  */
	};
    
	struct {
		float r;     /**< Alias for first component */
		float g;     /**< Alias for second component */
		float b;     /**< Alias for third component  */
	};
    
	struct {
		float red;   /**< Alias for first component */
		float green; /**< Alias for second component */
		float blue; /**< Alias for third component  */
	};
    
    struct {
        xvec2 xy;
        float _z;
    };
    
    struct {
        float _x;
        xvec2 yz;
    };
} xvec3;

static const xvec3 xvec3_x_axis = {{ 1.0f, 0.0f, 0.0f }};
static const xvec3 xvec3_y_axis = {{ 0.0f, 1.0f, 0.0f }};
static const xvec3 xvec3_z_axis = {{ 0.0f, 0.0f, 1.0f }};
static const xvec3 xvec3_zero   = {{ 0.0f, 0.0f, 0.0f }};

XPLINLINE xvec3 xvec3_set(float x, float y, float z) {
	xvec3 r = {{ x, y, z }};
	return r;
}

XPLINLINE xvec3 xvec3_all(float v) {
    return xvec3_set(v, v, v);
}

XPLINLINE xvec3 xvec3_add(xvec3 a, xvec3 b) {
    return xvec3_set(a.x + b.x, a.y + b.y, a.z + b.z);
}

XPLINLINE xvec3 xvec3_sub(xvec3 a, xvec3 b) {
	xvec3 r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	r.z = a.z - b.z;
	return r;
}


XPLINLINE xvec3 xvec3_copy(xvec3 v) {
	return xvec3_set(v.x, v.y, v.z);
}

XPLINLINE xvec3 xvec3_cross(xvec3 a, xvec3 b) {
    xvec3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.x * b.z - a.z * b.x;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

XPLINLINE float xvec3_dot(xvec3 a, xvec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

XPLINLINE int xvec3_equal(xvec3 a, xvec3 b, float epsilon) {
    return fabsf(a.x - b.x) < epsilon &&
    fabsf(a.y - b.y) < epsilon &&
    fabsf(a.z - b.z) < epsilon;
}

XPLINLINE xvec3 xvec3_extend(xvec2 v, float z) {
	return xvec3_set(v.x, v.y, z);
}

XPLINLINE xvec3 xvec3_extendi(xivec2 v, float z) {
	return xvec3_set((float)v.x, (float)v.y, z);
}

XPLINLINE float xvec3_length_sq(xvec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

XPLINLINE float xvec3_length(xvec3 v) {
	return sqrtf(xvec3_length_sq(v));
}

XPLINLINE xvec3 xvec3_abs(xvec3 v) {
    xvec3 r = {{ fabsf(v.x), fabsf(v.y), fabsf(v.z) }};
    return r;
}

XPLINLINE xvec3 xvec3_absmin(xvec3 a, xvec3 b) {
    xvec3 abs_a = xvec3_abs(a);
    xvec3 abs_b = xvec3_abs(b);
    xvec3 r = {{
        abs_a.x < abs_b.x ? a.x : b.x,
        abs_a.y < abs_b.y ? a.y : b.y,
        abs_a.z < abs_b.z ? a.z : b.z
    }};
    return r;
}


XPLINLINE xvec3 xvec3_absmax(xvec3 a, xvec3 b) {
    xvec3 abs_a = xvec3_abs(a);
    xvec3 abs_b = xvec3_abs(b);
    xvec3 r = {{
        abs_a.x > abs_b.x ? a.x : b.x,
        abs_a.y > abs_b.y ? a.y : b.y,
        abs_a.z > abs_b.z ? a.z : b.z
    }};
    return r;
}

XPLINLINE float xvec3_max_component(xvec3 v) {
    return xmax(v.x, xmax(v.y, v.z));
}

XPLINLINE float xvec3_min_component(xvec3 v) {
    return xmin(v.x, xmin(v.y, v.z));
}

XPLINLINE xvec3 xvec3_mul(xvec3 a, xvec3 b) {
    xvec3 r = {{
        a.x * b.x,
        a.y * b.y,
        a.z * b.z
    }};
    return r;
}

XPLINLINE xvec3 xvec3_scale(xvec3 v, float s) {
	xvec3 r = v;
	r.x *= s;
	r.y *= s;
	r.z *= s;
	return r;
}

XPLINLINE xvec3 xvec3_mix(xvec3 a, xvec3 b, float mix) {
	return xvec3_add(xvec3_scale(a, 1.0f - mix),
                     xvec3_scale(b, mix));
}

XPLINLINE xvec3 xvec3_normalize(xvec3 v) {
	return xvec3_scale(v, 1.0 / xvec3_length(v));
}

XPLINLINE float xvec3_distance_sq_to_line(const xvec3 point, const xvec3 line_p0, const xvec3 line_p1, xvec3 *point_on_line_out) {
    xvec3 v = xvec3_sub(point, line_p0);
    xvec3 s = xvec3_sub(line_p1, line_p0);
    float s_len_sq = xvec3_length_sq(s);
    float dot = xvec3_dot(v, s) / s_len_sq;
    xvec3 disp = xvec3_scale(s, dot);
    
    if (point_on_line_out) *point_on_line_out = xvec3_add(line_p0, disp);
    
    xvec3 r = xvec3_sub(v, disp);
    return xvec3_length_sq(r);
}

XPLINLINE float xvec3_distance_to_line(const xvec3 point, const xvec3 line_p0, const xvec3 line_p1, xvec3 *point_on_line_out) {
    return sqrtf(xvec3_distance_sq_to_line(point, line_p0, line_p1, point_on_line_out));
}

XPLINLINE float xvec3_point_inside_triangle(const xvec3 triangle[3], const xvec3 point) {
    xvec3 u = xvec3_sub(triangle[1], triangle[0]);
    xvec3 v = xvec3_sub(triangle[2], triangle[0]);
    xvec3 w = xvec3_sub(point, triangle[0]);
    
    float uu = xvec3_dot(u, u);
    float uv = xvec3_dot(u, v);
    float vv = xvec3_dot(v, v);
    float wu = xvec3_dot(w, u);
    float wv = xvec3_dot(w, v);
    float d = uv * uv - uu * vv;
    
    float inv_d = 1.0f / d;
    float s = (uv * wv - vv * wu) * inv_d;
    if (s < 0 || s > 1) return false;
    
    float t = (uv * wu - uu * wv) * inv_d;
    if (t < 0 || (s + t) > 1) return false;
    
    return true;
}

XPLINLINE float xvec3_point_inside_polygon(const xvec3 *vertices, size_t vertex_count, const xvec3 point) {
    size_t triangle_count = vertex_count - 2;
    for (size_t i = 0; i < triangle_count; ++i) {
        if (xvec3_point_inside_triangle(&vertices[i], point)) return true;
    }
    return false;
}

static char cbuf[128];
XPLINLINE const char * xvec3_str(xvec3 *vec) {
	snprintf(cbuf, 128, "[%f, %f, %f]", vec->data[0], vec->data[1], vec->data[2]);
    return cbuf;
}

#endif

