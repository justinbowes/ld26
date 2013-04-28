//
//  xpl_vec4.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_vec4_h
#define p1_xpl_vec4_h

#include "xpl_vec2.h"
#include "xpl_vec3.h"

/**
 * Tuple of 4 floats.
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b>, <b>r</b> or <b>red</b>
 *  - Second component: <b>y</b>, <b>g</b> or <b>green</b>
 *  - Third component:  <b>z</b>, <b>b</b>, <b>blue</b> or <b>width</b>
 *  - Fourth component: <b>w</b>, <b>a</b>, <b>alpha</b> or <b>height</b>
 */
typedef union {
	float data[4];    /**< All components at once    */
    
	struct {
		float x;      /**< Alias for first component */
		float y;      /**< Alias for second component */
		float z;      /**< Alias for third component  */
		float w;      /**< Alias for fourth component */
	};
    
	struct {
		float s;      /**< Alias for first component */
		float t;      /**< Alias for second component */
		float p;      /**< Alias for third component  */
		float q;      /**< Alias for fourth component */
	};
    
	struct {
		float r;      /**< Alias for first component */
		float g;      /**< Alias for second component */
		float b;      /**< Alias for third component  */
		float a;      /**< Alias for fourth component */
	};
    
	struct {
		float red;    /**< Alias for first component */
		float green;  /**< Alias for second component */
		float blue;   /**< Alias for third component  */
		float alpha;  /**< Alias for fourth component */
	};
    
    struct {
        xvec3 xyz;
        float _w;
    };
    
    struct {
        xvec3 rgb;
        float _a;
    };
    
    struct {
		xvec2 origin;
        xvec2 dest;
	};
    
    struct {
        xvec2 xy;
        xvec2 zw;
    };
    
    struct {
        float _x;
        xvec2 yz;
        float __w;
    };
    
} xvec4;

XPLINLINE xvec4 xvec4_set(float x, float y, float z, float w) {
	xvec4 r;
	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;
	return r;
}

XPLINLINE xvec4 xvec4_all(float v) {
    return xvec4_set(v, v, v, v);
}

XPLINLINE xvec4 xvec4_add(xvec4 a, xvec4 b) {
    return xvec4_set(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

XPLINLINE xvec4 xvec4_sub(xvec4 a, xvec4 b) {
	xvec4 r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	r.z = a.z - b.z;
	r.w = a.w - b.z;
	return r;
}

XPLINLINE xvec4 xvec4_multiply(xvec4 a, xvec4 b) {
	return xvec4_set(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

XPLINLINE xvec4 xvec4_copy(xvec4 v) {
	return xvec4_set(v.x, v.y, v.z, v.w);
}

XPLINLINE int xvec4_equal(xvec4 a, xvec4 b, float epsilon) {
    return fabsf(a.x - b.x) < epsilon &&
    fabsf(a.y - b.y) < epsilon &&
    fabsf(a.z - b.z) < epsilon &&
    fabsf(a.w - b.w) < epsilon;
}

XPLINLINE xvec4 xvec4_extend(xvec3 v, float w) {
	return xvec4_set(v.x, v.y, v.z, w);
}

XPLINLINE xvec4 xvec4_scale(xvec4 v, float s) {
	xvec4 r = v;
	r.x *= s;
	r.y *= s;
	r.z *= s;
	r.w *= s;
	return r;
}

XPLINLINE float xvec4_length_sq(xvec4 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

XPLINLINE float xvec4_length(xvec4 v) {
	return sqrtf(xvec4_length_sq(v));
}

XPLINLINE xvec4 xvec4_normalize(xvec4 v) {
	float s = 1.0 / xvec4_length(v);
	return xvec4_scale(v, s);
}

XPLINLINE xvec4 xvec4_mix(xvec4 a, xvec4 b, float mix) {
    return xvec4_add(xvec4_scale(a, 1.0f - mix),
                     xvec4_scale(b, mix));
}

XPLINLINE xvec3 xvec4_homogeneous_to_xvec3(xvec4 i) {
    return xvec3_scale(i.xyz, 1.0 / i.w);
}


XPLINLINE char * xvec4_str(xvec4 *vec, char *buffer, size_t buffer_size) {
	snprintf(buffer, buffer_size, "[%f, %f, %f, %f]", vec->data[0], vec->data[1], vec->data[2], vec->data[3]);
    return buffer;
}


#endif
