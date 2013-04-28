//
//  xpl_window.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-23.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_window_h
#define xpl_osx_xpl_window_h

#include "xpl.h"
#include "xpl_app_params.h"
#include "xpl_engine_info.h"
#include "xpl_app.h"

struct xpl_app;

typedef void (*xpl_init_func)(struct xpl_app *app);
typedef void (*xpl_destroy_func)(struct xpl_app *app);
typedef void (*xpl_main_loop_func)(struct xpl_app *app);

// ---- app descriptor

typedef struct xpl_app {

	const char                      *title;
    
    bool                            restart;
    bool                            did_restart;
	bool							allow_resize;
    
	xpl_init_func					init_func;
	xpl_destroy_func				destroy_func;
	xpl_main_loop_func              main_loop_func;
	void                            *callback_userdata;
    
    xpl_engine_info_t               *engine_info;
    xpl_engine_execution_info_t     *execution_info;
    
    xpl_app_params_t                display_params;
    
    int argc;
    char** argv;
    
} xpl_app_t;

xpl_app_t *xpl_app_new(int argc, char *argv[]);
void xpl_app_destroy(xpl_app_t **app);

int xpl_start_app(xpl_app_t *app);

#endif
