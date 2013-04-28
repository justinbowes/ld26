//
//  xpl_va.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-10-09.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_va_h
#define xpl_osx_xpl_va_h

#include <stdbool.h>

#include <uthash.h>

#include "xpl_bo.h"

#define VATTR_NAME_MAX      64
#define UNIFORM_NAME_MAX    256
#define SHADER_NAME_MAX     256
#define DRAW_ARRAYS_MAX     64


typedef struct xpl_vertex_attrib {
	char                        name[VATTR_NAME_MAX];
	xpl_bo_t                    *vbo_source;
	int                         enabled;

	GLint                       size;
	GLenum                      type;
	GLboolean                   normalize;
	GLsizei                     stride;
	GLsizei                     offset;

	// we'll hash by name
	UT_hash_handle              hh;
	// and do a DL list for fast prepend and traversal
	struct xpl_vertex_attrib    *prev;
	struct xpl_vertex_attrib    *next;
} xpl_vertex_attrib_t;

typedef struct xpl_vao {
	GLuint                      vao_id;

	xpl_vertex_attrib_t         *vertex_attribs_hash;
	xpl_vertex_attrib_t         *vertex_attribs_list;
	size_t                      vertex_attrib_count;

	GLboolean                   do_teardown;

	xpl_bo_t                    * index_bos[DRAW_ARRAYS_MAX];
} xpl_vao_t;

typedef struct xpl_shader {
	GLuint						id;
    bool                        linked;
	char						name[SHADER_NAME_MAX];
    const char*                 frag_data_locations[8];

	struct xpl_shader_node		*shader_program_nodes; // utlist
	struct xpl_uniform_info		*uniform_table;
	struct xpl_shader_vao_info	*vao_table;
} xpl_shader_t;

#endif
