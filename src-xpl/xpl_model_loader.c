//
//  xpl_model_loader.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-10-07.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utlist.h"

#include "xpl.h"
#include "xpl_memory.h"
#include "xpl_log.h"
#include "xpl_vao.h"

#include "xpl_model_loader.h"

#define INVERT_Z            1
#define CW_WINDING_ORDER    0
#define OBJ_FILE_LINE_SIZE  1024
#define WHITESPACE          " \t\n\r"

xpl_model_material_t *xpl_model_material_new(void) {
	xpl_model_material_t *material = xpl_alloc_type(xpl_model_material_t);

	static const xvec3 ambient = {
		{ 0.2f, 0.2f, 0.2f }
	};
	static const xvec3 diffuse = {
		{ 0.5f, 0.5f, 0.5f }
	};
	static const xvec3 specular = {
		{ 1.0f, 1.0f, 1.0f }
	};
	static const xvec3 emissive = {
		{ 1.0f, 1.0f, 1.0f }
	};

	material->ambient = ambient;
	material->diffuse = diffuse;
	material->specular = specular;
	material->emissive = emissive;
	material->reflectiveness = 0.0f;
	material->refractiveness = 0.0f;
	material->refraction_index = 1.0f;
	material->opacity = 1.0f;
	material->shininess = 0.0f;
	material->glossiness = 98.0f;

	material->name[0] = '\0';
	material->texture = NULL;

	material->prev = NULL;
	material->next = NULL;

	return material;
}

void xpl_model_material_destroy(xpl_model_material_t **ppmaterial) {
	xpl_model_material_t *material = *ppmaterial;

	xpl_texture_destroy(&material->texture);

	xpl_free(material);
	*ppmaterial = NULL;
}

xpl_model_face_t *xpl_model_face_new(void) {
	xpl_model_face_t *face = xpl_alloc_type(xpl_model_face_t);

	face->vertex_count = 0;

	face->prev = NULL;
	face->next = NULL;

	return face;
}

void xpl_model_face_destroy(xpl_model_face_t **ppface) {
	xpl_model_face_t *face = *ppface;

	xpl_free(face);

	*ppface = NULL;
}

static const UT_icd vec3_icd = { sizeof (xvec3), NULL, NULL, NULL };

xpl_model_t *xpl_model_new(void) {
	xpl_model_t *model = xpl_alloc_type(xpl_model_t);

	model->materials = NULL;
	model->material_count = 0;

	model->faces = NULL;
	model->face_count = 0;
	model->face_vertex_count = 0;

	utarray_new(model->vertices, &vec3_icd);
	model->vertex_count = 0;

	utarray_new(model->normals, &vec3_icd);
	model->normal_count = 0;

	utarray_new(model->texcoords, &vec3_icd);
	model->texcoord_count = 0;

	model->vao = NULL;
	model->vbo = NULL;
	model->shader = NULL;

	return model;
}

void xpl_model_destroy(xpl_model_t **ppmodel) {
	xpl_model_t *model = *ppmodel;

	xpl_model_material_t *material;
	while (model->materials) {
		material = model->materials;
		model->materials = material->next;
		xpl_model_material_destroy(&material);
	}

	xpl_model_face_t *face;
	while (model->faces) {
		face = model->faces;
		model->faces = face->next;
		xpl_model_face_destroy(&face);
	}

	utarray_free(model->vertices);
	utarray_free(model->normals);
	utarray_free(model->texcoords);

	if (model->vao) xpl_vao_destroy(&model->vao);
	if (model->vbo) xpl_bo_destroy(&model->vbo);

	xpl_free(model);
	*ppmodel = NULL;
}

XPLINLINE int strequal(const char *s1, const char *s2) {
	return (strcmp(s1, s2) == 0);
}

XPLINLINE int strcontains(const char *haystack, const char *needle) {
	return (strstr(haystack, needle) != NULL);
}

static void wf_parse_vector(xvec3 *vector) {
	char *pch;
	pch = strtok(NULL, WHITESPACE);
	if (pch) {
		vector->x = atof(pch);
		pch = strtok(NULL, WHITESPACE);
		if (pch) {
			vector->y = atof(pch);
			pch = strtok(NULL, WHITESPACE);
			if (pch) {
#                if INVERT_Z==1
				vector->z = -atof(pch);
#                else
				vector->z = atof(pch);
#                endif
			} else {
				vector->z = 0.0f;
			}
		} else {
			vector->y = 0.0f;
			vector->z = 0.0f;
		}
	} else {
		vector->x = 0.0f;
		vector->y = 0.0f;
		vector->z = 0.0f;
	}
}

static void wf_add_vector(UT_array *vertices, int *vertex_count) {
	xvec3 vector;
	wf_parse_vector(&vector);
	utarray_push_back(vertices, &vector);
	++(*vertex_count);
}

