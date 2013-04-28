/*
 * xpl_skybox.h
 *
 *  Created on: 2012-12-29
 *      Author: Justin
 */

#ifndef XPL_SKYBOX_H_
#define XPL_SKYBOX_H_

#include "GL3/gl3.h"

#include "xpl.h"
#include "xpl_vao.h"
#include "xpl_texture.h"
#include "xpl_bo.h"
#include "xpl_vec.h"
#include "xpl_shader.h"

// Matched to SOIL api ordering
enum skybox_plane {
	sp_right = 0,
	sp_left = 1,
	sp_top = 2,
	sp_bottom = 3,
	sp_front = 4,
	sp_back = 5,
};

typedef struct xpl_skybox_plane_def {
	enum skybox_plane plane;
	const char *suffix;
} xpl_skybox_plane_def_t;

extern const xpl_skybox_plane_def_t XPL_SKYBOX_FORMAT_SPACESCAPE[];

typedef struct xpl_skybox {
	xpl_shader_t *shader;
	xpl_vao_t *vao;
	xpl_bo_t *vertices;
	xpl_bo_t *indices;
	GLuint cubemap_texture;
} xpl_skybox_t;

xpl_skybox_t *xpl_skybox_new(const char *texture_group, const xpl_skybox_plane_def_t *format);
void xpl_skybox_destroy(xpl_skybox_t **skybox);

void xpl_skybox_render(xpl_skybox_t *self, const GLfloat *vp);

#endif /* XPL_SKYBOX_H_ */
