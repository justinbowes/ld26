//
//  xpl_markup.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-25.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "xpl.h"
#include "xpl_memory.h"
#include "xpl_markup.h"

xpl_markup_t *xpl_markup_new() {
	xpl_markup_t *markup = xpl_alloc_type(xpl_markup_t);
	xpl_markup_clear(markup);
	return markup;
}

void xpl_markup_destroy(xpl_markup_t **ppmarkup) {
	assert(ppmarkup);

	xpl_markup_t *markup = *ppmarkup;
	assert(markup);

	// The font is destroyed via the font manager, not here.
	//if (markup->font) xpl_font_destroy(&markup->font);

	xpl_free(markup);
	*ppmarkup = NULL;
}

void xpl_markup_clear(xpl_markup_t *markup) {
	markup->family[0] = '\0';
	markup->size = 0.0f;
	markup->bold = FALSE;
	markup->italic = FALSE;

	markup->foreground_color = xvec4_set(1.0f, 1.0f, 1.0f, 1.0f);
	markup->background_color = xvec4_set(0.0f, 0.0f, 0.0f, 0.0f);

	markup->rise = 0.0f;
	markup->spacing = 0.0f;
	markup->gamma = 1.0f;

	markup->outline = FALSE;
    markup->outline_thickness = 16.f;
    
	markup->underline = FALSE;
	markup->underline_color = xvec4_set(1.0f, 1.0f, 1.0f, 1.0f);

	markup->overline = FALSE;
	markup->overline_color = xvec4_set(1.0f, 1.0f, 1.0f, 1.0f);

	markup->strikethrough = FALSE;
	markup->strikethrough_color = xvec4_set(1.0f, 1.0f, 1.0f, 1.0f);

	markup->insane = '\0';

	markup->font = NULL;
}

void xpl_markup_set(xpl_markup_t *markup, const char *family, float size, int bold, int italic, xvec4 foreground_color, xvec4 background_color) {
	strncpy(markup->family, family, XPL_MARKUP_FAMILY_NAME_MAX);
	markup->size = size;
	markup->bold = !!bold;
	markup->italic = !!italic;
	markup->foreground_color = foreground_color;
	markup->background_color = background_color;
	markup->gamma = 1.0f;
}


