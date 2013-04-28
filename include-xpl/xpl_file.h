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

const char *xpl_file_extension(const char *filename);
bool xpl_file_has_extension(const char *filename, const char *extension);

#endif
