//
//  util.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>

#include "xpl_rand.h"

#include "game/util.h"


const char *random_word(const char *key_prefix) {
	int count = 0;
	char key[256];
	while (1) {
		snprintf(key, 256, "%s_%d", key_prefix, count);
		if (! xl_exists(key)) break;
		count++;
	}
	
	if (count == 0) {
		LOG_WARN("No random word keys for %s_#", key_prefix);
		return key_prefix;
	}
	
	int select = xpl_irand_range(0, count);
	snprintf(key, 256, "%s_%d", key_prefix, select);
	return xl(key);
}
