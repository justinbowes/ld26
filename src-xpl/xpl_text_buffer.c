//
//  xpl_text_buffer.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-24.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "xpl_gl.h"
#include "xpl_bo.h"
#include "xpl_memory.h"
#include "xpl_rc.h"
#include "xpl_shader.h"
#include "xpl_vao.h"
#include "xpl_vec.h"

#include "xpl_text_buffer.h"

typedef struct glyph_vertex {
	float x;
	float y;
	float z;

	float u;
	float v;

	float r;
	float g;
	float b;
	float a;

	float shift;
	float gamma;

	char padding[4]; // 48 bytes
} glyph_vertex_t;

xpl_text_buffer_t *xpl_text_buffer_new(int surface_width, int surface_height, int lcd_filtering_onoff) {
	int buffer_depth_bytes = lcd_filtering_onoff; // Yes, this is monstrous
	xpl_text_buffer_t *result = xpl_text_buffer_shared_font_manager_new(xpl_font_manager_new(surface_width, surface_height, buffer_depth_bytes));
	result->shared_font_manager = FALSE;
	return result;
}

xpl_text_buffer_t *xpl_text_buffer_shared_font_manager_new(xpl_font_manager_t *font_manager) {
	xpl_text_buffer_t *self = xpl_alloc_type(xpl_text_buffer_t);

	self->vao = xpl_vao_new();

	xpl_bo_t *vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	GLsizei stride = sizeof(glyph_vertex_t); // padding maybe?
	xpl_vao_define_vertex_attrib(self->vao, "position", vbo, 3, GL_FLOAT, GL_FALSE, stride, 0);
	xpl_vao_define_vertex_attrib(self->vao, "uv", vbo, 2, GL_FLOAT, GL_FALSE, stride, 12);
	xpl_vao_define_vertex_attrib(self->vao, "color", vbo, 4, GL_FLOAT, GL_FALSE, stride, 20);
	xpl_vao_define_vertex_attrib(self->vao, "shift", vbo, 1, GL_FLOAT, GL_FALSE, stride, 36);
	xpl_vao_define_vertex_attrib(self->vao, "gamma", vbo, 1, GL_FLOAT, GL_FALSE, stride, 40);
	self->vertices = vbo;
	self->vertex_count = 0;

	self->indices = xpl_bo_new(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
	xpl_vao_set_index_buffer(self->vao, 0, self->indices);
	self->index_count = 0;

	xpl_shader_t *shader = xpl_shader_get("Text");
	if (! shader->linked) {
		xpl_shader_add(shader, GL_VERTEX_SHADER, "Text.Vertex");
		xpl_shader_add(shader, GL_FRAGMENT_SHADER, "Text.Fragment");
		xpl_shader_link(shader);
	}
	self->shader = shader;

	self->line_start = 0;
	self->line_ascender = 0.0f;
	self->line_descender = 0.0f;

	self->font_manager = font_manager;
	self->shared_font_manager = TRUE;

	return self;
}

void xpl_text_buffer_destroy(xpl_text_buffer_t **ppbuffer) {
	assert(ppbuffer);
	xpl_text_buffer_t *buffer = *ppbuffer;
	assert(buffer);

	if (! buffer->shared_font_manager) xpl_font_manager_destroy(&buffer->font_manager);

	// The VAO doesn't own the VBOs associated with its vertex attributes.
	// Before we nuke the VAO, get the vbo from the vertex attrib and destroy that.
	// They all use the same VBO, so...we didn't cache it.
	xpl_vertex_attrib_t *vattrib = xpl_vao_get_vertex_attrib(buffer->vao, "position");
	xpl_bo_destroy(&vattrib->vbo_source);

	// Destroy the IBO we created.
	xpl_bo_destroy(&buffer->indices);

	// Okay, now the VAO is okay to nuke.
	xpl_vao_destroy(&buffer->vao);

	// The shader may or may not go, depending on the refcount.
	xpl_shader_release(&buffer->shader);

	// I forgot to add this for a long time. Funny, eh?
	xpl_free(buffer);

	*ppbuffer = NULL;
}

int xpl_text_buffer_add_text(xpl_text_buffer_t *self, xvec2 *pen, xpl_markup_t *markup, const wchar_t *text, size_t length) {
	assert(self);

	xpl_font_manager_t *manager = self->font_manager;
	if (! markup) {
		LOG_WARN("No markup provided");
		return FALSE;
	}

	// The markup font field, if set, may not match the markup if the user changed the markup.
	// We either need to compare the font or just reset it, and comparing it is most of the work
	// of resetting it. So we reset it.
	markup->font = xpl_font_manager_get_from_markup(manager, markup);
	if (! markup->font) {
		LOG_ERROR("Couldn't get font from manager for markup");
		return FALSE;
	}

	if (length == 0) {
		length = wcslen(text);
	}

	if (!(text && text[0])) return TRUE;

	if (self->vertex_count == 0) {
		self->pen_origin = *pen;
	} else {
        pen->y -= markup->font->ascender;
    }
    
	// Preload the glyphs
	xpl_font_load_glyphs(markup->font, text);

	// First character has no previous (for kerning)
	xpl_text_buffer_add_wchar(self, pen, markup, text[0], 0);
	for (size_t i = 1; i < length; i++) {
		xpl_text_buffer_add_wchar(self, pen, markup, text[i], text[i - 1]);
	}

	return TRUE;
}

static void text_buffer_move_last_line(xpl_text_buffer_t *self, float dy) {
	// This is the index of the first vertex in the line.

	// This is the first vertex in the line.
	glyph_vertex_t *vtx = (glyph_vertex_t *)self->vertices->client_data->content;
	vtx += self->line_start;

	for (size_t shift_index = self->line_start; shift_index < self->vertex_count; shift_index++, vtx++) {
		vtx->y -= dy;
	}
}

#define SET_GLYPH_VERTEX(value,x0,y0,z0,s0,t0,r,g,b,a,sh,gm) {                                \
		(value).x=((float)x0); (value).y=((float)y0); (value).z=((float)z0);                      \
		(value).u=((float)s0); (value).v=((float)t0);                                             \
		(value).r=((float)r);  (value).g=((float)g);  (value).b=((float)b); (value).a=((float)a); \
		(value).shift=((float)sh); (value).gamma=((float)gm);    }

