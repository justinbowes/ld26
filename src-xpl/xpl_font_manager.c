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
	UT_hash_handle		hh;

} fm_font_entry_t;

static fm_font_entry_t *font_entry_new(const int key, xpl_texture_atlas_t *atlas, const char *filename, const float size) {
	fm_font_entry_t *entry = xpl_alloc_type(fm_font_entry_t);
	entry->key = key;
	entry->font = xpl_font_new(atlas, filename, size);
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

xpl_font_manager_t *xpl_font_manager_new(int surface_width, int surface_height, int depth) {
	assert(depth == 3 || depth == 1); // FT only renders 1 bit, 8 bit or 24 bit LCD.

	xpl_font_manager_t *self;

	self = xpl_alloc_type(xpl_font_manager_t);
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

// Bold/italic accounted for in filename.

int is_same_fontspec(const char *rname1, const float size1, const char *rname2, const float size2) {
	return ((strcmp(rname1, rname2) == 0) &&
			(size1 == size2));
}

//int is_same_font(xpl_font_t *font1, xpl_font_t *font2) {
//	assert(font1);
//	assert(font2);
//	return is_same_fontspec(font1->filename, font1->size, font2->filename, font2->size);
//}

XPLINLINE int key_for_fontspec(const char *filename, const float size) {
	int hash = XPL_HASH_INIT;
	hash = xpl_hashs(filename, hash);
	hash = xpl_hashf(size, hash);
	return hash;
}

XPLINLINE int key_for_font(xpl_font_t *font) {
	return key_for_fontspec(font->filename, font->size);
}

void xpl_font_manager_delete_font(xpl_font_manager_t *self, xpl_font_t **ppfont) {
	assert(self);

	xpl_font_t *font = *ppfont;
	assert(font);

	int hash_key = key_for_font(font);
	fm_font_entry_t *stored_entry;
	HASH_FIND_INT(self->font_cache, &hash_key, stored_entry);
	assert(stored_entry);

	HASH_DEL(self->font_cache, stored_entry);
	font_entry_destroy(&stored_entry);

	xpl_font_destroy(&font);
	*ppfont = NULL;
}

xpl_font_t *xpl_font_manager_get_from_resource_name(xpl_font_manager_t *self, const char *resource_name, float size) {
	fm_font_entry_t *entry = NULL;
	assert(self);

	// Return matching font if present.
	int key = key_for_fontspec(resource_name, size);
	HASH_FIND_INT(self->font_cache, &key, entry);
	if (entry) {
		return entry->font;
	}

	// Not found. Create new.
	entry = font_entry_new(key, self->atlas, resource_name, size);
	if (! entry->font) {
		LOG_ERROR("Couldn't create font %s %f", resource_name, size);
		return NULL;
	}
	HASH_ADD_INT(self->font_cache, key, entry);
	xpl_font_load_glyphs(entry->font, self->wchar_cache);
	return entry->font;
}

xpl_font_t *xpl_font_manager_get_from_description(xpl_font_manager_t *self, const char *family, const float size, const int bold, const int italic) {
	assert(self);

	char font_name[FONT_NAME_MAX];
	char *bold_s = bold ? "Bold" : "";
	char *italic_s = italic ? "Italic" : "";

	sprintf(font_name, "%s%s%s", family, bold_s, italic_s);

	return xpl_font_manager_get_from_resource_name(self, font_name, size);
}

xpl_font_t *xpl_font_manager_get_from_markup(xpl_font_manager_t *self, const xpl_markup_t *markup) {
	assert(self);
	assert(markup);

	xpl_font_t *result;

	if (! (result = xpl_font_manager_get_from_description(self, markup->family, markup->size, markup->bold, markup->italic))) {
		LOG_ERROR("Couldn't get font from markup: [%s] bold=%d italic=%d (%f)", markup->family, markup->bold, markup->italic, markup->size);
        assert(0);
	}

	xpl_font_apply_markup(result, markup);

	return result;
}



