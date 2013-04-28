//
//  xpl_gl_debug.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-29.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include "xpl_log.h"
#include "xpl_gl_debug.h"

#define ECASE(x) case(x): return #x

#ifdef DEBUG_GL

void xpl_gl_breakpoint_func() {
    LOG_DEBUG("Set a breakpoint here to pause on GL errors");
}

void xpl_gl_log_bound_textures() {
    GLint active_texture_unit;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &active_texture_unit);
    
    GLint num_texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_texture_units);
    
    for (int i = 0; i < num_texture_units; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        GLint t1d, t1d_array, t2d, t2d_array, t2d_multisample, t2d_multisample_array, t3d, tbuffer, tcube, tcube_array, trectangle;
        glGetIntegerv(GL_TEXTURE_BINDING_1D, &t1d);
        glGetIntegerv(GL_TEXTURE_BINDING_1D_ARRAY, &t1d_array);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &t2d);
        glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &t2d_array);
        glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &t2d_multisample);
        glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, &t2d_multisample_array);
        glGetIntegerv(GL_TEXTURE_BINDING_3D, &t3d);
        glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &tbuffer);
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &tcube);
        glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP_ARRAY, &tcube_array);
        glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &trectangle);
        LOG_DEBUG("Bound texture: GL_TEXTURE%d: 1D %d\t 1DA %d\t 2D %d\t 2DA %d\t 2DM %d\t 2DMA %d\t 3D %d\t B %d\t C %d\t CA %d\t R %d",
                  i, t1d, t1d_array, t2d, t2d_array, t2d_multisample, t2d_multisample_array, t3d, tbuffer, tcube, tcube_array, trectangle);
    }
    
    glActiveTexture(active_texture_unit);
}

const char *xpl_gl_get_error_string(GLenum e) {
    switch (e) {
            ECASE(GL_INVALID_ENUM);
            ECASE(GL_INVALID_VALUE);
            ECASE(GL_INVALID_OPERATION);
            ECASE(GL_INVALID_INDEX);
#ifdef GL_STACK_OVERFLOW
            ECASE(GL_STACK_OVERFLOW);
#endif
#ifdef GL_STACK_UNDERFLOW
            ECASE(GL_STACK_UNDERFLOW);
#endif
            ECASE(GL_OUT_OF_MEMORY);
            ECASE(GL_INVALID_FRAMEBUFFER_OPERATION);
#ifdef GL_TABLE_TOO_LARGE
            ECASE(GL_TABLE_TOO_LARGE)
#endif
		default: return "XPL_GL_UNKNOWN";
    }
}

XPLINLINE const char *source_type(GLenum type) {
    switch (type) {
            ECASE(GL_DEBUG_SOURCE_API_ARB);
            ECASE(GL_DEBUG_SOURCE_APPLICATION_ARB);
            ECASE(GL_DEBUG_SOURCE_OTHER_ARB);
            ECASE(GL_DEBUG_SOURCE_SHADER_COMPILER_ARB);
            ECASE(GL_DEBUG_SOURCE_THIRD_PARTY_ARB);
            ECASE(GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
            default: return "GL_DEBUG_SOURCE_UNKNOWN_ARB";
    }
}

XPLINLINE const char *type_type(GLenum type) {
    switch (type) {
            ECASE(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
            ECASE(GL_DEBUG_TYPE_ERROR_ARB);
            ECASE(GL_DEBUG_TYPE_OTHER_ARB);
            ECASE(GL_DEBUG_TYPE_PERFORMANCE_ARB);
            ECASE(GL_DEBUG_TYPE_PORTABILITY_ARB);
            ECASE(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
            default: return "GL_DEBUG_TYPE_UNKNOWN_ARB";
    }
}

XPLINLINE const char *severity_type(GLenum type) {
    switch (type) {
            ECASE(GL_DEBUG_SEVERITY_LOW_ARB);
            ECASE(GL_DEBUG_SEVERITY_MEDIUM_ARB);
            ECASE(GL_DEBUG_SEVERITY_HIGH_ARB);
            default: return "GL_DEBUG_SEVERITY_UNKNOWN_ARB";
    }
}

static APIENTRY void xpl_gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, void *user_param) {
    const char *source_str = source_type(source);
    const char *type_str = type_type(type);
    const char *severity_str = severity_type(severity);
    switch (severity) {
        case GL_DEBUG_SEVERITY_LOW_ARB:
            LOG_DEBUG("%s %s %d %s %s", source_str, type_str, id, severity_str, message);
            break;
            
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            LOG_WARN("%s %s %d %s %s", source_str, type_str, id, severity_str, message);
            break;
            
        case GL_DEBUG_SEVERITY_HIGH_ARB:
            LOG_ERROR("%s %s %d %s %s", source_str, type_str, id, severity_str, message);
            break;
            
        default:
            LOG_WARN("%s %s %d %s %s", source_str, type_str, id, severity_str, message);
            break;
    }
}

void xpl_gl_debug_install() {
    if (! glDebugMessageControlARB) {
        LOG_WARN("ARB_debug_output not present, can't install callback");
        return;
    }
    
    glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM_ARB, 0, NULL, GL_TRUE);
    glDebugMessageCallbackARB(xpl_gl_debug_callback, NULL);
    GL_DEBUG();
}

#endif