int xpl_text_buffer_add_wchar(xpl_text_buffer_t *self, xvec2 *pen, xpl_markup_t *markup, wchar_t current, wchar_t previous) {
	assert(self);
	assert(pen);
	assert(markup);
	assert(current);

	// indices moved forward by this because all share the same vbo
	size_t voffset = self->vertex_count;
	size_t vcount = 0;
	size_t icount = 0;

	xpl_font_t *font = markup->font;
	float gamma = markup->gamma;

	// Maximum number of vertices is 20 (= 5x2 triangles) per glyph:
	//  - 2 triangles for background
	//  - 2 triangles for overline
	//  - 2 triangles for underline
	//  - 2 triangles for strikethrough
	//  - 2 triangles for glyph
	glyph_vertex_t vertices[4 * 5];
	GLushort indices[6 * 5]; // two shared vertices per triangle

	if (current == L'\n') {
		pen->x = self->pen_origin.x;
		if (previous == L'\n') {
			// No line descender for this line, so make something up
			pen->y -= markup->font->height * 1.1f;
		} else {
			pen->y -= self->line_descender + markup->font->height;
		}
		self->line_descender = 0.0f;
		self->line_ascender = 0.0f;
		self->line_start = self->vertex_count;
		return FALSE;
	}

	if (markup->font->ascender > self->line_ascender) {
		float y = pen->y;
        pen->y -= markup->font->ascender - self->line_ascender;
		text_buffer_move_last_line(self, (int)(y - pen->y));
		self->line_ascender = markup->font->ascender;
	}
	if (markup->font->descender < self->line_descender) {
		self->line_descender = xmin(markup->font->descender, self->line_descender);
	}

	xpl_glyph_t *glyph = xpl_font_get_glyph(font, current);
	if (! glyph) {
		LOG_WARN("Glyph %c not found", current);
		return FALSE;
	}
	xpl_glyph_t *empty = xpl_font_get_glyph(font, -1);

	float kerning = 0.0f;
	if (previous > 0) {
		kerning = xpl_font_glyph_get_kerning(glyph, previous);
		pen->x += kerning;
	}

	if (markup->background_color.alpha > 0) {
		float r = markup->background_color.r;
		float g = markup->background_color.g;
		float b = markup->background_color.b;
		float a = markup->background_color.a;
		float x0 = pen->x - kerning;
		float y0 = (float)(int)(pen->y + font->descender);
		float x1 = x0 + glyph->advance_x;
		float y1 = (float)(int)(y0 + font->height + font->linegap);
		float s0 = empty->tex_s0;
		float t0 = empty->tex_t0;
		float s1 = empty->tex_s1;
		float t1 = empty->tex_t1;

		SET_GLYPH_VERTEX(vertices[vcount + 0], (int)x0, y0, 0,  s0, t0,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 1], (int)x0, y1, 0,  s0, t1,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 2], (int)x1, y1, 0,  s1, t1,  r, g, b, a,  x1 - ((int)x1), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 3], (int)x1, y0, 0,  s1, t0,  r, g, b, a,  x1 - ((int)x1), gamma);
		indices[icount + 0] = voffset + vcount + 0;
		indices[icount + 1] = voffset + vcount + 1;
		indices[icount + 2] = voffset + vcount + 2;
		indices[icount + 3] = voffset + vcount + 0;
		indices[icount + 4] = voffset + vcount + 2;
		indices[icount + 5] = voffset + vcount + 3;
		vcount += 4;
		icount += 6;
	}

	if (markup->underline) {
		float r = markup->underline_color.r;
		float g = markup->underline_color.g;
		float b = markup->underline_color.b;
		float a = markup->underline_color.a;
		float x0 = pen->x - kerning;
		float y0 = (float)(int)(pen->y + font->underscore_position);
		float x1 = x0 + glyph->advance_x;
		float y1 = (float)(int)(y0 + font->underscore_thickness);
		float s0 = empty->tex_s0;
		float t0 = empty->tex_t0;
		float s1 = empty->tex_s1;
		float t1 = empty->tex_t1;

		SET_GLYPH_VERTEX(vertices[vcount + 0], (int)x0, y0, 0,  s0, t0,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 1], (int)x0, y1, 0,  s0, t1,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 2], (int)x1, y1, 0,  s1, t1,  r, g, b, a,  x1 - ((int)x1), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 3], (int)x1, y0, 0,  s1, t0,  r, g, b, a,  x1 - ((int)x1), gamma);

		indices[icount + 0] = voffset + vcount + 0;
		indices[icount + 1] = voffset + vcount + 1;
		indices[icount + 2] = voffset + vcount + 2;
		indices[icount + 3] = voffset + vcount + 0;
		indices[icount + 4] = voffset + vcount + 2;
		indices[icount + 5] = voffset + vcount + 3;
		vcount += 4;
		icount += 6;
	}

	if (markup->overline) {
		float r = markup->overline_color.r;
		float g = markup->overline_color.g;
		float b = markup->overline_color.b;
		float a = markup->overline_color.a;
		float x0 = pen->x - kerning;
		float y0 = (float)(int)(pen->y + (int)font->ascender);
		float x1 = x0 + glyph->advance_x;
		float y1 = (float)(int)(y0 + (int)font->underscore_thickness);
		float s0 = empty->tex_s0;
		float t0 = empty->tex_t0;
		float s1 = empty->tex_s1;
		float t1 = empty->tex_t1;

		SET_GLYPH_VERTEX(vertices[vcount + 0], (int)x0, y0, 0,  s0, t0,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 1], (int)x0, y1, 0,  s0, t1,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 2], (int)x1, y1, 0,  s1, t1,  r, g, b, a,  x1 - ((int)x1), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 3], (int)x1, y0, 0,  s1, t0,  r, g, b, a,  x1 - ((int)x1), gamma);
		indices[icount + 0] = voffset + vcount + 0;
		indices[icount + 1] = voffset + vcount + 1;
		indices[icount + 2] = voffset + vcount + 2;
		indices[icount + 3] = voffset + vcount + 0;
		indices[icount + 4] = voffset + vcount + 2;
		indices[icount + 5] = voffset + vcount + 3;
		vcount += 4;
		icount += 6;
	}

	if (markup->strikethrough) {
		float r = markup->strikethrough_color.r;
		float g = markup->strikethrough_color.g;
		float b = markup->strikethrough_color.b;
		float a = markup->strikethrough_color.a;
		float x0 = pen->x - kerning;
		float y0 = (float)(int)(pen->y + (int)(0.33f * font->ascender));
		float x1 = x0 + glyph->advance_x;
		float y1 = (float)(int)(y0 + (int)font->underscore_thickness);
		float s0 = empty->tex_s0;
		float t0 = empty->tex_t0;
		float s1 = empty->tex_s1;
		float t1 = empty->tex_t1;

		SET_GLYPH_VERTEX(vertices[vcount + 0], x0, y0, 0,  s0, t0,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 1], x0, y1, 0,  s0, t1,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 2], x1, y1, 0,  s1, t1,  r, g, b, a,  x1 - ((int)x1), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 3], x1, y0, 0,  s1, t0,  r, g, b, a,  x1 - ((int)x1), gamma);
		indices[icount + 0] = voffset + vcount + 0;
		indices[icount + 1] = voffset + vcount + 1;
		indices[icount + 2] = voffset + vcount + 2;
		indices[icount + 3] = voffset + vcount + 0;
		indices[icount + 4] = voffset + vcount + 2;
		indices[icount + 5] = voffset + vcount + 3;
		vcount += 4;
		icount += 6;
	}

	// HOOORAY, IT'S THE GLYPH!
	// Watch, I'll never use strikeline or overthrough
	{
		float r = markup->foreground_color.r;
		float g = markup->foreground_color.g;
		float b = markup->foreground_color.b;
		float a = markup->foreground_color.a;
		float x0 = pen->x + glyph->offset_x;
		float y0 = (float)(int)(pen->y + glyph->offset_y);
		float x1 = x0 + glyph->width;
		float y1 = (float)(int)(y0 - glyph->height);
		float s0 = glyph->tex_s0;
		float t0 = glyph->tex_t0;
		float s1 = glyph->tex_s1;
		float t1 = glyph->tex_t1;

		SET_GLYPH_VERTEX(vertices[vcount + 0], x0, y0, 0,  s0, t0,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 1], x0, y1, 0,  s0, t1,  r, g, b, a,  x0 - ((int)x0), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 2], x1, y1, 0,  s1, t1,  r, g, b, a,  x1 - ((int)x1), gamma);
		SET_GLYPH_VERTEX(vertices[vcount + 3], x1, y0, 0,  s1, t0,  r, g, b, a,  x1 - ((int)x1), gamma);
		indices[icount + 0] = voffset + vcount + 0;
		indices[icount + 1] = voffset + vcount + 1;
		indices[icount + 2] = voffset + vcount + 2;
		indices[icount + 3] = voffset + vcount + 0;
		indices[icount + 4] = voffset + vcount + 2;
		indices[icount + 5] = voffset + vcount + 3;
		vcount += 4;
		icount += 6;
		LOG_TRACE("Adding character %lc: (%f,%f):(%f,%f)", current, x0, y0, x1, y1);
	}

	xpl_bo_append(self->vertices, vertices, vcount * sizeof (glyph_vertex_t));
	xpl_bo_append(self->vao->index_bos[0], indices, icount * sizeof (GLushort));

