//
//  xpl_ivec4.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_ivec4_h
#define p1_xpl_ivec4_h

#include "xpl_ivec2.h"
#include "xpl_ivec3.h"

/**
 * Tuple of 4 ints.
 *
 * Each field can be addressed using several aliases:
 *  - First component:  <b>x</b>, <b>r</b>, <b>red</b> or <b>vstart</b>
 *  - Second component: <b>y</b>, <b>g</b>, <b>green</b> or <b>vcount</b>
 *  - Third component:  <b>z</b>, <b>b</b>, <b>blue</b>, <b>width</b> or <b>istart</b>
 *  - Fourth component: <b>w</b>, <b>a</b>, <b>alpha</b>, <b>height</b> or <b>icount</b>
 *
 */
typedef union {
	int data[4];    /**< All components at once     */
    
	struct {
		int x;      /**< Alias for first component  */
		int y;      /**< Alias for second component */
		int z;      /**< Alias for third component  */
		int w;      /**< Alias for fourth component */
	};
    
	struct {
		int s;      /**< Alias for first component  */
		int t;      /**< Alias for second component */
		int p;      /**< Alias for third component  */
		int q;      /**< Alias for fourth component */
	};
    
	struct {
		int r;      /**< Alias for first component  */
		int g;      /**< Alias for second component */
		int b;      /**< Alias for third component  */
		int a;      /**< Alias for fourth component */
	};
    
	struct {
		int red;    /**< Alias for first component  */
		int green;  /**< Alias for second component */
		int blue;   /**< Alias for third component  */
		int alpha;  /**< Alias for fourth component */
	};
    
	struct {
		int vstart; /**< Alias for first component  */
		int vcount; /**< Alias for second component */
		int istart; /**< Alias for third component  */
		int icount; /**< Alias for fourth component */
	};
    
    struct {
        xivec3 ivec3;
        int _w;
    };
    
    struct {
        xivec2 ivec2;
        int _z;
    };
    
    struct {
        xivec2 start;
        xivec2 end;
    };
    
} xivec4;

#include "xpl_vec.h"

XPLINLINE xivec4 xivec4_set(int x, int y, int z, int w) {
	xivec4 r;
	r.x = x;
	r.y = y;
	r.z = z;
	r.w = w;
	return r;
}

XPLINLINE xivec4 xivec4_add(xivec4 a, xivec4 b) {
	xivec4 r;
	r.x = a.x + b.x;
	r.y = a.y + b.y;
	r.z = a.z + b.z;
	r.w = a.w + b.z;
	return r;
}

XPLINLINE xivec4 xivec4_sub(xivec4 a, xivec4 b) {
	xivec4 r;
	r.x = a.x - b.x;
	r.y = a.y - b.y;
	r.z = a.z - b.z;
	r.w = a.w - b.z;
	return r;
}

XPLINLINE xivec4 xivec4_copy(xivec4 v) {
	xivec4 r;
	r.x = v.x;
	r.y = v.y;
	r.z = v.z;
	r.w = v.w;
	return r;
}

XPLINLINE xivec4 xivec4_extend(xivec3 v, int w) {
	xivec4 r;
	r.x = v.x;
	r.y = v.y;
	r.z = v.z;
	return r;
}

XPLINLINE xivec4 xivec4_scale(xivec4 v, int s) {
	xivec4 r;
	r.x *= s;
	r.y *= s;
	r.z *= s;
	r.w *= s;
	return r;
}



#endif
