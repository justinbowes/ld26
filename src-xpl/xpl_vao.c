//
//  xpl_vao.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-09.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <uthash.h>
#include <utlist.h>

#include "xpl_gl.h"
#include "xpl_memory.h"
#include "xpl_shader.h"
#include "xpl_bo.h"

#include "xpl_vec.h"

#include "xpl_vao.h"

static GLint platform_max_vertex_attribs = -1;

static xpl_vertex_attrib_t *vertex_attrib_new(const char *name, xpl_bo_t *vbo_source,
										GLint size, GLenum type, GLboolean normalize, GLsizei stride,
										GLsizei offset) {
	xpl_vertex_attrib_t *node = xpl_calloc_type(xpl_vertex_attrib_t);

	strncpy(node->name, name, VATTR_NAME_MAX);
	node->vbo_source = vbo_source;
	node->enabled = 1;

	node->size = size;
	node->type = type;
	node->normalize = normalize;
	node->stride = stride;
	node->offset = offset;

	if (platform_max_vertex_attribs == -1) {
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &platform_max_vertex_attribs);
	}

	return node;
}

static void vertex_attrib_destroy(xpl_vertex_attrib_t **ppvat) {
	xpl_vertex_attrib_t *vat = *ppvat;

	xpl_free(vat);
	*ppvat = NULL;
}

xpl_vao_t *xpl_vao_new() {
	xpl_vao_t *vao = xpl_calloc_type(xpl_vao_t);

	return vao;
}

xpl_vao_t *xpl_vao_clone(const xpl_vao_t *old_vao) {
	assert(old_vao);
	xpl_vao_t *new_vao = xpl_vao_new();

	xpl_bo_t *clone_bo;
	xpl_vertex_attrib_t *vattrib;

	DL_FOREACH(old_vao->vertex_attribs_list, vattrib) {
		clone_bo = xpl_bo_clone(vattrib->vbo_source);

		xpl_vao_define_vertex_attrib(new_vao, vattrib->name, clone_bo,
									vattrib->size, vattrib->type, vattrib->normalize,
									vattrib->stride, (GLsizei) vattrib->offset);

		if (!vattrib->enabled) {
			xpl_vao_disable_vertex_attrib(new_vao, vattrib->name);
		}
	}

	new_vao->do_teardown = old_vao->do_teardown;

	for (int i = 0; i < DRAW_ARRAYS_MAX; i++) {
		if (old_vao->index_bos[i]) {
			clone_bo = xpl_bo_clone(old_vao->index_bos[i]);
			xpl_vao_set_index_buffer(new_vao, i, clone_bo);
		}
	}

	return new_vao;
}

void xpl_vao_destroy(xpl_vao_t **ppvao) {
	xpl_vao_t *vao = *ppvao;
	assert(vao);

    if (vao->vao_id) glDeleteVertexArrays(1, &vao->vao_id);

	xpl_vertex_attrib_t *attrib, *tmp;

	HASH_ITER(hh, vao->vertex_attribs_hash, attrib, tmp) {
		HASH_DELETE(hh, vao->vertex_attribs_hash, attrib);
		vertex_attrib_destroy(&attrib);
		attrib = NULL;
	}


	xpl_free(vao);
	*ppvao = NULL;
}

void xpl_vao_define_vertex_attrib(xpl_vao_t *vao, const char *name,
								  xpl_bo_t *vbo, GLint size, GLenum type, GLboolean normalize,
								  GLsizei stride, GLsizei offset) {
	assert(vao);

	if (vao->vertex_attrib_count == platform_max_vertex_attribs) {
		LOG_ERROR(
				"Cannot define any more vertex attributes, platform limit of %d reached", platform_max_vertex_attribs);
		return;
	}

	xpl_vertex_attrib_t *va = vertex_attrib_new(name, vbo, size, type,
												normalize, stride, offset);

	HASH_ADD_STR(vao->vertex_attribs_hash, name, va);
	DL_APPEND(vao->vertex_attribs_list, va);

	vao->vertex_attrib_count++;
}

