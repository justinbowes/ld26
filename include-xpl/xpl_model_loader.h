//
//  xpl_model_loader.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-10-07.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_model_loader_h
#define xpl_osx_xpl_model_loader_h

#include "utarray.h"

#include "xpl_vao.h"
#include "xpl_bo.h"
#include "xpl_shader.h"
#include "xpl_vec.h"
#include "xpl_texture.h"

#define MAX_VERTEX_COUNT 4
#define MATERIAL_NAME_SIZE 255

typedef struct xpl_model_face {

	int                     vertex_indices[MAX_VERTEX_COUNT];
	int                     normal_indices[MAX_VERTEX_COUNT];
	int                     texture_indices[MAX_VERTEX_COUNT];
	int                     vertex_count;
	int                     material_index;

	struct xpl_model_face   *prev;
	struct xpl_model_face   *next;
} xpl_model_face_t;

typedef struct xpl_model_material {

	xvec3                   ambient;
	xvec3                   diffuse;
	xvec3                   specular;
	xvec3                   emissive;
	float                   reflectiveness;
	float                   refractiveness;
	float                   refraction_index;
	float                   opacity;
	float                   shininess;
	float                   glossiness;

	char                    name[MATERIAL_NAME_SIZE];
	xpl_texture_t           *texture;

	struct xpl_model_material *prev;
	struct xpl_model_material *next;
} xpl_model_material_t;

typedef struct xpl_model {

	xpl_model_material_t    *materials;
	int                     material_count;

	xpl_model_face_t        *faces;
	int                     face_count;
	int                     face_vertex_count;

	UT_array                *vertices; // xvec3
	int                     vertex_count;
	UT_array                *normals; // xvec3
	int                     normal_count;
	UT_array                *texcoords; // xvec3...
	int                     texcoord_count;

	xpl_vao_t               *vao;
	xpl_bo_t                *vbo;
	xpl_shader_t            *shader;

} xpl_model_t;



xpl_model_t *xpl_model_new(void);
void xpl_model_destroy(xpl_model_t **ppmodel);

xpl_model_t *xpl_model_load_from_wavefront(const char *wavefront_resource_name);
void xpl_model_build_vao(xpl_model_t *model);

#endif
