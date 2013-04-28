//
//  xpl_font.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-23.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_font_h
#define xpl_osx_xpl_font_h

#include <wchar.h>

#include <uthash.h>

#include "xpl_markup.h"
#include "xpl_texture_atlas.h"

// awesome

/*
 * Glyph metrics:
 * --------------
 *
 *                       xmin                     xmax
 *                        |                         |
 *                        |<-------- width -------->|
 *                        |                         |
 *              |         +-------------------------+----------------- ymax
 *              |         |    ggggggggg   ggggg    |     ^        ^
 *              |         |   g:::::::::ggg::::g    |     |        |
 *              |         |  g:::::::::::::::::g    |     |        |
 *              |         | g::::::ggggg::::::gg    |     |        |
 *              |         | g:::::g     g:::::g     |     |        |
 *    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
 *              |         | g:::::g     g:::::g     |     |        |
 *              |         | g::::::g    g:::::g     |     |        |
 *              |         | g:::::::ggggg:::::g     |     |        |
 *              |         |  g::::::::::::::::g     |     |      height
 *              |         |   gg::::::::::::::g     |     |        |
 *  baseline ---*---------|---- gggggggg::::::g-----*--------      |
 *            / |         |             g:::::g     |              |
 *     origin   |         | gggggg      g:::::g     |              |
 *              |         | g:::::gg   gg:::::g     |              |
 *              |         |  g::::::ggg:::::::g     |              |
 *              |         |   gg:::::::::::::g      |              |
 *              |         |     ggg::::::ggg        |              |
 *              |         |         gggggg          |              v
 *              |         +-------------------------+----------------- ymin
 *              |                                   |
 *              |------------- advance_x ---------->|
 */

enum xpl_font_outline_type {
	xfo_none = 0,
	xfo_line = 1,
	xfo_inner = 2,
	xfo_outer = 3
} ;

typedef struct xpl_glyph {
	int                     	charcode; // can't be wchar_t which is ushort in windows
	float                       width;
	float                       height;
	int                         offset_x, offset_y;
	float                       advance_x, advance_y;

	float                       tex_s0, tex_t0;
	float                       tex_s1, tex_t1;

	struct xpl_kerning			*kerning_table; // hash

	//	enum xpl_font_outline_type  outline_type;
	//	float                       outline_thickness;

	UT_hash_handle				hh;

} xpl_glyph_t;

typedef struct xpl_font {
	xpl_glyph_t					*glyph_ttable; // hash
	xpl_texture_atlas_t         *manager_atlas;

	char                        *filename;
	char						*name;
	float                       size;
	int                         hinting;
	enum xpl_font_outline_type  outline_type;
	float                       outline_thickness;

	int                         lcd_filtering; // bool
	unsigned char               lcd_weights[5];

	// Default line spacing. Glyphs may exceed this.
	float                       height;

	// Gap between two lines of this font.
	float                       linegap;

	// The vertical distance from the baseline to the top of the highest character.
	float                       ascender;

	// The vertical distance from the baseline to the bottom of the lowest character.
	float                       descender;

	// The distance from the baseline to an underscore.
	float                       underscore_position;

	// The thickness of an underscore.
	float                       underscore_thickness;

} xpl_font_t;



xpl_font_t *xpl_font_new(xpl_texture_atlas_t *atlas, const char *filename, const float size);
void xpl_font_destroy(xpl_font_t **ppfont);

xpl_glyph_t *xpl_font_get_glyph(xpl_font_t *self, wchar_t charcode);
size_t xpl_font_load_glyphs(xpl_font_t *self, const wchar_t *charcodes);
float xpl_font_glyph_get_kerning(const xpl_glyph_t *self, const wchar_t charcode);

struct xpl_markup; // forward declaration
void xpl_font_apply_markup(xpl_font_t *font, const struct xpl_markup *markup);

#endif