void xpl_vao_define_vertex_attrib_xvec2(xpl_vao_t *vao, const char *name, xpl_bo_t *vbo, GLsizei stride, GLsizei offset) {
    xpl_vao_define_vertex_attrib(vao, name, vbo, 2, GL_FLOAT, GL_FALSE, stride, offset);
}

void xpl_vao_define_vertex_attrib_xvec3(xpl_vao_t *vao, const char *name, xpl_bo_t *vbo, GLsizei stride, GLsizei offset) {
    xpl_vao_define_vertex_attrib(vao, name, vbo, 3, GL_FLOAT, GL_FALSE, stride, offset);
}

void xpl_vao_define_vertex_attrib_xvec4(xpl_vao_t *vao, const char *name, xpl_bo_t *vbo, GLsizei stride, GLsizei offset) {
    xpl_vao_define_vertex_attrib(vao, name, vbo, 4, GL_FLOAT, GL_FALSE, stride, offset);
}

xpl_vertex_attrib_t *xpl_vao_get_vertex_attrib(xpl_vao_t *vao, const char *name) {
	assert(vao);

	xpl_vertex_attrib_t *va;
	HASH_FIND_STR(vao->vertex_attribs_hash, name, va);
	if (!va) {
		LOG_ERROR(
				"Couldn't find vertex attribute %s on VAO %d", name, vao->vao_id);
	}
	return va;
}

void xpl_vao_enable_vertex_attrib(xpl_vao_t *vao, const char *name) {
	xpl_vertex_attrib_t *va = xpl_vao_get_vertex_attrib(vao, name);
	va->enabled = 1;
}

void xpl_vao_disable_vertex_attrib(xpl_vao_t *vao, const char *name) {
	assert(vao);

	xpl_vertex_attrib_t *va = xpl_vao_get_vertex_attrib(vao, name);
	if (!va) {
		LOG_ERROR(
				"Couldn't find vertex attribute %s on VAO %d", name, vao->vao_id);
		return;
	}
	va->enabled = 0;
}

xpl_bo_t *xpl_vao_set_index_buffer(xpl_vao_t *vao, int buffer_index,
								   xpl_bo_t *ibo) {
	assert(vao);

	xpl_bo_t *existing = vao->index_bos[buffer_index];
	vao->index_bos[buffer_index] = ibo;
	return existing;
}

