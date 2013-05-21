//
//  window.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-01.
//  Copyright (c) 2012 Justin Bowes. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include "xpl.h"

#include "xpl_gl_debug.h"
#include "xpl_texture.h"

#include "xpl_app.h"
#include "xpl_rc.h"
#include "xpl_l10n.h"
#include "xpl_app_params.h"

#ifdef XPL_PLATFORM_OSX
#define GL_ARB_shader_objects
#include <OpenGL/CGLTypes.h>
#include <OpenGL/OpenGL.h>

static void osx_force_discrete_gpu() {
	LOG_WARN("Forcing discrete GPU switch");
	static CGLPixelFormatObj format = NULL;
	if (format) return;
	
	CGLPixelFormatAttribute attribs[1];
	attribs[0] = (CGLPixelFormatAttribute)(0);
	GLint num_pixel_formats = 0;
	CGLChoosePixelFormat(attribs, &format, &num_pixel_formats);
	// format is deliberately leaked a la Chrome
}

static void set_platform_hints() {
    // Create a forward-compatible OpenGL 3.2 context.
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

#elif !defined(XPL_PLATFORM_IOS)
static void set_platform_hints() {
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}
#endif

#if !defined(XPL_PLATFORM_IOS)
void GLFWCALL window_resized(int width, int height) {
    LOG_DEBUG("Window resized to %d x %d", width, height);
	glViewport(0, 0, width, height);
}

static void set_common_hints(xpl_app_t *app) {
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, ! app->allow_resize);
	if (app->allow_resize) {
		glfwSetWindowSizeCallback(window_resized);
	}
#	ifdef GL_DEBUG
    glfwOpenWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#	endif
}
#endif

int xpl_start_app(xpl_app_t *app) {

    int defaults = FALSE;

#if !defined(XPL_PLATFORM_WINDOWS)
    // won't work
    // http://msdn.microsoft.com/en-CA/library/x99tb11d(v=vs.80).aspx
    // need to use MultiByteToWideChar
    setlocale(LC_CTYPE, "UTF-8");
#endif
    
#if defined(XPL_PLATFORM_OSX)
	// Do this before we grab a real context.
	osx_force_discrete_gpu();
#endif

#ifndef XPL_PLATFORM_IOS
    if (glfwInit() != GL_TRUE) {
        LOG_ERROR("Couldn't initialize GLFW");
        exit(XPL_RC_GLFW_INIT_FAILED);
    }
#endif
	
    do {
        if (app->restart) {
            app->restart = false;
            app->did_restart = true;
        }

#ifndef XPL_PLATFORM_IOS
        LOG_DEBUG("Loading %sapp parameters", defaults ? "default " : "");
        xpl_app_params_t display_params = xpl_app_params_load(defaults);

        set_platform_hints();
        set_common_hints(app);

        if (! glfwOpenWindow(display_params.width, display_params.height,
                             display_params.color_bits, display_params.color_bits, display_params.color_bits,
                             display_params.alpha_bits, display_params.z_bits, display_params.stencil_bits,
                             display_params.is_fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)) {

        	LOG_DEBUG("Failed opening window: %d x %d, R%dG%dB%dA%dD%dS%d, %s",
        			display_params.width, display_params.height,
        			display_params.color_bits, display_params.color_bits, display_params.color_bits, display_params.alpha_bits,
        			display_params.z_bits, display_params.stencil_bits, (display_params.is_fullscreen ? "fullscreen" : "windowed"));

        	if (! defaults) {
                // fall back to defaults
                defaults = TRUE;
                continue;

            } else {
                // Fell back to defaults; didn't help
                LOG_ERROR("Couldn't open window with fallback params");
                exit(XPL_RC_GLFW_OPEN_WINDOW_FAILED);
            }

        }

		if (gl3wInit()) {
            LOG_ERROR("Couldn't initialize gl3w");
            exit(XPL_RC_GL3W_INIT_FAILED);
        }
        
        xpl_gl_debug_install(); // No-op if not debug
		
        glfwSetWindowTitle(app->title);
        glfwSetWindowSizeCallback(&window_resized);

        int gl_major = 0, gl_minor = 0, gl_rev;
		int width = 0, height = 0;
		int red_bits = glfwGetWindowParam(GLFW_RED_BITS),
			green_bits = glfwGetWindowParam(GLFW_GREEN_BITS),
			blue_bits = glfwGetWindowParam(GLFW_BLUE_BITS),
			color_bits = red_bits + green_bits + blue_bits;
        glfwGetGLVersion(&gl_major, &gl_minor, &gl_rev);
		glfwGetWindowSize(&width, &height);
        LOG_INFO("Starting up %s with OpenGL %d.%d.%d", app->title, gl_major, gl_minor, gl_rev);
        LOG_INFO("Request/Provided:");
        LOG_INFO("          width: %d	%d", display_params.width, width);
        LOG_INFO("         height: %d	%d", display_params.height, height);
        LOG_INFO("     color bits: %d	%d", display_params.color_bits * 3, color_bits);
		LOG_INFO("     alpha bits: %d	%d", display_params.alpha_bits, glfwGetWindowParam(GLFW_ALPHA_BITS));
		LOG_INFO("         z bits: %d	%d", display_params.z_bits, glfwGetWindowParam(GLFW_DEPTH_BITS));
        LOG_INFO("   stencil bits: %d	%d", display_params.stencil_bits, glfwGetWindowParam(GLFW_STENCIL_BITS));
        LOG_INFO("     fullscreen: %s", display_params.is_fullscreen ? "true" : "false");

        int accelerated = glfwGetWindowParam(GLFW_ACCELERATED);
        if (accelerated == GL_FALSE) {
            LOG_ERROR("Couldn't create a hardware accelerated context");
            exit(XPL_RC_GL_NO_HWACCEL);
        }

		glViewport(0, 0, display_params.width, display_params.height);
		GL_DEBUG();

        glfwSwapInterval(display_params.is_framelimit ? 1 : 0);
        GL_DEBUG();

        app->display_params = display_params;
#endif
        
        xpl_l10n_set_fallback_locale("en");
        xpl_l10n_load_saved_locale();

    	app->execution_info = xpl_engine_execution_info_new();
#ifndef XPL_PLATFORM_IOS
        app->execution_info->screen_size = xivec2_set(width, height);
#endif
        app->engine_info = xpl_engine_info_new();

        app->init_func(app);
        app->main_loop_func(app);
        app->destroy_func(app);

        xpl_engine_info_destroy(&app->engine_info);

        LOG_INFO("Main loop shut down.");

#ifndef XPL_PLATFORM_IOS
        glfwCloseWindow();
        
        if ( ! app->restart) {
            glfwTerminate();
        }
#endif

        xpl_engine_execution_info_destroy(&app->execution_info);

    } while (defaults || app->restart);

    xpl_app_destroy(&app);
    
    return 0;
}

// --- xpl_app_info

static char *unset_title = "Unnamed XPL Application";

xpl_app_t *xpl_app_new(int argc, char *argv[]) {
	xpl_app_t *app_info = xpl_calloc_type(xpl_app_t);
	app_info->title = unset_title;
    app_info->argc = argc;
    app_info->argv = argv;
	setlocale(LC_CTYPE, "UTF-8");
	return app_info;
}

void xpl_app_destroy(xpl_app_t **app_info) {
	xpl_free(*app_info);
	*app_info = NULL;
}
