//
//  xpl_gl.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-29.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_gl_h
#define xpl_osx_xpl_gl_h

// ------------- OpenGL -------------------
// Include this file first. gl3w macroizes
// calls to opengl and is sensitive to include
// order.
// ----------------------------------------

// Need to set winver before GL headers are included
#include "xpl_platform.h"

#if defined(XPL_PLATFORM_IOS)
#	define XPL_GLES
#	import <OpenGLES/ES2/gl.h>
#	import <OpenGLES/ES2/glext.h>
#	define glBindFragDataLocation	glBindFragDataLocationOES
#	define glBindVertexArray		glBindVertexArrayOES
#	define glDeleteVertexArrays		glDeleteVertexArraysOES
#	define glDrawArraysInstanced	glDrawArraysInstancedOES
#	define glGenVertexArrays		glGenVertexArraysOES
#else
#	if defined(XPL_PLATFORM_OSX)
#		include <OpenGL/gl3.h>
#		include <OpenGL/gl3ext.h>
#		define GLFW_INCLUDE_GL3
#		define GLFW_NO_GLU
#	else
#		include "GL3/gl3w.h"
#		include "GL3/gl3.h"
#	endif
#	include "GL/glfw.h"
#endif
#include "xpl_gl_debug.h"

#if defined(GL_RED)
#	define XPL_GL_SINGLE_CHANNEL GL_RED
#elif defined(GL_ALPHA)
#	define XPL_GL_SINGLE_CHANNEL GL_ALPHA
#else
#	error "Couldn't define single channel"
#endif

#endif
