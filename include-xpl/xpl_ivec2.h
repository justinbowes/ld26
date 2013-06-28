//
//  xpl_ivec2.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_ivec2_h
#define p1_xpl_ivec2_h

#include "xpl.h"

/**
 * Tuple of 2 ints.
 *
 * Each field can be addressed using several aliases:
 *  - First component: <b>x</b>, <b>s</b> or <b>start</b>
 *  - Second component: <b>y</b>, <b>t</b> or <b>end</b>
 *
 */
typedef union _xiv2 {
	int data[2];    /**< All components at once     */
    
	struct {
		int x;      /**< Alias for first component  */
		int y;      /**< Alias for second component */
	};
    
	struct {
		int s;      /**< Alias for first component  */
		int t;      /**< Alias for second component */
	};
    
	struct {
		int start;  /**< Alias for first component  */
		int end;    /**< Alias for second component */
	};
	
	struct {
		int width;
		int height;
	};
} xivec2;

XPLINLINE xivec2 xivec2_set(int x, int y) {
	xivec2 r;
	r.x = x;
	r.y = y;
	return r;
}


XPLINLINE xivec2 xivec2_add(xivec2 a, xivec2 b) {
	xivec2 r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	return r;
}

XPLINLINE int xivec2_equal(xivec2 a, xivec2 b) {
    return ((a.x == b.x) &&
            (a.y == b.y));
}

XPLINLINE xivec2 xivec2_sub(xivec2 a, xivec2 b) {
	xivec2 r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	return r;
}


XPLINLINE xivec2 xivec2_copy(xivec2 v) {
	xivec2 r;
	r.x = v.x;
	r.y = v.y;
	return r;
}

XPLINLINE xivec2 xivec2_scale(xivec2 v, float s) {
	xivec2 r;
	r.x = v.x * s;
	r.y = v.y * s;
	return r;
}

XPLINLINE int xivec2_length_sq(xivec2 a, xivec2 b) {
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

#endif
