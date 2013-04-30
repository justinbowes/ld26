//
//  camera.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>

#include "game/camera.h"

camera_t camera;

void camera_calculate_center(position_t *center, int nudge_x, int nudge_y) {
	int64_t x = center->px;
	int64_t y = center->py;
	x += nudge_x;
	y += nudge_y;
	
	int64_t halfwidth = camera.draw_area.width >> 1;
	int64_t halfheight = camera.draw_area.height >> 1;
	
	int64_t minx = x - halfwidth;
	if (minx < 0) x -= minx;
	
	int64_t maxx = x + halfwidth;
	if (maxx > UINT32_MAX) y -= (maxx - UINT32_MAX);
	
	int64_t miny = y - halfheight;
	if (miny < 0) y -= miny;
	
	int64_t maxy = y + halfheight;
	if (maxy > UINT32_MAX) y -= (maxy - UINT32_MAX);
	
	camera.center.px = (uint32_t)x;
	camera.center.py = (uint32_t)y;
	
	camera.min.px = (uint32_t)((int64_t)camera.center.px - halfwidth);
	camera.min.py = (uint32_t)((int64_t)camera.center.py - halfheight);
	
	camera.max.px = (uint32_t)((int64_t)camera.center.px + halfwidth);
	camera.max.py = (uint32_t)((int64_t)camera.center.py + halfheight);

}
