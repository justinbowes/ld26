//
//  xpl_gl_debug.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-29.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_gl_debug_h
#define xpl_osx_xpl_gl_debug_h

#include "xpl.h"
#include "xpl_gl.h"

const char *xpl_gl_get_error_string(GLenum e);

#ifdef DEBUG
#define DEBUG_GL
#endif

#ifdef DEBUG_GL
/** DEBUG macros. **/

#include "xpl_log.h"

#define GL_DEBUG() \
    { \
        GLenum e; \
        while ((e = glGetError()) != GL_NO_ERROR) { \
            LOG_ERROR("GL_DEBUG error: 0x%x %s", e, xpl_gl_get_error_string(e)); \
            xpl_gl_breakpoint_func(); \
        } \
    }

#define GL_DEBUG_THIS(x) \
    x; \
    GL_DEBUG();

#define GL_CLEAR_ERROR() \
	{ \
		GLenum e; \
		while ((e = glGetError()) != GL_NO_ERROR) { \
			LOG_INFO("Clearing error: 0x%x %s", e, xpl_gl_get_error_string(e)); \
		} \
	}


// end debug macros

void xpl_gl_breakpoint_func(void);
void xpl_gl_debug_install(void);
void xpl_gl_log_bound_textures(void);

#else
/** Non GL_DEBUG macros. Should be no-ops. */

#define GL_DEBUG_THIS(x) x
#define GL_DEBUG()
#define GL_CLEAR_ERROR()
#define xpl_gl_breakpoint_func()
#define xpl_gl_debug_install()
#define xpl_gl_log_bound_textures()

// end release macros
#endif

#endif
