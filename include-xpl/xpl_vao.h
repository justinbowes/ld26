//
//  xpl_vao.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-09.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_vao_h
#define xpl_osx_xpl_vao_h

#include "uthash.h"

#include "xpl.h"
#include "xpl_bo.h"
#include "xpl_vao_shader.h"

xpl_vao_t *xpl_vao_new(void);
xpl_vao_t *xpl_vao_clone(const xpl_vao_t *vao);
void xpl_vao_destroy(xpl_vao_t **ppvao);

void xpl_vao_define_vertex_attrib(xpl_vao_t *vao, const char *name, xpl_bo_t *vbo, GLint size, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset_bytes);
void xpl_vao_define_vertex_attrib_xvec2(xpl_vao_t *vao, const char *name, xpl_bo_t *vbo, GLsizei stride, GLsizei offset);
void xpl_vao_define_vertex_attrib_xvec3(xpl_vao_t *vao, const char *name, xpl_bo_t *vbo, GLsizei stride, GLsizei offset);
void xpl_vao_define_vertex_attrib_xvec4(xpl_vao_t *vao, const char *name, xpl_bo_t *vbo, GLsizei stride, GLsizei offset);
void xpl_vao_enable_vertex_attrib(xpl_vao_t *vao, const char *name);
void xpl_vao_disable_vertex_attrib(xpl_vao_t *vao, const char *name);

// Gets the vertex buffer to make modifications. Buffer info changes appear to be OK to make at any time.
xpl_vertex_attrib_t *xpl_vao_get_vertex_attrib(xpl_vao_t *vao, const char *name);

// Sets a new index buffer at the given IBO index. If this buffer replaces an old one, the old one is returned.
// The buffer is assumed to contain unsigned shorts.
xpl_bo_t *xpl_vao_set_index_buffer(xpl_vao_t *vao, int buffer_index, xpl_bo_t *ibo);

#include "xpl_shader.h"

// Draw unindexed elements.
void xpl_vao_program_draw_arrays(xpl_vao_t *vao, xpl_shader_t *shader, GLenum draw_mode, GLint start, GLsizei count);
void xpl_vao_program_draw_arrays_instanced(xpl_vao_t *vao, xpl_shader_t *shader, GLenum draw_mode, GLint start, GLsizei element_count, GLsizei instance_count);

// Draw elements using the index buffer with the supplied index.
void xpl_vao_program_draw_elements(xpl_vao_t *vao, xpl_shader_t *shader, GLenum draw_mode, int ibo_index);
void xpl_vao_program_draw_elements_count_offset(xpl_vao_t *vao, xpl_shader_t *shader,
								   GLenum draw_mode, int ibo_index, size_t count, size_t offset);

// Draw elements using the given index buffer indices. Terminate the index array with NULL.
void xpl_vao_program_draw_elements_multi(xpl_vao_t *vao, xpl_shader_t *shader, GLenum draw_mode, int *index_vbo_indices[]);

#endif
