//
//  hotspots.c
//  app
//
//  Created by Justin Bowes on 2013-06-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "hotspots.h"

hotspot_t								*hotspot_table = NULL;

void hotspots_clear() {
	hotspot_t *el, *tmp;
	HASH_ITER(hh, hotspot_table, el, tmp) {
		HASH_DEL(hotspot_table, el);
		xpl_free(el);
	}
}
