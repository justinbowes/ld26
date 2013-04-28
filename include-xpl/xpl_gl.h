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

#include "GL3/gl3w.h"
#include "GL3/gl3.h"
#include "GL/glfw.h"
#include "xpl_gl_debug.h"

#endif
