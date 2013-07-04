//
//  xpl_file.h
//  ld26
//
//  Created by Justin Bowes on 2013-04-25.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef ld26_xpl_file_h
#define ld26_xpl_file_h

#include <stdbool.h>

#include "xpl_dynamic_buffer.h"

const char *xpl_file_extension(const char *filename);
bool xpl_file_has_extension(const char *filename, const char *extension);
void xpl_file_get_contents(const char *filename, xpl_dynamic_buffer_t *buffer);
char *xpl_basename(const char *name);
char *xpl_dirname(char *path);

#if defined(XPL_PLATFORM_WINDOWS)
#	define XPL_PATH_SEPARATOR '\\'
#else
#	define XPL_PATH_SEPARATOR '/'
#endif

#endif
