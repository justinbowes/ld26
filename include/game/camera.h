//
//  camera.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_camera_h
#define ld26_camera_h

#include "xpl_irect.h"
#include "xpl_vec2.h"

#include "game/game.h"

typedef struct camera {
	position_t	center;
	position_t	min;
	position_t	max;
	
	xirect		draw_area;
	xirect		dc;
} camera_t;

extern camera_t camera;

void camera_calculate_center(position_t *center, int nudge_x, int nudge_y);

XPLINLINE xvec2 camera_get_draw_position(position_t position) {
	int64_t lx = (int64_t)position.px - (int64_t)camera.min.px + (int64_t)camera.draw_area.x;
	int64_t ly = (int64_t)position.py - (int64_t)camera.min.py + (int64_t)camera.draw_area.y;
	xvec2 v = {{ (float)lx, (float)ly }};
	return v;
}

#endif
