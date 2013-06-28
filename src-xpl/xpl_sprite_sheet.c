//
//  xpl_sprite_sheet.c
//  app
//
//  Created by Justin Bowes on 2013-06-28.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "xpl_sprite_sheet.h"
#include "xpl_sprite.h"
#include "xpl_file.h"
#include "xpl_dynamic_buffer.h"
#include "uthash.h"
#include "cJSON/cJSON.h"

struct xpl_sprite_sheet_entry {
	char							*name;
    struct xpl_sprite               *sprite;
    UT_hash_handle                  hh;
};

struct xpl_sprite_sheet {
    char							*resource;
    xpl_sprite_sheet_entry_t        *entries;
	xivec2							size;
};

xpl_sprite_sheet_t *xpl_sprite_sheet_new(struct xpl_sprite_batch *batch, const char *json) {
	xpl_sprite_sheet_t *sheet = xpl_calloc_type(xpl_sprite_sheet_t);
	
	char resource[PATH_MAX];
	xpl_resolve_resource(resource, json, PATH_MAX);
	xpl_dynamic_buffer_t *file = xpl_dynamic_buffer_new();
	xpl_file_get_contents(resource, file);
	const char *contents = (const char *)file->content;
	
	cJSON *root = cJSON_Parse(contents);
	cJSON *meta = cJSON_GetObjectItem(root, "meta");
	
	const char *sheet_source = cJSON_GetObjectItem(meta, "image")->valuestring;

	cJSON *meta_size = cJSON_GetObjectItem(meta, "size");
	sheet->size.x = cJSON_GetObjectItem(meta_size, "w")->valueint;
	sheet->size.y = cJSON_GetObjectItem(meta_size, "h")->valueint;
	
	cJSON *frames = cJSON_GetObjectItem(root, "frames");
	size_t entry_count = cJSON_GetArraySize(frames);
	for (size_t i = 0; i < entry_count; ++i) {
		cJSON *el = cJSON_GetArrayItem(frames, (int)i);
		
		xpl_sprite_sheet_entry_t *entry = xpl_calloc_type(xpl_sprite_sheet_entry_t);
		entry->name = strdup(el->string);
		
		cJSON *el_frame = cJSON_GetObjectItem(el, "frame");
		xirect region = xirect_set(cJSON_GetObjectItem(el_frame, "x")->valueint,
								   cJSON_GetObjectItem(el_frame, "y")->valueint,
								   cJSON_GetObjectItem(el_frame, "w")->valueint,
								   cJSON_GetObjectItem(el_frame, "h")->valueint);
		// invert y. of course
		region.y = sheet->size.y - region.y - region.height;
		entry->sprite = xpl_sprite_new(batch, sheet_source, &region);
		
		HASH_ADD_KEYPTR(hh, sheet->entries, entry->name, strlen(entry->name), entry);
	}
	
	xpl_dynamic_buffer_destroy(&file);
	cJSON_Delete(root);
	
	
	return sheet;
}

struct xpl_sprite *xpl_sprite_get(struct xpl_sprite_sheet *sheet, const char *name) {
    xpl_sprite_sheet_entry_t *entry;
    HASH_FIND_STR(sheet->entries, name, entry);
    if (entry) return entry->sprite;
    return NULL;
}

void xpl_sprite_sheet_destroy(xpl_sprite_sheet_t **ppsheet) {
    assert(ppsheet);
    xpl_sprite_sheet_t *sheet = *ppsheet;
    assert(sheet);
    
    xpl_sprite_sheet_entry_t *el, *tmp;
    HASH_ITER(hh, sheet->entries, el, tmp) {
        HASH_DEL(sheet->entries, el);
		free(el->name);
		
		// I think the batch owns the sprite actually.
		xpl_sprite_destroy(&el->sprite);
		
        xpl_free(el);
    }
	free(sheet->resource);
    
    xpl_free(sheet);
    *ppsheet = NULL;
}

