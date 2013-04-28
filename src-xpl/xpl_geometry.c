//
//  xpl_geometry.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-14.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <limits.h>

#include "uthash.h"
#include "utlist.h"

#include "xpl.h"
#include "xpl_gl.h"
#include "xpl_math.h"
#include "xpl_hash.h"
#include "xpl_memory.h"
#include "xpl_vao.h"
#include "xpl_log.h"
#include "xpl_geometry.h"

#define GEOM_TYPE_SLAB      1
#define GEOM_TYPE_ELLIPSOID 2
#define GEOM_TYPE_PLANE     3

static xpl_geometry_t *geometry_table = NULL;
static int detach_id = 0;

static xpl_geometry_t *create_geometry(int instance_id, int create_vao) {
    xpl_geometry_t *geom = xpl_alloc_type(xpl_geometry_t);
    geom->instance_id = instance_id;
    geom->refcount = 1;
    LOG_DEBUG("Caching new geometry instance with id=%d", geom->instance_id);
    HASH_ADD_INT(geometry_table, instance_id, geom);

    if (create_vao) {
        xpl_vao_t *vao = xpl_vao_new();
        geom->vao = vao;
    }

    return geom;
}

static void create_vertex_attribs(xpl_geometry_t *geom) {
    xpl_vao_t *vao = geom->vao;
    xpl_bo_t *vbo = geom->vertices;
    xpl_vao_define_vertex_attrib(vao, "position", vbo, 3, GL_FLOAT, GL_FALSE, 32, 0);
    xpl_vao_define_vertex_attrib(vao, "normal", vbo, 3, GL_FLOAT, GL_FALSE, 32, 3 * sizeof(float));
    xpl_vao_define_vertex_attrib(vao, "uv", vbo, 2, GL_FLOAT, GL_FALSE, 32, 6 * sizeof(float));
}

