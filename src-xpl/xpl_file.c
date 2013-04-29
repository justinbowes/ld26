//
//  xpl_file.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-25.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>
#include <string.h>

#include "xpl_file.h"

const char *xpl_file_extension(const char *filename) {
	char *e = strrchr(filename, '.');
	if (e == NULL) e = "";
	return ++e;
}

bool xpl_file_has_extension(const char *filename, const char *extension) {
	const char *file_extension = xpl_file_extension(filename);
	return strcmp(file_extension, extension) == 0;
}
