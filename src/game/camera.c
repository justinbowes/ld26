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
	long x = center->px;
	long y = center->py;
	x += nudge_x;
	y += nudge_y;
	
	int halfwidth = camera.draw_area.width >> 1;
	int halfheight = camera.draw_area.height >> 1;
	
	long minx = x - halfwidth;
	if (minx < 0) x -= minx;
	
	long maxx = x + halfwidth;
	if (maxx > UINT32_MAX) y -= (maxx - UINT32_MAX);
	
	long miny = y - halfheight;
	if (miny < 0) y -= miny;
	
	long maxy = y + halfheight;
	if (maxy > UINT32_MAX) y -= (maxy - UINT32_MAX);
	
	camera.center.px = (int)x;
	camera.center.py = (int)y;
	
	camera.min.px = camera.center.px - halfwidth;
	camera.min.py = camera.center.py - halfheight;
	
	camera.max.px = camera.center.px + halfwidth;
	camera.max.py = camera.center.py + halfheight;
}
