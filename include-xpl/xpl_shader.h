//
//  xpl_shader.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-06.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_shader_h
#define xpl_osx_shader_h

#include "xpl_gl.h"
#include "xpl_vao_shader.h"

// Use xpl_shader_get and check program_handle
// xpl_shader_t *xpl_shader_new(const char *name);
GLuint xpl_shader_add(xpl_shader_t *shader, const GLenum shader_type, const char *effect_key);
GLuint xpl_shader_link(xpl_shader_t *shader);
GLint xpl_shader_get_uniform(xpl_shader_t *shader, const char *uniform_name);
GLint xpl_shader_get_va(xpl_shader_t *shader, xpl_vao_t *vao, const char *va_name);
void xpl_shader_bind_frag_data_location(xpl_shader_t *shader, GLuint color_number, const char *name);


xpl_shader_t *xpl_shader_get(const char *name);
xpl_shader_t *xpl_shader_get_prepared(const char *name, const char *vs_name, const char *fs_name);
void xpl_shader_release(xpl_shader_t **ppshader);

int xpl_shaders_init(const char *path_prefix, const char *path_suffix);
void xpl_shaders_add_directive(const char *define);
void xpl_shaders_shutdown(void);


#endif