static int wf_parse_vertex_index(int *vertex_indices, int *texture_indices, int *normal_indices) {
	char *temp;
	char *token;
	int vertex_count = 0;

	while ((token = strtok(NULL, WHITESPACE))) {
		if (texture_indices) texture_indices[vertex_count] = 0;
		if (normal_indices) normal_indices[vertex_count] = 0;

		vertex_indices[vertex_count] = atoi(token);

		if (strcontains(token, "//")) {                 // normal only
			temp = strchr(token, '/');
			++temp;
			normal_indices[vertex_count] = atoi(++temp);
		} else if (strcontains(token, "/")) {           // not just a vertex

			// First is texture.
			temp = strchr(token, '/');
			texture_indices[vertex_count] = atoi(++temp);

			// If another exists, it's normal.
			if (strcontains(temp, "/")) {
				temp = strchr(temp, '/');
				normal_indices[vertex_count] = atoi(++temp);
			}
		}

		++vertex_count;
	}

	return vertex_count;
}

// Apparently wavefront indices are 0-based.

static int wf_convert_index(int item_count, int index) {
	// If this asserts, the file may be invalid or empty.
	assert(index > 0); // I don't see this documented
	return index - 1;  // 1 based to 0 based
}

static void wf_convert_indices(int item_count, int index_count, int *indices) {
	for (int i = 0; i < index_count; ++i) {
		indices[i] = wf_convert_index(item_count, indices[i]);
	}
}

static void wf_parse_face(xpl_model_t *model, int current_material) {
	xpl_model_face_t *face = xpl_model_face_new();

	face->vertex_count = wf_parse_vertex_index(face->vertex_indices,
											face->texture_indices,
											face->normal_indices);
	wf_convert_indices(model->vertex_count,
						face->vertex_count,
						face->vertex_indices);
	wf_convert_indices(model->normal_count,
						face->vertex_count,
						face->normal_indices);
	wf_convert_indices(model->texcoord_count,
						face->vertex_count,
						face->texture_indices);

	face->material_index = current_material;

	DL_APPEND(model->faces, face);
	++model->face_count;
	model->face_vertex_count += face->vertex_count;


}

static int wf_find_material(xpl_model_t *model, const char *material_name) {
	xpl_model_material_t *el;
	int index = 0;

	DL_FOREACH(model->materials, el) {
		if (strequal(el->name, material_name)) return index;
		++index;
	}
	return -1;
}

static void wf_add_material_lib(xpl_model_t *model, const char *resource) {

	int line_number = 0;
	char *current_token = NULL;
	char current_line[OBJ_FILE_LINE_SIZE];
	int material_open = FALSE;
	xpl_model_material_t *current_material = NULL;

	FILE *mtl_file_stream = fopen(resource, "r");
	if (!mtl_file_stream) {
		LOG_ERROR("Couldn't open file %s", resource);
		return;
	}

	while (fgets(current_line, OBJ_FILE_LINE_SIZE, mtl_file_stream)) {
		++line_number;
		current_token = strtok(current_line, WHITESPACE);

		// Skip blank lines and comments.
		if ((current_token == NULL) || (current_token[0] == '#')) continue;

		if (strequal(current_token, "newmtl")) {                                // start new material
			material_open = TRUE;
			current_material = xpl_model_material_new();
			char *material_name = strtok(NULL, " \t");
			strncpy(current_material->name, material_name, MATERIAL_NAME_SIZE);
			DL_APPEND(model->materials, current_material);
			++(model->material_count);

		} else if (material_open && strequal(current_token, "Ka")) {            // ambient
			wf_parse_vector(&current_material->ambient);

		} else if (material_open && strequal(current_token, "Kd")) {            // diffuse
			wf_parse_vector(&current_material->diffuse);

		} else if (material_open && strequal(current_token, "Ks")) {            // specular
			wf_parse_vector(&current_material->specular);

		} else if (material_open && strequal(current_token, "Ns")) {            // shininess
			current_material->shininess = atof(strtok(NULL, WHITESPACE));

		} else if (material_open && strequal(current_token, "d")) {             // opacity
			current_material->opacity = atof(strtok(NULL, WHITESPACE));

		} else if (material_open && strequal(current_token, "r")) {             // reflectiveness
			current_material->reflectiveness = atof(strtok(NULL, WHITESPACE));

		} else if (material_open && strequal(current_token, "sharpness")) {     // glossiness?!
			current_material->glossiness = atof(strtok(NULL, WHITESPACE));

		} else if (material_open && strequal(current_token, "Ni")) {            // refract index
			current_material->refraction_index = atof(strtok(NULL, WHITESPACE));

		} else if (material_open && strequal(current_token, "illum")) {
			LOG_WARN("Illumination not handled");

		} else if (material_open && strequal(current_token, "map_Ka")) {
			char *filename = strtok(NULL, WHITESPACE);
			char tex_resource_name[PATH_MAX];
			xpl_resolve_resource(tex_resource_name, filename, PATH_MAX);
			current_material->texture = xpl_texture_new();
			xpl_texture_load(current_material->texture, tex_resource_name, true);

		} else {
			LOG_WARN("Unknown command '%s' in material file %s at line %i:\n\t%s",
					current_token, resource, line_number, current_line);
		}
	}

}

