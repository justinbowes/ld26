//
//  window_params.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-01.
//  Copyright (c) 2012 Justin Bowes. All rights reserved.
//

#include <minIni.h>
#include "xpl.h"

#define APP_RESOURCE "app.ini"

#include "xpl_app_params.h"

xpl_app_params_t xpl_app_params_load(int use_defaults) {
    char filename[PATH_MAX] = { 0 };
    if (! use_defaults) {
        xpl_resolve_resource(filename, APP_RESOURCE, PATH_MAX);
    }

    xpl_app_params_t result;

    // Window dimensions
    result.width   = (int)ini_getl("Resolution", "width",
								   XPL_WINDOW_DEFAULT_WIDTH, filename);
    result.height  = (int)ini_getl("Resolution", "height",
								   XPL_WINDOW_DEFAULT_HEIGHT, filename);

    // Context format
    result.color_bits   = (int)ini_getl("Format", "colorBits",
		                                XPL_WINDOW_DEFAULT_COLOR, filename);
    result.alpha_bits   = (int)ini_getl("Format", "alphaBits",
										XPL_WINDOW_DEFAULT_ALPHA, filename);
    result.z_bits       = (int)ini_getl("Format", "zBits",
										XPL_WINDOW_DEFAULT_Z, filename);
    result.stencil_bits = (int)ini_getl("Format", "stencilBits",
										XPL_WINDOW_DEFAULT_STENCIL, filename);
    result.is_fullscreen = (int)ini_getbool("Format", "fullscreen",
											XPL_WINDOW_DEFAULT_FULLSCREEN, filename);

    result.is_framelimit = (int)ini_getbool("Performance", "frameLimiter",
											XPL_WINDOW_DEFAULT_FRAMELIMIT, filename);

    return result;
}

void xpl_app_params_save(xpl_app_params_t params) {
    char filename[PATH_MAX];
    xpl_resolve_resource(filename, APP_RESOURCE, PATH_MAX);
    
    ini_putl("Resolution", "width", params.width, filename);
    ini_putl("Resolution", "height", params.height, filename);
    
    ini_putl("Format", "colorBits", params.color_bits, filename);
    ini_putl("Format", "alphaBits", params.alpha_bits, filename);
    ini_putl("Format", "zBits", params.z_bits, filename);
    ini_putl("Format", "stencilBits", params.stencil_bits, filename);
    ini_puts("Format", "fullscreen", params.is_fullscreen ? "true" : "false", filename);
    
    ini_puts("Performance", "frameLimiter", params.is_framelimit ? "true" : "false", filename);
}