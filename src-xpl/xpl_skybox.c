/*
 * xpl_skybox.c
 *
 *  Created on: 2012-12-29
 *      Author: Justin
 */

#include <assert.h>

#include "xpl_gl.h"
#include "SOIL.h"
#include "xpl_memory.h"

#include "xpl_skybox.h"

const xpl_skybox_plane_def_t XPL_SKYBOX_FORMAT_SPACESCAPE[] = {
		{ sp_right, "_right1.png" },
		{ sp_left, "_left2.png" },
		{ sp_top, "_top3.png" },
		{ sp_bottom, "_bottom4.png" },
		{ sp_front, "_front5.png" },
		{ sp_back, "_back6.png" }
};

static GLfloat cube_vertices[] = {
	-1.0,  1.0,  1.0,
	-1.0, -1.0,  1.0,
	 1.0, -1.0,  1.0,
	 1.0,  1.0,  1.0,
	-1.0,  1.0, -1.0,
	-1.0, -1.0, -1.0,
	 1.0, -1.0, -1.0,
	 1.0,  1.0, -1.0
};

static GLushort cube_indices[] = {
	0, 1, 2, 2, 3, 0,
	3, 2, 6, 6, 7, 3,
	7, 6, 5, 5, 4, 7,
	4, 5, 1, 1, 0, 4,
	0, 3, 7, 7, 4, 0,
	1, 2, 6, 6, 5, 1
};

#define SKYBOX_ACTIVE_TEXTURE 7

xpl_skybox_t *xpl_skybox_new(const char *texture_group, const xpl_skybox_plane_def_t *format) {
#ifndef XPL_GLES
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
	

	xpl_skybox_t *skybox = xpl_calloc_type(xpl_skybox_t);

	// ---- Load textures

	char resources[6][PATH_MAX];
	char filenames[6][PATH_MAX];
	for (size_t i = 0; i < 6; ++i) {
		const xpl_skybox_plane_def_t *def = &format[i];
		int plane_index = def->plane;
		snprintf(resources[plane_index], PATH_MAX, "bitmaps/%s%s", texture_group, def->suffix);
		xpl_resolve_resource(filenames[plane_index], resources[plane_index], PATH_MAX);
	}

	GLint original_unpack_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_unpack_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	skybox->cubemap_texture = SOIL_load_OGL_cubemap(filenames[0], filenames[1],
			filenames[2], filenames[3],	filenames[4], filenames[5],
			SOIL_LOAD_AUTO, skybox->cubemap_texture,
			SOIL_FLAG_MULTIPLY_ALPHA);

	glPixelStorei(GL_UNPACK_ALIGNMENT, original_unpack_alignment);

	skybox->shader = xpl_shader_get_prepared("Skybox", "Skybox.Vertex", "Skybox.Fragment");

	// ---- Prepare VAO

	skybox->vao = xpl_vao_new();

	skybox->vertices = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	xpl_bo_append(skybox->vertices, &cube_vertices[0], sizeof(cube_vertices));
	xpl_bo_commit(skybox->vertices);

	skybox->indices = xpl_bo_new(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
	xpl_bo_append(skybox->indices, &cube_indices[0], sizeof(cube_indices));
	xpl_bo_commit(skybox->indices);

	xpl_vao_set_index_buffer(skybox->vao, 0, skybox->indices);
	xpl_vao_define_vertex_attrib(skybox->vao, "position", skybox->vertices, 3, GL_FLOAT, FALSE, 3 * sizeof(GLfloat), 0);

	// ---- Prepare shader

	glUseProgram(skybox->shader->id);
	glUniform1i(xpl_shader_get_uniform(skybox->shader, "cubemap"), SKYBOX_ACTIVE_TEXTURE);
	glUseProgram(GL_NONE);
    GL_DEBUG();

	return skybox;
}

void xpl_skybox_destroy(xpl_skybox_t **ppskybox) {
	assert(ppskybox);
	xpl_skybox_t *skybox = *ppskybox;
	assert(skybox);

	xpl_shader_release(&skybox->shader);
	xpl_vao_destroy(&skybox->vao);
	xpl_bo_destroy(&skybox->vertices);
	xpl_bo_destroy(&skybox->indices);
	glDeleteTextures(1, &skybox->cubemap_texture);
    
    GL_DEBUG();

	xpl_free(skybox);
	*ppskybox = NULL;
}

static xmat4 mat_skybox;

void xpl_skybox_render(xpl_skybox_t *self, const GLfloat *vp) {
	memmove(&mat_skybox.data[0], vp, 16 * sizeof(GLfloat));
    // Set zero translation...could just zero out .w somewhere...
	mat_skybox.data[12] = mat_skybox.data[13] = mat_skybox.data[14] = mat_skybox.data[15] = 0;

    glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
    GL_DEBUG();
    
	glUseProgram(self->shader->id);
    glActiveTexture(GL_TEXTURE0 + SKYBOX_ACTIVE_TEXTURE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, self->cubemap_texture);
	glUniformMatrix4fv(xpl_shader_get_uniform(self->shader, "mvp"), 1, GL_FALSE, &mat_skybox.data[0]);
	xpl_vao_program_draw_elements(self->vao, self->shader, GL_TRIANGLES, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
    
    glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    
    GL_DEBUG();
}
