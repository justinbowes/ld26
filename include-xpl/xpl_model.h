/*
 * xpl_model.h
 *
 *  Created on: 2013-01-11
 *      Author: Justin
 */

#ifndef XPL_MODEL_H_
#define XPL_MODEL_H_

#include <stddef.h>

#include "xpl_vec.h"
#include "xpl_vao.h"

#ifndef XPL_MAX_LODS
#define XPL_MAX_LODS 3
#endif

#define XPL_LOD(data, vertex_type_enum, threshold) \
		{ \
			(void *)data, \
			vertex_type_enum, \
			sizeof(data) / sizeof(data[0]), \
			threshold \
		}
#define XPL_LOD_LAST() \
		{ \
			NULL, \
			xmvt__last, \
			0, \
			-1.0f \
		}

typedef enum xpl_model_vertex_type {
    xmvt_vertex_normal,
    xmvt_vertex_normal_color,
    xmvt_vertex_normal_uv,
    xmvt__last
} xpl_model_vertex_type_t;

typedef struct xpl_lod {
    void                    *data;
    xpl_model_vertex_type_t type;
    size_t                  elements;
    float                   threshold;
} xpl_lod_t;

typedef struct vertex_normal {
	xvec3		vertex;
	xvec3		normal;
} vertex_normal_t;

typedef struct vertex_normal_color {
	xvec3		vertex;
	xvec3		normal;
	xvec4		color;
} vertex_normal_color_t;

typedef struct vertex_normal_uv {
	xvec3		vertex;
	xvec3		normal;
	xvec2		uv;
} vertex_normal_uv_t;

void xpl_lod_load(const xpl_lod_t *lod, xpl_vao_t *vao, xpl_bo_t *vbo, float *extent_out);
size_t xpl_lod_stride(const xpl_model_vertex_type_t vertex_type);
void xpl_lod_offsets(const xpl_model_vertex_type_t vertex_type, size_t *vertex_offset, size_t *normal_offset, size_t *color_offset, size_t *uv_offset);

#endif /* XPL_MODEL_H_ */
