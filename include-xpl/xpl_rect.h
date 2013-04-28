//
//  xpl_rect.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_rect_h
#define p1_xpl_rect_h

#include "xpl_vec2.h"

/**
 * xvec4 for rects is dangerous, because it's ambiguous whether we're using
 * x1->x2 or corner->width format.
 */
typedef union {
	float data[4];
	struct {
		float x;
		float y;
		float width;
		float height;
	};
    struct {
        xvec2 origin;
        xvec2 size;
    };
} xrect;

#include "xpl_vec.h"


XPLINLINE xrect xrect_set(float origin_x, float origin_y, float size_x, float size_y) {
	xrect r = {{ origin_x, origin_y, size_x, size_y }};
	return r;
}

XPLINLINE xrect xrect_scale(xrect rect, float scale) {
	xrect r = {{ rect.x * scale, rect.y * scale, rect.width * scale, rect.height * scale }};
	return r;
}

XPLINLINE xrect xrect_contract(xrect rect, int amount) {
	float width = rect.width - 2.f * amount;
	float height = rect.height - 2.f * amount;
	width = xmax(width, 0);
	height = xmax(height, 0);
	return xrect_set(rect.x + amount, rect.y + amount, width, height);
}

XPLINLINE xrect xrect_contract_to(xrect source, float width, float height) {
    xvec2 center = {{ source.width * 0.5f + source.x, source.height * 0.5f + source.y }};
    xrect r = {{ center.x - width * 0.5f, center.y - height * 0.5f, width, height }};
    return r;
}

XPLINLINE int xrect_in_bounds(xrect rect, xvec2 point) {
	return	point.x >= rect.origin.x &&
    point.y >= rect.origin.y &&
    point.x <= rect.origin.x + rect.size.width &&
    point.y <= rect.origin.y + rect.size.height;
}

XPLINLINE int xrect_in_boundsi(xrect rect, xivec2 point) {
	return	point.x >= rect.origin.x &&
	point.y >= rect.origin.y &&
	point.x <= rect.origin.x + rect.size.width &&
	point.y <= rect.origin.y + rect.size.height;
}

XPLINLINE xrect xrect_point_size(xvec2 point, xvec2 size) {
	return xrect_set(point.x, point.y, size.x, size.y);
}

XPLINLINE xrect xrect_union(xrect r1, xrect r2) {
	xvec2 p1 = {{ xmin(r1.x, r2.x), xmin(r1.y, r2.y) }};
	xvec2 p11 = {{ r1.x + r1.width, r1.y + r1.height }};
	xvec2 p21 = {{ r2.x + r2.width, r2.y + r2.height }};
	xvec2 p2 = {{ xmax(p11.x, p21.x), xmax(p11.y, p21.y) }};
	return xrect_point_size(p1, xvec2_sub(p2, p1));
}

#endif