static void vao_prepare_program_draw(xpl_vao_t *vao, xpl_shader_t *shader) {
    if (! vao->vao_id) {
        glGenVertexArrays(1, &vao->vao_id);
    }
    
	glBindVertexArray(vao->vao_id);

	xpl_vertex_attrib_t *el;

	DL_FOREACH(vao->vertex_attribs_list, el) {
		GLint va_id = xpl_shader_get_va(shader, vao, el->name);
		if ((va_id != -1) && (!el->enabled)) {
			// Short circuit case: just turn off the enabled VA.
			glDisableVertexAttribArray(va_id);
			continue;
		}

		if (va_id == -1) {
			// Nothing cached, nothing found.
			// We've logged enough already via the VA cache lookup.
			continue;
		}

		if (el->enabled) {
			// If the buffer object is not clean, it's not up to date and it
			// might not even exist.
			if (el->vbo_source->dirty_state != xpl_bods_clean) {
				xpl_bo_commit(el->vbo_source);
				LOG_INFO(
						"PERF: uncommitted buffer object associated with "
						"VAO %d attrib %s; committed BO %d", vao->vao_id, el->name, el->vbo_source->bo_id);
			}
			glBindBuffer(GL_ARRAY_BUFFER, el->vbo_source->bo_id);
			glVertexAttribPointer(va_id, el->size, el->type, el->normalize, el->stride, (GLvoid *)(intptr_t)el->offset);
			glEnableVertexAttribArray(va_id);
		} else {
			glDisableVertexAttribArray(va_id);
		}

	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL_DEBUG();
}

static inline void vao_teardown_program_draw(xpl_vao_t *vao) {
	if (vao->do_teardown) {
		glUseProgram(0);
		glBindVertexArray(0);
	}
}

static inline void vao_program_draw(xpl_vao_t *vao, GLenum draw_mode, GLint start,
								 GLsizei count) {
	glDrawArrays(draw_mode, start, count);
    GL_DEBUG();
}

#ifndef XPL_GLES
static inline void program_draw_instanced(xpl_vao_t *vao, GLenum draw_mode, GLint start, GLsizei count, GLsizei instance_count) {
	glDrawArraysInstanced(draw_mode, start, count, instance_count);
	GL_DEBUG();
}
#endif

static inline void vao_program_draw_index_count_offset(xpl_vao_t *vao, GLenum draw_mode,
									   int ibo_index, size_t elements, size_t offset) {
	// We don't bind a vertex array buffer -- those were bound via vertex attributes.
	xpl_bo_t *ibo = vao->index_bos[ibo_index];
	assert(ibo);

	if (ibo->dirty_state != xpl_bods_clean) {
		xpl_bo_commit(ibo);
		LOG_INFO("PERF: uncommitted buffer object associated with VAO %d IBO %d; committed BO %d",
                 vao->vao_id, ibo_index, ibo->bo_id);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->bo_id);
	glDrawElements(draw_mode, (GLsizei)elements, GL_UNSIGNED_SHORT, (GLvoid *)offset);
    GL_DEBUG();
}

static inline void vao_program_draw_index(xpl_vao_t *vao, GLenum draw_mode,
									   int ibo_index) {
	// We don't bind a vertex array buffer -- those were bound via vertex attributes.
	xpl_bo_t *ibo = vao->index_bos[ibo_index];
	assert(ibo);

	vao_program_draw_index_count_offset(vao, draw_mode, ibo_index, ibo->client_data->length / sizeof (GLushort), 0);
}

#ifndef XPL_GLES
void xpl_vao_program_draw_arrays_instanced(xpl_vao_t *vao, xpl_shader_t *shader, GLenum draw_mode, GLint start, GLsizei element_count, GLsizei instance_count) {
    assert(vao);
    assert(shader && shader->id);
    vao_prepare_program_draw(vao, shader);
    program_draw_instanced(vao, draw_mode, start, element_count, instance_count);
    vao_teardown_program_draw(vao);
}
#endif

void xpl_vao_program_draw_arrays(xpl_vao_t *vao, xpl_shader_t *shader,
								 GLenum draw_mode, GLint start, GLsizei count) {
    assert(vao);
    assert(shader && shader->id);
	vao_prepare_program_draw(vao, shader);
    assert(vao->vao_id);
	vao_program_draw(vao, draw_mode, start, count);
	vao_teardown_program_draw(vao);
}

void xpl_vao_program_draw_elements(xpl_vao_t *vao, xpl_shader_t *shader,
								   GLenum draw_mode, int ibo_index) {
	assert(vao && vao->index_bos[ibo_index]);
	assert(shader && shader->id);
	vao_prepare_program_draw(vao, shader);
    assert(vao->vao_id);
	vao_program_draw_index(vao, draw_mode, ibo_index);
	vao_teardown_program_draw(vao);
}

void xpl_vao_program_draw_elements_count_offset(xpl_vao_t *vao, xpl_shader_t *shader,
								   GLenum draw_mode, int ibo_index, size_t count, size_t offset) {
	vao_prepare_program_draw(vao, shader);
	vao_program_draw_index_count_offset(vao, draw_mode, ibo_index, count, offset);
	vao_teardown_program_draw(vao);
}

void xpl_vao_program_draw_elements_multi(xpl_vao_t *vao, xpl_shader_t *shader,
										 GLenum draw_mode, int *ibo_indices[]) {
	vao_prepare_program_draw(vao, shader);
	int *pibo_index = ibo_indices[0];
	while (*pibo_index) {
		vao_program_draw_index(vao, draw_mode, *pibo_index);
		pibo_index++;
	}
	vao_teardown_program_draw(vao);
}