int xpl_model_load_obj(xpl_model_t *model, const char *resource) {

	int current_material = -1;
	char *current_token = NULL;
	char current_line[OBJ_FILE_LINE_SIZE];
	int line_number = 0;

	char filename[PATH_MAX];
	char resource_path[PATH_MAX];
	snprintf(resource_path, PATH_MAX, "models/%s", resource);
	xpl_resolve_resource(filename, resource_path, PATH_MAX);

	FILE *obj_file_stream = fopen(filename, "r");
	if (!obj_file_stream) {
		LOG_ERROR("Couldn't open file %s", filename);
		return FALSE;
	}

	while (fgets(current_line, OBJ_FILE_LINE_SIZE, obj_file_stream)) {
		++line_number;
		current_token = strtok(current_line, WHITESPACE);

		// Skip blank lines and comments.
		if ((current_token == NULL) || (current_token[0] == '#')) continue;

		if (strequal(current_token, "v")) {                 // vertex
			wf_add_vector(model->vertices, &model->vertex_count);

		} else if (strequal(current_token, "vn")) {         // normal
			wf_add_vector(model->normals, &model->normal_count);

		} else if (strequal(current_token, "vt")) {         // texture
			wf_add_vector(model->texcoords, &model->texcoord_count);

		} else if (strequal(current_token, "f")) {          // face
			wf_parse_face(model, current_material);

		} else if (strequal(current_token, "usemtl")) {
			const char *material_name = strtok(NULL, WHITESPACE);
			current_material = wf_find_material(model, material_name);
			if (current_material < 0) {
				LOG_WARN("Material named %s not found", material_name);
			}

		} else if (strequal(current_token, "mtllib")) {
			char fname[PATH_MAX];
			char rname[PATH_MAX];
			xpl_resolve_resource(rname, fname, PATH_MAX);
			wf_add_material_lib(model, rname);

		} else if (strequal(current_token, "o")) {
			LOG_DEBUG("Ignored: object name");

		} else if (strequal(current_token, "s")) {
			LOG_DEBUG("Ignored: smoothing");

		} else if (strequal(current_token, "g")) {
			LOG_DEBUG("Ignored: group");

		} else {
			LOG_WARN("Unknown command %s in scene code at line %i:\n\t%s",
					current_token, line_number, current_line);

		}

	}

	fclose(obj_file_stream);
	return 1;
}

xpl_model_t *xpl_model_load_from_wavefront(const char *wavefront_resource_name) {
	xpl_model_t *model = xpl_model_new();

	if (! xpl_model_load_obj(model, wavefront_resource_name)) {
		LOG_ERROR("Couldn't load model from %s", wavefront_resource_name);
		xpl_model_destroy(&model);
		return NULL;
	}

	return model;
}

typedef struct _mvertex {
	xvec3 position;
	xvec3 normal;
	xvec3 texture;

	int tex_index;
	// 10

	xvec4 ambient;
	xvec4 diffuse;
	xvec4 specular;
	xvec4 emissive;
	// 26

	float reflectiveness;
	float refractiveness;
	float refraction_index;
	float shininess;
	float glossiness;

	float padding;
} _mvertex_t;
static const size_t _mvertex_stride = sizeof (_mvertex_t);

