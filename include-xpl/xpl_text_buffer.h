//
//  xpl_text_buffer.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-24.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_text_buffer_h
#define xpl_osx_xpl_text_buffer_h

#include "xpl.h"
#include "xpl_vec.h"
#include "xpl_vao.h"
#include "xpl_font_manager.h"

#define LCD_FILTERING_ON    4
#define LCD_FILTERING_OFF   1

typedef struct xpl_text_buffer {
	int						shared_font_manager;
	xpl_font_manager_t      *font_manager;

	xvec2                   pen_origin;
	size_t                  line_start; // Index in vbo of line start
	float                   line_ascender;
	float                   line_descender;

	xpl_vao_t               *vao;
	xpl_bo_t				*indices;
	xpl_bo_t                *vertices;
	size_t                  vertex_count;
	int                     index_count;
	xpl_shader_t            *shader;

} xpl_text_buffer_t;

xpl_text_buffer_t *xpl_text_buffer_new(int surface_width, int surface_height, int lcd_filtering_onoff);
xpl_text_buffer_t *xpl_text_buffer_shared_font_manager_new(xpl_font_manager_t *font_manager);
void xpl_text_buffer_destroy(xpl_text_buffer_t **ppbuffer);

int xpl_text_buffer_add_text(xpl_text_buffer_t *self, xvec2 *pen, xpl_markup_t *markup, const wchar_t *text, size_t length);
int xpl_text_buffer_add_wchar(xpl_text_buffer_t *self, xvec2 *pen, xpl_markup_t *markup, wchar_t current, wchar_t previous);

// Add pairs of markup and wchar. Terminate with null.
void xpl_text_buffer_add_markup_pairs(xpl_text_buffer_t *self, xvec2 *pen, ...);
void xpl_text_buffer_clear(xpl_text_buffer_t *self);

// Commit changes to the buffer to OpenGL.
void xpl_text_buffer_commit(const xpl_text_buffer_t *self);
void xpl_text_buffer_render(const xpl_text_buffer_t *self, const GLfloat *mvp);
void xpl_text_buffer_render_tinted(const xpl_text_buffer_t *self, const GLfloat *mvp, const xvec4 color);
void xpl_text_buffer_render_no_setup(const xpl_text_buffer_t *self, const GLfloat *mvp);

#endif