#    if 0
	LOG_DEBUG("Index buffer content: ")
	GLushort *val;
	size_t elements = self->vao->index_bos[0]->client_data->length / sizeof (GLushort);
	for (size_t i = 0; i < elements; i++) {
		val = ((GLushort *)self->vao->index_bos[0]->client_data->content) + i;
		LOG_DEBUG("%hu", *val);
	}
#    endif

	self->vertex_count += vcount;
	self->index_count += icount;

	pen->x += glyph->advance_x * (1.0f + markup->spacing);

	return TRUE;
}

// Add pairs of markup and wchar. Terminate with null.

void xpl_text_buffer_add_markup_pairs(xpl_text_buffer_t *self, xvec2 *pen, ...) {
	xpl_markup_t *markup;
	wchar_t *text;

	if (! self->vertex_count) {
		self->pen_origin = *pen;
	}

	va_list args;
	va_start(args, pen);
	do {
		markup = va_arg(args, xpl_markup_t *);
		if (! markup) break; // done;

		if (markup->insane) {
			LOG_ERROR("Terminate markup pairs with NULL, dummy");
			exit(XPL_RC_DUMMY_PROGRAMMER);
		}

		text = va_arg(args, wchar_t *);
		xpl_text_buffer_add_text(self, pen, markup, text, wcslen(text));
	} while (markup);

	va_end(args);
}