static void create_slab(xpl_geom_slab_info_t *slab_info, xpl_geometry_t *geom) {
    xpl_vao_t *vao = geom->vao;
    xpl_bo_t *vertices = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_t *indices = xpl_bo_new(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
    float rx = slab_info->width / 2;
    float ry = slab_info->height / 2;
    float rz = slab_info->depth / 2;
    float fv[] = {
        // 0
        rx,     ry,     rz,     0.,     0.,     1.,     1.,     1.,
        // 1
        -rx,    ry,     rz,     0.,     0.,     1.,     0.,     1.,
        // 2
        -rx,    -ry,    rz,     0.,     0.,     1.,     0.,     0.,
        // 3
        rx,     -ry,    rz,     0.,     0.,     1.,     1.,     0.,
        // 4
        rx,     ry,     rz,     1.,     0.,     0.,     1.,     1.,
        // 5
        rx,     -ry,    rz,     1.,     0.,     0.,     0.,     1.,
        // 6
        rx,     -ry,    -rz,    1.,     0.,     0.,     0.,     0.,
        // 7
        rx,     ry,     -rz,    1.,     0.,     0.,     1.,     0.,
        // 8
        rx,     ry,     rz,     0.,     1.,     0.,     1.,     1.,
        // 9
        rx,     ry,     -rz,    0.,     1.,     0.,     1.,     0.,
        // 10
        -rx,    ry,     -rz,    0.,     1.,     0.,     0.,     0.,
        // 11
        -rx,    ry,     rz,     0.,     1.,     0.,     0.,     1.,
        // 12
        -rx,    ry,     rz,     -1.,    0.,     0.,     1.,     1.,
        // 13
        -rx,    ry,     -rz,    -1.,    0.,     0.,     1.,     0.,
        // 14
        -rx,    -ry,    -rz,    -1.,    0.,     0.,     0.,     0.,
        // 15
        -rx,    -ry,    rz,     -1.,    0.,     0.,     0.,     1.,
        // 16
        -rx,    -ry,    -rz,    0.,     -1.,    0.,     0.,     0.,
        // 17
        rx,     -ry,    -rz,    0.,     -1.,    0.,     1.,     0.,
        // 18
        rx,     -ry,    rz,     0.,     -1.,    0.,     1.,     1.,
        // 19
        -rx,    -ry,    rz,     0.,     -1.,    0.,     0.,     1.,
        // 20
        rx,     -ry,    -rz,    0.,     0.,     -1.,    1.,     0.,
        // 21
        -rx,    -ry,    -rz,    0.,     0.,     -1.,    0.,     0.,
        // 22
        -rx,    ry,     -rz,    0.,     0.,     -1.,    0.,     1.,
        // 23
        rx,     ry,     -rz,    0.,     0.,     -1.,    1.,     1.
    };

    GLushort iv[] = {
        0, 1, 2,
        0, 2, 3,

        4, 5, 6,
        4, 6, 7,

        8, 9, 10,
        8, 10, 11,

        12, 13, 14,
        12, 14, 15,

        16, 17, 18,
        16, 18, 19,

        20, 21, 22,
        20, 22, 23
    }; // 6 vertices of 8 floats per face, 6 faces

    xpl_bo_append(vertices, &fv, 8 * 24 * sizeof(float));
    xpl_bo_append(indices, &iv, 6 * 6 * sizeof(GLushort));
    xpl_bo_commit(vertices);
    xpl_bo_commit(indices);

    geom->vertices = vertices;
    xpl_vao_set_index_buffer(vao, 0, indices);
}

static void create_ellipsoid(xpl_geom_ellipsoid_info_t *ellipsoid_info, xpl_geometry_t *geom) {
    assert(ellipsoid_info);
    assert(geom);

    xpl_vao_t *vao = geom->vao;
    xpl_bo_t *vertices = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_t *indices = xpl_bo_new(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

    int lat_slicesi = ellipsoid_info->lat_slices;
    float lat_slicesf = (float)lat_slicesi;

    int long_slicesi = ellipsoid_info->long_slices;
    float long_slicesf = (float)long_slicesi;

    float rx = ellipsoid_info->rx, ry = ellipsoid_info->ry, rz = ellipsoid_info->rz;

    int vertex_length = 8;
    int vertex_count = lat_slicesi * long_slicesi;
    float *vdata = (float *)xpl_alloc(sizeof(float) * vertex_length * vertex_count);
    int vptr = 0;

    for (int lat_slice = 0; lat_slice <= lat_slicesi; lat_slice++) {
        float lat0 = (float)M_PI * (-0.5f + ((float)lat_slice - 1.f) / lat_slicesf);
        float y0 = sinf(lat0);
        float yr0 = cosf(lat0);

        float lat1 = (float)M_PI * (-0.5f + (float)lat_slice / lat_slicesf);
        float y1 = sinf(lat1);
        float yr1 = cosf(lat1);

        for (int long_slice = 0; long_slice <= long_slicesi; long_slice++) {
            float lng = 2.0f * (float)M_PI * ((float)long_slice - 1.f) / long_slicesf;
            float x = cosf(lng);
            float z = sinf(lng);

            vdata[vptr++] = x * yr0 * rx; vdata[vptr++] = y0 * ry;  vdata[vptr++] = z * yr0 * rz;
            vdata[vptr++] = x * yr0;      vdata[vptr++] = y0;       vdata[vptr++] = z * yr0;
            vdata[vptr++] = (float)long_slice / long_slicesf;
            vdata[vptr++] = (float)lat_slice / lat_slicesf;

            vdata[vptr++] = x * yr1 * rx; vdata[vptr++] = y1 * ry;  vdata[vptr++] = z * yr1 * rz;
            vdata[vptr++] = x * yr1;      vdata[vptr++] = y1;       vdata[vptr++] = z * yr1;
            vdata[vptr++] = (float)(long_slice + 1) / long_slicesf;
            vdata[vptr++] = (float)(lat_slice + 1) / lat_slicesf;
        }
    }

    int icount = (vertex_count - 2) * 3; // 012, 123, 234, ...
    GLushort *idata = (GLushort *)xpl_alloc(sizeof(GLushort) * icount);
    int iptr = 0;
    for (int i = 0; i < vertex_count; i += 2) { // start vertex
        // do two tris at once to maintain winding
        idata[iptr++] = i + 0;
        idata[iptr++] = i + 1;
        idata[iptr++] = i + 2;

        idata[iptr++] = i + 1;
        idata[iptr++] = i + 3;
        idata[iptr++] = i + 2;
    }

    xpl_bo_append(vertices, vdata, vptr * sizeof(float));
    xpl_bo_append(indices, idata, iptr * sizeof(GLushort));
    xpl_bo_commit(vertices);
    xpl_bo_commit(indices);

	xpl_free(vdata);
	xpl_free(idata);

    geom->vertices = vertices;
    xpl_vao_set_index_buffer(vao, 0, indices);
}

static void create_plane(xpl_geom_plane_info_t *plane_info, xpl_geometry_t *geom) {
    assert(plane_info);
    assert(geom);

    xpl_vao_t *vao = geom->vao;
    xpl_bo_t *vertices = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_t *indices = xpl_bo_new(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

    float sx = plane_info->sx;
    float sz = plane_info->sz;

    float vertex_data[] = {
        0., 0., 0., 0., 1., 0., 0., 0.,
        sx, 0., 0., 0., 1., 0., 1., 0.,
        0., 0., sz, 0., 1., 0., 0., 1.,
        sx, 0., sz, 0., 1., 0., 1., 1.,
    };
    GLushort index_data[] = {
        0, 1, 2,
        1, 3, 2
    };
    xpl_bo_append(vertices, vertex_data, 32 * sizeof(float));
    xpl_bo_append(indices, index_data, 6 * sizeof(GLushort));
    xpl_bo_commit(vertices);
    xpl_bo_commit(indices);

    geom->vertices = vertices;
    xpl_vao_set_index_buffer(vao, 0, indices);
}

void xpl_geometry_release(xpl_geometry_t **ppgeom) {
    xpl_geometry_t *geom = *ppgeom;
    geom->refcount--;

    LOG_DEBUG("Geometry id %d has %d remaining references", geom->instance_id, geom->refcount);

    if (geom->refcount == 0) {
        LOG_DEBUG("Destroying VAO and geometry table instance for id %d", geom->instance_id);

        // Find all the distinct buffers used by the vertex attributes.
        xpl_vertex_attrib_t *vattrib;
        xpl_bo_t **distinct_bos = (xpl_bo_t **)xpl_alloc(sizeof(xpl_bo_t *) * geom->vao->vertex_attrib_count);
        int distinct_bo_count = 0;
        DL_FOREACH(geom->vao->vertex_attribs_list, vattrib) {
            xpl_bo_t *find_bo = vattrib->vbo_source;
            int found = FALSE;
            // Is it already in our array of distinct BOs?
            for (int i = 0; i < distinct_bo_count; i++) {
                if (distinct_bos[i] == find_bo) {
                    // Yes. Set a flag and stop looking.
                    found = TRUE;
                    break;
                }
            }
            // So did we find it?
            if (! found) {
                // Nope. Add it.
                distinct_bos[distinct_bo_count++] = find_bo;
            }
        }
        // Free each distinct BO.
        for (int i = 0; i < distinct_bo_count; i++) {
            xpl_bo_destroy(&distinct_bos[i]);
        }

        for (int i = 0; i < DRAW_ARRAYS_MAX; i++) {
            if (geom->vao->index_bos[i]) {
                xpl_bo_destroy(& geom->vao->index_bos[i]);
            }
        }

        xpl_vao_destroy(&geom->vao);
        HASH_DEL(geometry_table, geom);
        xpl_free(geom);
    }

    ppgeom = NULL;
}

xpl_geometry_t *xpl_geometry_fork(const xpl_geometry_t *geom) {
    int instance_id = geom->instance_id;
    instance_id = xpl_hashi(++detach_id, instance_id);
    xpl_geometry_t *new_geom = create_geometry(instance_id, FALSE);
    new_geom->vao = xpl_vao_clone(geom->vao);
    new_geom->instance_id = instance_id;
    return new_geom;
}


xpl_geometry_t *xpl_geometry_get_slab(xpl_geom_slab_info_t *slab_info) {
    int hash = xpl_hashi(GEOM_TYPE_SLAB, XPL_HASH_INIT);
    hash = xpl_hashf(slab_info->width, hash);
    hash = xpl_hashf(slab_info->height, hash);
    hash = xpl_hashf(slab_info->depth, hash);

    xpl_geometry_t *geometry;
    HASH_FIND_INT(geometry_table, &hash, geometry);
    if (geometry != NULL) {
        geometry->refcount++;
        return geometry;
    }

    geometry = create_geometry(hash, TRUE);
    create_slab(slab_info, geometry);
    create_vertex_attribs(geometry);
    return geometry;
}

xpl_geometry_t *xpl_geometry_get_ellipsoid(xpl_geom_ellipsoid_info_t *ellipsoid_info) {
    int hash = xpl_hashi(GEOM_TYPE_ELLIPSOID, XPL_HASH_INIT);
    hash = xpl_hashf(ellipsoid_info->rx, hash);
    hash = xpl_hashf(ellipsoid_info->ry, hash);
    hash = xpl_hashf(ellipsoid_info->rz, hash);
    hash = xpl_hashi(ellipsoid_info->lat_slices, hash);
    hash = xpl_hashi(ellipsoid_info->long_slices, hash);

    xpl_geometry_t *geometry;
    HASH_FIND_INT(geometry_table, &hash, geometry);
    if (geometry != NULL) {
        geometry->refcount++;
        return geometry;
    }

    geometry = create_geometry(hash, TRUE);
    create_ellipsoid(ellipsoid_info, geometry);
    create_vertex_attribs(geometry);
    return geometry;
}

xpl_geometry_t *xpl_geometry_get_plane(xpl_geom_plane_info_t *plane_info) {
    int hash = xpl_hashi(GEOM_TYPE_PLANE, XPL_HASH_INIT);
    hash = xpl_hashf(plane_info->sx, hash);
    hash = xpl_hashf(plane_info->sz, hash);

    xpl_geometry_t *geometry;
    HASH_FIND_INT(geometry_table, &hash, geometry);
    if (geometry != NULL) {
        geometry->refcount++;
        return geometry;
    }

    geometry = create_geometry(hash, TRUE);
    create_plane(plane_info, geometry);
    create_vertex_attribs(geometry);
    return geometry;
}
