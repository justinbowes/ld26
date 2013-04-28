//
//  xpl_model.c
//  p1
//
//  Created by Justin Bowes on 2013-01-18.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdint.h>
#include <limits.h>

#include "xpl_thread.h"

#include "xpl_model.h"


static void vn_vao_attrs(xpl_vao_t *vao, xpl_bo_t *bo) {
    xpl_vao_define_vertex_attrib_xvec3(vao, "position", bo, sizeof(vertex_normal_t), offsetof(vertex_normal_t, vertex));
    xpl_vao_define_vertex_attrib_xvec3(vao, "normal", bo, sizeof(vertex_normal_t), offsetof(vertex_normal_t, normal));
}

static void vnc_vao_attrs(xpl_vao_t *vao, xpl_bo_t *bo) {
    xpl_vao_define_vertex_attrib_xvec3(vao, "position", bo, sizeof(vertex_normal_color_t), offsetof(vertex_normal_color_t, vertex));
    xpl_vao_define_vertex_attrib_xvec3(vao, "normal", bo, sizeof(vertex_normal_color_t), offsetof(vertex_normal_color_t, normal));
    xpl_vao_define_vertex_attrib_xvec4(vao, "color", bo, sizeof(vertex_normal_color_t), offsetof(vertex_normal_color_t, color));
}

static void vnt_vao_attrs(xpl_vao_t *vao, xpl_bo_t *bo) {
    xpl_vao_define_vertex_attrib_xvec3(vao, "position", bo, sizeof(vertex_normal_uv_t), offsetof(vertex_normal_uv_t, vertex));
    xpl_vao_define_vertex_attrib_xvec3(vao, "normal", bo, sizeof(vertex_normal_uv_t), offsetof(vertex_normal_uv_t, normal));
    xpl_vao_define_vertex_attrib_xvec2(vao, "uv", bo, sizeof(vertex_normal_uv_t), offsetof(vertex_normal_uv_t, uv));
}

typedef void(* vao_attr_func)(xpl_vao_t *, xpl_bo_t *);

typedef struct vertex_type_handlers {
    size_t          stride;
    vao_attr_func   define_vao_attrs;
} vertex_type_handlers_t;

static vertex_type_handlers_t vt_info[xmvt__last] = {
    { sizeof(vertex_normal_t), vn_vao_attrs },
    { sizeof(vertex_normal_color_t), vnc_vao_attrs },
    { sizeof(vertex_normal_uv_t), vnt_vao_attrs }
};

void xpl_lod_load(const xpl_lod_t *lod, xpl_vao_t *vao, xpl_bo_t *vbo, float *extent_out) {
    
    size_t stride = vt_info[lod->type].stride;
    
    xpl_bo_append(vbo, lod->data, stride * lod->elements);
    if (xpl_thread_is_primary()) {
        xpl_bo_commit(vbo);
    }
    
    // Get spherical extent from model
	float extent = 0.f;
	void *ptr = lod->data;
	for (size_t i = 0; i < lod->elements; ++i) {
		xvec3 *vtx_ptr = (xvec3 *)ptr;
		extent = xmax(extent, vtx_ptr->x * vtx_ptr->x + vtx_ptr->y * vtx_ptr->y + vtx_ptr->z * vtx_ptr->z);
		ptr += stride;
	}
    *extent_out = extent;
    
    vt_info[lod->type].define_vao_attrs(vao, vbo);
    
}

size_t xpl_lod_stride(const xpl_model_vertex_type_t vertex_type) {
    return vt_info[vertex_type].stride;
}

void xpl_lod_offsets(const xpl_model_vertex_type_t vertex_type,
                     size_t *vertex_offset, size_t *normal_offset, size_t *color_offset, size_t *uv_offset) {
    switch (vertex_type) {
        case xmvt_vertex_normal:
            if (vertex_offset)  *vertex_offset  = offsetof(vertex_normal_t, vertex);
            if (normal_offset)  *normal_offset  = offsetof(vertex_normal_t, normal);
            if (color_offset)   *color_offset   = SIZE_MAX;
            if (uv_offset)      *uv_offset      = SIZE_MAX;
            break;
            
        case xmvt_vertex_normal_color:
            if (vertex_offset)  *vertex_offset  = offsetof(vertex_normal_color_t, vertex);
            if (normal_offset)  *normal_offset  = offsetof(vertex_normal_color_t, normal);
            if (color_offset)   *color_offset   = offsetof(vertex_normal_color_t, color);
            if (uv_offset)      *uv_offset      = SIZE_MAX;
            break;
            
        case xmvt_vertex_normal_uv:
            if (vertex_offset)  *vertex_offset  = offsetof(vertex_normal_uv_t, vertex);
            if (normal_offset)  *normal_offset  = offsetof(vertex_normal_uv_t, normal);
            if (color_offset)   *color_offset   = SIZE_MAX;
            if (uv_offset)      *uv_offset      = offsetof(vertex_normal_uv_t, uv);
            break;
            
        default:
            assert(false);
            break;
    }
}

