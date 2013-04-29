//
//  prefs.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-29.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "xpl.h"

#include "game/prefs.h"
#include "minIni.h"

static prefs_t prefs_default(void) {
	prefs_t prefs;
	
	prefs.skip_tutorial = false;
	snprintf(prefs.name, NAME_SIZE, "");
	snprintf(prefs.server, SERVER_SIZE, "gs.ultrapew.com");
	prefs.port = 3000;
	
	return prefs;
}

void prefs_reset(void) {
	prefs_set(prefs_default());
}

prefs_t prefs_get(void) {
	char resource[PATH_MAX];
	prefs_t prefs;
	prefs_t defaults = prefs_default();
	
	xpl_resolve_resource(resource, "prefs.ini", PATH_MAX);
	
	prefs.skip_tutorial = ini_getbool("prefs", "skip_tutorial", defaults.skip_tutorial, resource);
	ini_gets("prefs", "name", defaults.name, prefs.name, NAME_SIZE, resource);
	ini_gets("prefs", "server", defaults.server, prefs.server, SERVER_SIZE, resource);
	prefs.port = (unsigned short)ini_getl("prefs", "port", defaults.port, resource);
	
	return prefs;
}

void prefs_set(prefs_t prefs) {
	char resource[PATH_MAX];
	xpl_resolve_resource(resource, "prefs.ini", PATH_MAX);
	ini_puts("prefs", "skip_tutorial", prefs.skip_tutorial ? "true" : "false", resource);
	ini_puts("prefs", "name", prefs.name, resource);
	ini_puts("prefs", "server", prefs.server, resource);
	ini_putl("prefs", "port", (unsigned short)prefs.port, resource);
}

