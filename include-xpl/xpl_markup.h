//
//  xpl_markup.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-25.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_markup_h
#define xpl_osx_xpl_markup_h

#include "xpl_vec.h"
#include "xpl_font.h"

#define XPL_MARKUP_FAMILY_NAME_MAX 128

struct xpl_font;

typedef struct xpl_markup {

	char            family[XPL_MARKUP_FAMILY_NAME_MAX];
	float           size;
	int             bold;
	int             italic;

	xvec4           foreground_color;
	xvec4           background_color;

	float           rise;               // vertical rise from baseline
	float           spacing;            // spacing multiplier between letters (0.0 = 100%)
	float           gamma;              // gamma correction

	int             outline;
    float           outline_thickness;

	int             underline;
	xvec4           underline_color;

	int             overline;
	xvec4           overline_color;

	int             strikethrough;
	xvec4           strikethrough_color;

	char            insane;

	struct xpl_font *font;

} xpl_markup_t;

extern xpl_markup_t default_markup;

xpl_markup_t *xpl_markup_new(void);
void xpl_markup_destroy(xpl_markup_t **ppmarkup);

void xpl_markup_clear(xpl_markup_t *markup);
void xpl_markup_set(xpl_markup_t *markup,
                    const char *family, float size, int bold, int italic,
                    xvec4 foreground_color, xvec4 background_color);

#endif
