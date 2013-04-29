//
//  prefs.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_prefs_h
#define ld26_prefs_h

#include <stdbool.h>

#include "game/game.h"

typedef struct prefs {
	bool skip_tutorial;
	char name[NAME_SIZE];
	char server[SERVER_SIZE];
	int port;
} prefs_t;

void prefs_reset(void);
prefs_t prefs_get(void);
void prefs_set(prefs_t prefs);

#endif
