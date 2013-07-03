//
//  hotspots.h
//  app
//
//  Created by Justin Bowes on 2013-06-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef app_hotspots_h
#define app_hotspots_h

#include <uthash.h>

#include "xpl_platform.h"
#include "xpl_irect.h"
#include "xpl_preprocessor_hash.h"
#include "xpl_hash.h"
#include "xpl_input.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define HOTSPOT(x, i) x "_" STR(i)

typedef struct hotspot {
	int key;
	const char *name;
	int index;
	xirect area;
	
	UT_hash_handle hh;
} hotspot_t;

extern hotspot_t *hotspot_table;


XPLINLINE int hotspot_key(const char *name, int index) {
	return xpl_hashi(index, PS_HASH(name));
}

XPLINLINE void hotspot_set(const char *name, int index, xirect area, xivec2 screen_size) {
	hotspot_t *entry = NULL;
	int key = hotspot_key(name, index);
	HASH_FIND_INT(hotspot_table, &key, entry);
	if (entry) {
		entry->area = area;
	} else {
		entry = xpl_calloc_type(hotspot_t);
		entry->key = key;
		entry->name = name;
		entry->index = index;
		entry->area = area;
		HASH_ADD_INT(hotspot_table, key, entry);
	}
	// Remap from center-origin, y-down
	entry->area.y = screen_size.height - entry->area.y - entry->area.height;
}


XPLINLINE bool hotspot_active(const char *name, int index, xivec2 *coord, int *iid) {
	hotspot_t *entry = NULL;
	int key = hotspot_key(name, index);
	HASH_FIND_INT(hotspot_table, &key, entry);
	if (! entry) {
		if (iid) *iid = -1;
		return false;
	}
	
	return (xpl_input_mouse_down_in(entry->area, coord, iid));
}


void hotspots_clear();


#endif
