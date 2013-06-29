//
//  context_snapshot.c
//
//	Stub context for rendering the Informi logo when iOS is taking an app
//	screenshot.
//
//  app
//
//  Created by Justin Bowes on 2013-06-28.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "context/context_snapshot.h"

#include "models/informi_brick.h"
#include "models/plane-xy.h"

#include <xpl_vao_shader.h>

static xpl_vao_t    *brick_vao;
static xpl_bo_t     *brick_vbo;
static xpl_shader_t *brick_shader;
static GLsizei      brick_elements;

static xpl_vao_t    *quad_vao;
static xpl_bo_t     *quad_vbo;
static xpl_shader_t *quad_shader;
static GLsizei      quad_elements;

static xmat4        lookat_vp;

static struct {
    xvec3 position;
    xvec4 color;
} brick[3];

static void *init(xpl_context_t *self) {
	glEnable(GL_BLEND);

	GL_DEBUG();

    brick_vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_append(brick_vbo, &brick_vertex_data, sizeof(brick_vertex_data));
    xpl_bo_commit(brick_vbo);
    brick_elements = sizeof(brick_vertex_data) / sizeof(brick_vertex_data[0]);
    brick_vao = xpl_vao_new();
    xpl_vao_define_vertex_attrib(brick_vao, "position", brick_vbo,
                                 3, GL_FLOAT, GL_FALSE,
                                 sizeof(brick_vertex_data[0]), offsetof(vertex_normal_t, vertex));
	
	GL_DEBUG();

    
    // Quad for effects
    quad_vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_append(quad_vbo, &plane_xy_vertex_data[0], sizeof(plane_xy_vertex_data));
    xpl_bo_commit(quad_vbo);
    quad_elements = sizeof(plane_xy_vertex_data) / sizeof(plane_xy_vertex_data[0]);
    quad_vao = xpl_vao_new();
    xpl_vao_define_vertex_attrib(quad_vao, "position", quad_vbo,
                                 3, GL_FLOAT, GL_FALSE,
                                 sizeof(plane_xy_vertex_data[0]), offsetof(vertex_normal_t, vertex));
	
	GL_DEBUG();

    
    brick_shader = xpl_shader_get_prepared("LogoBackground", "Logo.Vertex", "Logo.Brick.Fragment");
    quad_shader = xpl_shader_get_prepared("LogoForeground", "Overlay.Vertex", "Overlay.Fragment");
	
	glUseProgram(quad_shader->id);
    glUniform1f(xpl_shader_get_uniform(quad_shader, "scanline_amount"), 0.9f);
    glUniform1f(xpl_shader_get_uniform(quad_shader, "strength"), 0.f);
    glUniform4f(xpl_shader_get_uniform(quad_shader, "color"), 0.f, 0.f, 0.f, 1.f);
	glUseProgram(0);
	GL_DEBUG();
//
    
    for (size_t i = 0; i < 3; ++i) {
        brick[i].position = xvec3_set(i % 2, 1.8f * i, 0.f);
		// Dude, use a table.
        brick[i].color = xvec4_set((i + 1) % 3 ? 0.f : 1.f, (i + 2) % 3 ? 0.f : 1.f, i % 3 ? 0.f : 1.f, 0.5f);
    }
	
	return NULL;
}

static void engine(xpl_context_t *self, double time, void *data) {
	xvec3 origin = {{ 6.f, 8.f, 13.3f }};
    xvec3 target = {{ 0.f, 1.f, 0.f }};
    xmat4 lookat_v, lookat_p;
    xmat4_look_at(&origin, &target, &xvec3_y_axis, &lookat_v);
    xmat4_perspective(30.f, 16.f/9.f, 0.1f, 100.f, &lookat_p);
    xmat4_multiply(&lookat_p, &lookat_v, &lookat_vp);
}

static void render(xpl_context_t *self, double time, void *data) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(brick_shader->id);
    glBlendFunc(GL_ONE, GL_ONE);
    GL_DEBUG();
    for (size_t i = 0; i < 3; ++i) {
        xmat4 m, mvp;
        xmat4_identity(&m);
        xmat4_translate(&m, &brick[i].position, NULL);
        xmat4_multiply(&lookat_vp, &m, &mvp);
        glUniform4fv(xpl_shader_get_uniform(brick_shader, "color"), 1, brick[i].color.data);
        glUniformMatrix4fv(xpl_shader_get_uniform(brick_shader, "mvp"), 1, GL_FALSE, mvp.data);
        GL_DEBUG();
        xpl_vao_program_draw_arrays(brick_vao, brick_shader, GL_TRIANGLES, 0, brick_elements);
    }
    glUseProgram(quad_shader->id);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	xpl_vao_program_draw_arrays(quad_vao, quad_shader, GL_TRIANGLES, 0, (GLsizei)quad_elements);
    glUseProgram(GL_NONE);
}


static void destroy(xpl_context_t *self, void *vdata) {
	xpl_vao_destroy(&brick_vao);
	xpl_bo_destroy(&brick_vbo);
	xpl_shader_release(&brick_shader);
	xpl_vao_destroy(&quad_vao);
	xpl_bo_destroy(&quad_vbo);
	xpl_shader_release(&quad_shader);
}

static xpl_context_t *handoff(xpl_context_t *self, void *vdata) {
	return NULL;
}

xpl_context_def_t snapshot_context_def = {
    init,
    engine,
    render,
    destroy,
    handoff
};