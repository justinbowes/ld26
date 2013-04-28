//
//  window_params.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-01.
//  Copyright (c) 2012 Justin Bowes. All rights reserved.
//

#ifndef xpl_osx_window_params_h
#define xpl_osx_window_params_h

#include "xpl_platform.h"

// ------------- window ---------------------
#ifndef XPL_WINDOW_DEFAULT_WIDTH
#define XPL_WINDOW_DEFAULT_WIDTH        720
#endif
#ifndef XPL_WINDOW_DEFAULT_HEIGHT
#define XPL_WINDOW_DEFAULT_HEIGHT       480
#endif
#ifndef XPL_WINDOW_DEFAULT_COLOR
#define XPL_WINDOW_DEFAULT_COLOR        8
#endif
#ifndef XPL_WINDOW_DEFAULT_ALPHA
#define XPL_WINDOW_DEFAULT_ALPHA        8
#endif
#ifndef XPL_WINDOW_DEFAULT_Z
#define XPL_WINDOW_DEFAULT_Z            24
#endif
#ifndef XPL_WINDOW_DEFAULT_STENCIL
#define XPL_WINDOW_DEFAULT_STENCIL      0
#endif
#ifndef XPL_WINDOW_DEFAULT_FULLSCREEN
#define XPL_WINDOW_DEFAULT_FULLSCREEN   0
#endif
#ifndef XPL_WINDOW_DEFAULT_FRAMELIMIT
#define XPL_WINDOW_DEFAULT_FRAMELIMIT   0
#endif

typedef struct xpl_app_params {
    int width;
	int height;
	int color_bits;
	int alpha_bits;
	int z_bits;
	int stencil_bits;
	int is_fullscreen;
	int is_framelimit;
    
} xpl_app_params_t;

xpl_app_params_t xpl_app_params_load(int use_defaults);
void xpl_app_params_save(xpl_app_params_t params);

#endif
