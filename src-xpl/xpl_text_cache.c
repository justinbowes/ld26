//
//  xpl_font_cache.c
//  p1
//
//  Created by Justin Bowes on 2013-03-18.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "xpl.h"
#include "xpl_text_buffer.h"
#include "xpl_hash.h"

#include "xpl_text_cache.h"


typedef struct _text_table_entry {
	int key;
	int markup_key;
	char *text;
	size_t text_length;
    
    wchar_t *wtext;
    size_t wtext_length;
    
    xpl_cached_text_t *value;
    
	UT_hash_handle hh;
} _text_table_entry_t;

typedef struct _text_table {
	_text_table_entry_t *entries;
} _text_table_t;

struct xpl_text_cache {
	_text_table_t *last_frame;
	_text_table_t *this_frame;
	xpl_font_manager_t *font_manager;
};

// ---------------------------------------------------------------------

XPLINLINE int font_cache_key(const xpl_markup_t *markup) {
	int hash = XPL_HASH_INIT;
	hash = xpl_hashs(markup->family, hash);
	hash = xpl_hashf(markup->size, hash);
	hash = xpl_hashi(markup->bold, hash);
	hash = xpl_hashi(markup->italic, hash);
    hash = xpl_hashi(markup->outline, hash);
	hash = xpl_hashf(markup->foreground_color.r, hash);
	hash = xpl_hashf(markup->foreground_color.g, hash);
	hash = xpl_hashf(markup->foreground_color.b, hash);
	hash = xpl_hashf(markup->foreground_color.a, hash);
	hash = xpl_hashf(markup->background_color.r, hash);
	hash = xpl_hashf(markup->background_color.g, hash);
	hash = xpl_hashf(markup->background_color.b, hash);
	hash = xpl_hashf(markup->background_color.a, hash);
    
	return hash;
}

// ---------------------------------------------------------------------


XPLINLINE int text_cache_key(int markup_key, const char *text) {
	int hash = XPL_HASH_INIT;
	hash = xpl_hashi(markup_key, hash);
	hash = xpl_hashs(text, hash);
	return hash;
}

static _text_table_entry_t *text_table_entry_new(int markup_key,
                                                 const char *text, xpl_text_buffer_t *buffer) {
	_text_table_entry_t *entry = xpl_alloc_type(_text_table_entry_t);
    
	int hash = text_cache_key(markup_key, text);
#    ifndef DISABLE_CACHES
	LOG_DEBUG("New text table entry for %d", hash);
#    endif
    
	entry->key = hash;
	entry->markup_key = markup_key;
    
	entry->text = strdup(text);
	entry->text_length = strlen(text);
    
    // Cache wide representation
    entry->wtext_length = mbstowcs(NULL, entry->text, entry->text_length);
    entry->wtext = xpl_alloc(sizeof(wchar_t) * (entry->wtext_length + 1));
    mbstowcs(entry->wtext, entry->text, entry->wtext_length);
    
    entry->value = xpl_calloc_type(xpl_cached_text_t);
	entry->value->buffer = buffer;
	entry->value->managed_font = NULL;
    
	return entry;
}

static void text_table_entry_destroy(_text_table_entry_t **ppentry) {
	assert(ppentry);
    
	_text_table_entry_t *entry = *ppentry;
	assert(entry);
    
	free(entry->text); // allocated by wcsdup
    xpl_free(entry->wtext);
	xpl_text_buffer_destroy(&entry->value->buffer);
    
	// Managed by font manager
	// xpl_font_destroy(&entry->managed_font);
    
    xpl_free(entry->value);
    
	xpl_free(entry);
    
	*ppentry = NULL;
}

// ---------------------------------------------------------------------

static _text_table_t *text_table_new() {
	_text_table_t *table = xpl_alloc_type(_text_table_t);
	table->entries = NULL;
	return table;
}

static void text_table_destroy(_text_table_t **pptable) {
	assert(pptable);
    
	_text_table_t *table = *pptable;
	assert(table);
    
	_text_table_entry_t *entry, *tmp;
    
	HASH_ITER(hh, table->entries, entry, tmp)
	{
		HASH_DEL(table->entries, entry);
		text_table_entry_destroy(&entry);
	}
    
	xpl_free(table);
    
	*pptable = NULL;
}

// ---------------------------------------------------------------------

xpl_text_cache_t *xpl_text_cache_new() {
	xpl_text_cache_t *cache = xpl_alloc_type(xpl_text_cache_t);
	cache->last_frame = text_table_new();
	cache->this_frame = text_table_new();
	cache->font_manager = xpl_font_manager_new(1024, 1024, 1);
	return cache;
}

void xpl_text_cache_destroy(xpl_text_cache_t **ppcache) {
	assert(ppcache);
    
	xpl_text_cache_t *cache = *ppcache;
	assert(cache);
    
	if (cache->last_frame)
		text_table_destroy(&cache->last_frame);
	if (cache->this_frame)
		text_table_destroy(&cache->this_frame);
    
	xpl_font_manager_destroy(&cache->font_manager);
    
	xpl_free(cache);
    
	*ppcache = NULL;
}

void xpl_text_cache_advance_frame(xpl_text_cache_t *text_cache) {
	_text_table_entry_t *entry, *tmp;
    
	HASH_ITER(hh, text_cache->last_frame->entries, entry, tmp)
	{
		HASH_DEL(text_cache->last_frame->entries, entry);
		text_table_entry_destroy(&entry);
	}
	_text_table_t *swap = text_cache->last_frame;
	text_cache->last_frame = text_cache->this_frame;
	text_cache->this_frame = swap;
    
}

static xpl_cached_text_t * text_cache_create(xpl_text_cache_t *text_cache,
                                          xpl_markup_t *markup,
                                          const char *text) {
	xpl_text_buffer_t *buffer = xpl_text_buffer_shared_font_manager_new(text_cache->font_manager);
	xvec2 position = xvec2_set(0, 0);
	int markup_key = font_cache_key(markup);
    
	_text_table_entry_t *table_entry = text_table_entry_new(markup_key, text,
                                                            buffer);
	HASH_ADD_INT(text_cache->this_frame->entries, key, table_entry);
    
	xpl_text_buffer_add_text(buffer, &position, markup, table_entry->wtext, table_entry->wtext_length);
	xpl_text_buffer_commit(buffer);
    
    table_entry->value->managed_font = xpl_font_manager_get_from_markup(table_entry->value->buffer->font_manager, markup);
	return table_entry->value;
}

xpl_cached_text_t * xpl_text_cache_get(xpl_text_cache_t *text_cache,
                                       xpl_markup_t *markup,
                                       const char *text) {
#    ifdef DISABLE_CACHES
	return NULL;
#    endif
	int markup_key = font_cache_key(markup);
	int text_key = text_cache_key(markup_key, text);
    
	_text_table_entry_t *entry;
	HASH_FIND_INT(text_cache->this_frame->entries, &text_key, entry);
	if (entry) return entry->value;
    
	HASH_FIND_INT(text_cache->last_frame->entries, &text_key, entry);
	if (entry == NULL) return text_cache_create(text_cache, markup, text);
    
	HASH_DEL(text_cache->last_frame->entries, entry);
	HASH_ADD_INT(text_cache->this_frame->entries, key, entry);
	return entry->value;
}