//
//  xpl_ivec3.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_ivec3_h
#define p1_xpl_ivec3_h

#include "xpl_ivec2.h"

/**
 * Tuple of 3 ints.
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b>, <b>r</b> or <b>red</b>
 *  - Second component: <b>y</b>, <b>g</b> or <b>green</b>
 *  - Third component:  <b>z</b>, <b>b</b> or <b>blue</b>
 *
 */
typedef union _xiv3 {
	int data[3];    /**< All components at once     */
    
	struct {
		int x;      /**< Alias for first component  */
		int y;      /**< Alias for second component */
		int z;      /**< Alias for third component  */
	};
    
	struct {
		int s;      /**< Alias for first component  */
		int t;      /**< Alias for second component */
		int p;      /**< Alias for third component  */
	};
    
	struct {
		int r;      /**< Alias for first component  */
		int g;      /**< Alias for second component */
		int b;      /**< Alias for third component  */
	};
    
	struct {
		int red;    /**< Alias for first component  */
		int green;  /**< Alias for second component */
		int blue;   /**< Alias for third component  */
	};
    
    struct {
        xivec2 ivec2;
        int _z;
    };
    
} xivec3;

#include "xpl_vec.h"

XPLINLINE xivec3 xivec3_set(int x, int y, int z) {
	xivec3 r;
	r.x = x;
	r.y = y;
	r.z = z;
	return r;
}

XPLINLINE xivec3 xivec3_add(xivec3 a, xivec3 b) {
	xivec3 r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;
	return r;
}

XPLINLINE xivec3 xivec3_sub(xivec3 a, xivec3 b) {
	xivec3 r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	r.z = a.z - b.z;
	return r;
}


XPLINLINE xivec3 xivec3_copy(xivec3 v) {
	xivec3 r;
	r.x = v.x;
	r.y = v.y;
	r.z = v.z;
	return r;
}

XPLINLINE xivec3 xivec3_extend(xivec2 v, int z) {
	xivec3 r;
	r.x = v.x;
	r.y = v.y;
	r.z = z;
	return r;
}

XPLINLINE xivec3 xivec3_scale(xivec3 v, int s) {
	xivec3 r;
	r.x *= s;
	r.y *= s;
	r.z *= s;
	return r;
}


XPLINLINE char * xivec3_str(xivec3 *vec, char *buffer, size_t buffer_size) {
	snprintf(buffer, buffer_size, "[%d, %d, %d]", vec->data[0], vec->data[1], vec->data[2]);
    return buffer;
}

#endif
