//
//  xpl_irect.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_irect_h
#define p1_xpl_irect_h

#include "xpl_ivec2.h"

typedef union {
	int data[4];
	struct {
		int x;
		int y;
		int width;
		int height;
	};
	struct {
		xivec2 origin;
		xivec2 size;
	};
} xirect;

#include "xpl_vec.h"

XPLINLINE xirect xirect_set(int origin_x, int origin_y, int width, int height) {
	xirect r = {{ origin_x, origin_y, width, height }};
	return r;
}

XPLINLINE xirect xirect_contract(xirect rect, int amount) {
	int width = rect.width - 2 * amount;
	int height = rect.height - 2 * amount;
	width = xmax(width, 0);
	height = xmax(height, 0);
	return xirect_set(rect.x + amount, rect.y + amount, width, height);
}


#endif