void xpl_text_buffer_clear(xpl_text_buffer_t *self) {
	assert(self);

	xpl_bo_clear(self->vertices);
	self->vertex_count = 0;

	xpl_bo_clear(self->vao->index_bos[0]);
	self->index_count = 0;

	self->line_start = 0;
	self->line_ascender = 0.0f;
	self->line_descender = 0.0f;

}

void xpl_text_buffer_commit(const xpl_text_buffer_t *self) {
	xpl_bo_commit(self->vertices);
	xpl_bo_commit(self->vao->index_bos[0]);
}

void xpl_text_buffer_render(const xpl_text_buffer_t *self, const GLfloat *mvp) {
	// lazy atlas
	if (! self->font_manager->atlas->texture_id) return;
	
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);

	glDepthMask(GL_FALSE);
	glUseProgram(self->shader->id);

//	if (self->font_manager->atlas->depth == 1) {
//		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
//	} else {
	// premultiplied alpha. Premultiply your alpha.
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	//	}
#ifndef XPL_PLATFORM_IOS
	glUniform3f(xpl_shader_get_uniform(self->shader, "subpixel"),
                1.0f / self->font_manager->atlas->width,
                1.0f / self->font_manager->atlas->height,
                self->font_manager->atlas->depth);
#endif

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, self->font_manager->atlas->texture_id);
	glUniform1i(xpl_shader_get_uniform(self->shader, "tex"), 0); // Use texture unit zero.
	glUniformMatrix4fv(xpl_shader_get_uniform(self->shader, "mvp"), 1, GL_FALSE, mvp);

	xpl_vao_program_draw_elements(self->vao, self->shader, GL_TRIANGLES, 0);

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
	glUseProgram(GL_NONE);
	glDepthMask(GL_TRUE);
    
    GL_DEBUG();
}

void xpl_text_buffer_render_no_setup(const xpl_text_buffer_t *self, const GLfloat *mvp) {
	glUniformMatrix4fv(xpl_shader_get_uniform(self->shader, "mvp"), 1, GL_FALSE, mvp);
	xpl_vao_program_draw_elements(self->vao, self->shader, GL_TRIANGLES, 0);
}