void xpl_model_build_vao(xpl_model_t *model) {
	assert(sizeof (float) == sizeof (int)); // otherwise _mvertex is bogus

	assert(model->vao == NULL);
	assert(model->vbo == NULL);

	xpl_bo_t *vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	xpl_bo_t *ibo = xpl_bo_new(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

	size_t index_bytes = model->face_vertex_count * sizeof (GLushort);
	size_t vertex_bytes = model->face_vertex_count * sizeof (_mvertex_t);
	GLushort *indices = (GLushort *)xpl_alloc(index_bytes);
	_mvertex_t *vertices = (_mvertex_t *)xpl_alloc(vertex_bytes);
	// GLushort indices[model->face_vertex_count];
	//_mvertex_t vertices[model->face_vertex_count];

	size_t vertex_index = 0, index_index = 0;
	xpl_model_face_t *face;
	xpl_model_material_t *material;

	// Add a default material.
	material = xpl_model_material_new();
	// Can't do the _new call in the macro; it's evaluated more than once
	DL_APPEND(model->materials, material);
	model->material_count++;

	DL_FOREACH(model->faces, face) {
		// If no material was assigned, use the last (default) material.
		if (face->material_index < 0) {
			face->material_index = model->material_count - 1;
		}
		material = (model->materials) + face->material_index;

#        if CW_WINDING_ORDER==1
		// OBJ files seem to have CW winding order
		for (int face_vertex_index = 0; face_vertex_index < face->vertex_count; ++face_vertex_index) {
#        else
		for (int face_vertex_index = face->vertex_count - 1; face_vertex_index >= 0; --face_vertex_index) {
#            endif
			size_t vertex_offset = face->vertex_indices[face_vertex_index];
			size_t normal_offset = face->normal_indices[face_vertex_index];
			size_t texcoord_offset = face->texture_indices[face_vertex_index];

			xvec3 *vertex;

			vertex = (xvec3 *)utarray_eltptr(model->vertices, vertex_offset);
			vertices[vertex_index].position = *vertex;

			vertex = (xvec3 *)utarray_eltptr(model->normals, normal_offset);
			vertices[vertex_index].normal = *vertex;

			vertex = (xvec3 *)utarray_eltptr(model->texcoords, texcoord_offset);
			vertices[vertex_index].texture = *vertex;

			vertices[vertex_index].tex_index = face->material_index;

			vertices[vertex_index].ambient.r = material->ambient.r;
			vertices[vertex_index].ambient.g = material->ambient.g;
			vertices[vertex_index].ambient.b = material->ambient.b;
			vertices[vertex_index].ambient.a = material->opacity;

			vertices[vertex_index].diffuse.r = material->diffuse.r;
			vertices[vertex_index].diffuse.g = material->diffuse.g;
			vertices[vertex_index].diffuse.b = material->diffuse.b;
			vertices[vertex_index].diffuse.a = material->opacity;

			vertices[vertex_index].specular.r = material->specular.r;
			vertices[vertex_index].specular.g = material->specular.g;
			vertices[vertex_index].specular.b = material->specular.b;
			vertices[vertex_index].specular.a = material->opacity;

			vertices[vertex_index].emissive.r = material->emissive.r;
			vertices[vertex_index].emissive.g = material->emissive.g;
			vertices[vertex_index].emissive.b = material->emissive.b;
			vertices[vertex_index].emissive.a = material->opacity;

			vertices[vertex_index].reflectiveness = material->reflectiveness;
			vertices[vertex_index].refractiveness = material->refractiveness;
			vertices[vertex_index].refraction_index = material->refraction_index;
			vertices[vertex_index].shininess = material->shininess;
			vertices[vertex_index].glossiness = material->glossiness;

			indices[index_index++] = (GLushort)vertex_index;
			vertex_index++;
		}
	}

	xpl_bo_append(vbo, vertices, vertex_bytes);
	xpl_bo_commit(vbo);
	xpl_bo_append(ibo, indices, index_bytes);
	xpl_bo_commit(ibo);

	xpl_vao_t *vao = xpl_vao_new();
	xpl_vao_set_index_buffer(vao, 0, ibo);
	xpl_vao_define_vertex_attrib(vao, "position", vbo, 3, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, position));
	xpl_vao_define_vertex_attrib(vao, "normal", vbo, 3, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, normal));
	xpl_vao_define_vertex_attrib(vao, "texcoord", vbo, 3, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, texture));
	xpl_vao_define_vertex_attrib(vao, "texture_index", vbo, 1, GL_INT, FALSE, _mvertex_stride, offsetof(_mvertex_t, tex_index));
	xpl_vao_define_vertex_attrib(vao, "ambient", vbo, 4, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, ambient));
	xpl_vao_define_vertex_attrib(vao, "diffuse", vbo, 4, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, diffuse));
	xpl_vao_define_vertex_attrib(vao, "specular", vbo, 4, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, specular));
	xpl_vao_define_vertex_attrib(vao, "emissive", vbo, 4, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, emissive));
	xpl_vao_define_vertex_attrib(vao, "reflectiveness", vbo, 1, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, reflectiveness));
	xpl_vao_define_vertex_attrib(vao, "refractiveness", vbo, 1, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, refractiveness));
	xpl_vao_define_vertex_attrib(vao, "refraction_index", vbo, 1, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, refraction_index));
	xpl_vao_define_vertex_attrib(vao, "shininess", vbo, 1, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, shininess));
	xpl_vao_define_vertex_attrib(vao, "glossiness", vbo, 1, GL_FLOAT, FALSE, _mvertex_stride, offsetof(_mvertex_t, glossiness));

	model->vao = vao;
	model->vbo = vbo;
}

