//
//  xpl_font_manager.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-23.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <utlist.h>

#include "xpl.h"
#include "xpl_hash.h"
#include "xpl_markup.h"
#include "xpl_memory.h"
#include "xpl_platform.h"
#include "xpl_font_manager.h"
#include "xpl_font.h"
#include "uthash.h"

#define FONT_NAME_MAX PATH_MAX

typedef struct fm_font_entry {
	int					key;
	xpl_font_t			*font;
	xpl_markup_t		markup;
	UT_hash_handle		hh;

} fm_font_entry_t;

static fm_font_entry_t *font_entry_new(const int key, xpl_texture_atlas_t *atlas, const xpl_markup_t *markup) {
	fm_font_entry_t *entry = xpl_alloc_type(fm_font_entry_t);
	entry->key = key;
	entry->markup = *markup;

	char filename[FONT_NAME_MAX];
	char *bold_s = markup->bold ? "Bold" : "";
	char *italic_s = markup->italic ? "Italic" : "";
	sprintf(filename, "%s%s%s", markup->family, bold_s, italic_s);

	entry->font = xpl_font_new(atlas, filename, markup->size);
	return entry;
}

void font_entry_destroy(fm_font_entry_t **ppentry) {
	assert(ppentry);
	fm_font_entry_t *entry = *ppentry;
	assert(entry);

	xpl_font_destroy(&entry->font);
	xpl_free(entry);

	*ppentry = NULL;
}

xpl_font_manager_t *xpl_font_manager_new(size_t surface_width, size_t surface_height, size_t depth) {
	assert(depth == 3 || depth == 1); // FT only renders 1 bit, 8 bit or 24 bit LCD.

	xpl_font_manager_t *self;

	self = xpl_alloc_type(xpl_font_manager_t);
#ifdef XPL_GLES
	depth = 1;
#endif
	self->atlas = xpl_texture_atlas_new(surface_width, surface_height, depth);
	self->font_cache = NULL;
	self->wchar_cache = wcsdup(L" ");

	return self;
}

void xpl_font_manager_destroy(xpl_font_manager_t **ppmgr) {
	xpl_font_manager_t *mgr = *ppmgr;

	assert(mgr);

	free(mgr->wchar_cache); // wcsdup

	fm_font_entry_t *elem, *tmp;

	HASH_ITER(hh, mgr->font_cache, elem, tmp) {
		HASH_DEL(mgr->font_cache, elem);
		font_entry_destroy(&elem);
	}

	xpl_texture_atlas_destroy(&mgr->atlas);

	xpl_free(mgr);
	mgr = NULL;
}

void xpl_font_manager_delete_font(xpl_font_manager_t *self, xpl_font_t **ppfont) {
	assert(self);

	xpl_font_t *font = *ppfont;
	assert(font);

	fm_font_entry_t *stored_entry, *tmp;
	HASH_ITER(hh, self->font_cache, stored_entry, tmp) {
		if (stored_entry->font == font) {
			HASH_DEL(self->font_cache, stored_entry);
			font_entry_destroy(&stored_entry);
			xpl_font_destroy(&font);
			*ppfont = NULL;
			return;
		}
	}
	
	LOG_WARN("Didn't find font to destroy");
}

xpl_font_t *xpl_font_manager_get_from_markup(xpl_font_manager_t *self, xpl_markup_t *markup) {
	assert(self);
	assert(markup);

	fm_font_entry_t *entry = NULL;
	// Return matching font if present.
	int key = xpl_markup_hash(markup);
	HASH_FIND_INT(self->font_cache, &key, entry);
	if (! entry) {
		entry = font_entry_new(key, self->atlas, markup);
		if (! entry->font) {
			LOG_ERROR("Couldn't get font from markup: [%s] bold=%d italic=%d (%f)", markup->family, markup->bold, markup->italic, markup->size);
			assert(0);
		}
		HASH_ADD_INT(self->font_cache, key, entry);
	}
	
	// If we have a cached font for this hash and it doesn't match the markup
	// overwrite what's in the markup. Either we don't own the font (markup
	// used in multiple managers) or the markup was mutated.
	if (markup->font != entry->font) markup->font = entry->font;
	xpl_font_apply_markup(markup->font, markup);

	return markup->font;
}



