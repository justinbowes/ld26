//
//  memory.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-02.
//  Copyright (c) 2012 Justin Bowes. All rights reserved.
//

#ifndef xpl_osx_memory_h
#define xpl_osx_memory_h

#include <stdio.h>
#include <stdlib.h>

#include "xpl_rc.h"

#ifdef DEBUG
#define XPL_MEMORY_TRACKING_ALLOCATOR   0
#endif

// ----------- tracking allocator -----------
#ifndef XPL_MEMORY_TRACKING_ALLOCATOR
#define XPL_MEMORY_TRACKING_ALLOCATOR   0
#endif

#if XPL_MEMORY_TRACKING_ALLOCATOR == 0

#define xpl_alloc(size)			malloc(size)
#define xpl_calloc(size)		calloc(size, 1)
#define xpl_realloc(ptr, size)	realloc(ptr, size)
#define xpl_free(ptr)			free(ptr)

#else
// ------ memory management -----------
void *xpl_alloc(size_t size);
void *xpl_calloc(size_t size);
void xpl_free(void *ptr);
void *xpl_realloc(void *ptr, size_t size);

#endif

// ----- helps when compiling as C++ instead of C99
#define xpl_alloc_type(type) 		(type *)xpl_alloc(sizeof(type))
#define xpl_zero_struct(pinstance)	memset(pinstance, 0, sizeof(*pinstance))
#define xpl_calloc_type(type)		(type *)xpl_calloc(sizeof(type))

typedef struct xpl_memory_comparison_result {
    int is_different;
    size_t first_difference_offset;
    size_t last_difference_offset;
} xpl_memory_comparison_result_t;

xpl_memory_comparison_result_t xpl_memory_compare(void *m1, void *m2, size_t compare_bytes);

#endif
