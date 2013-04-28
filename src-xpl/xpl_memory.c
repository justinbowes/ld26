//
//  memory.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-02.
//  Copyright (c) 2012 Justin Bowes. All rights reserved.
//
#ifndef XPL_MEMORY_H
#define XPL_MEMORY_H

#include <stdlib.h>

#include "uthash.h"

#include "xpl_log.h"
#include "xpl_memory.h"
#include "xpl_platform.h"

#if XPL_MEMORY_TRACKING_ALLOCATOR != 0

// allocation info struct.
typedef struct xpl_allocation {
    void    *handle;
    size_t  bytes;

    UT_hash_handle hh;
} xpl_allocation_t;

// table of allocations.
static xpl_allocation_t *allocations = NULL;
static size_t allocated_bytes = 0;

void *xpl_alloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        LOG_ERROR("malloc failed for %lu bytes", (unsigned long)size);
        abort();
    }

    xpl_allocation_t *allocation_info = (xpl_allocation_t *)malloc(sizeof(xpl_allocation_t));
    if (allocation_info == NULL) {
        LOG_ERROR("malloc failed for table entry of %lu bytes", (unsigned long)sizeof(xpl_allocation_t));
        abort();
    }

    allocation_info->handle = ptr;
    allocation_info->bytes = size;
    HASH_ADD_PTR(allocations, handle, allocation_info);

    LOG_TRACE("+ %p     %lu bytes entry: %p", allocation_info->handle, allocation_info->bytes, allocation_info);

    allocated_bytes += size;

    return ptr;
}

void *xpl_calloc(size_t size) {
	void *result = xpl_alloc(size);
	memset(result, 0, size);
	return result;
}

void xpl_free(void *ptr) {
    xpl_allocation_t *allocation_info = NULL;
    HASH_FIND_PTR(allocations, &ptr, allocation_info);

    if (allocation_info == NULL) {
        LOG_ERROR("Double-free detected %p (allocation table entry not found)", ptr);

        xpl_allocation_t *found, *tmp;
        LOG_DEBUG("Table:");
        HASH_ITER(hh, allocations, found, tmp) {
            LOG_DEBUG("> %p     %lu bytes", found->handle, (unsigned long)found->bytes);
        }
        abort();
    }
    LOG_TRACE("! %p     %lu bytes", allocation_info->handle, allocation_info->bytes);

    if (allocation_info->handle != ptr) {
        LOG_ERROR("Corrupt handle in memory table; expected %p, found %p", allocation_info->handle, ptr);
        abort();
    }

    HASH_DEL(allocations, allocation_info);
    allocated_bytes -= allocation_info->bytes;

    free(allocation_info);
    free(ptr);
}

void *xpl_realloc(void *ptr, size_t newsize) {
    // realloc() is tolerant of a null
    if (ptr == NULL) {
        return xpl_alloc(newsize);
    }
    // If newsize() is 0, ptr is deallocated.
    if (newsize == 0) {
        xpl_free(ptr);
        return NULL;
    }

    xpl_allocation_t *allocation_info = NULL;
    HASH_FIND_PTR(allocations, &ptr, allocation_info);

    if (allocation_info == NULL) {
        LOG_ERROR("Allocation not found %p (allocation table entry not found)", ptr);
        abort();
    }
    LOG_TRACE("~ %p     %lu bytes (was %lu)", allocation_info->handle, newsize, allocation_info->bytes);

    if (allocation_info->handle != ptr) {
        LOG_ERROR("Corrupt handle in memory table; expected %p, found %p", allocation_info->handle, ptr);
        abort();
    }

    // Delete the old entry; the allocated address "moved"
    HASH_DEL(allocations, allocation_info);
    allocation_info->handle = realloc(allocation_info->handle, newsize);
    HASH_ADD_PTR(allocations, handle, allocation_info);

    allocated_bytes -= allocation_info->bytes;
    allocated_bytes += newsize;
    allocation_info->bytes = newsize;

    return allocation_info->handle;
}
#endif

xpl_memory_comparison_result_t xpl_memory_compare(void *m1, void *m2, const size_t compare_bytes) {
    xpl_memory_comparison_result_t result;

    uint8_t compare_fwd, *pf1 = m1, *pf2 = m2;
    uint8_t compare_back, *pb1 = m1 + compare_bytes - 1, *pb2 = m2 + compare_bytes - 1;
    do {
        compare_fwd = *(pf1++) - *(pf2++);
    } while ((pf1 <= pb1) && (compare_fwd == 0));

    if (! compare_fwd) {
        
        result.is_different = 0;
        result.first_difference_offset = ~0;
        result.last_difference_offset = 0;
        
    } else {
        
        do {
            compare_back = *(pb1--) - *(pb2--);
        } while ((pf1 <= pb1) && (compare_back == 0));

        result.is_different = 1;
        result.first_difference_offset = ((size_t)((void *)pf1 - m1));
        result.last_difference_offset = ((size_t)((void *)pb1 - m1));
    }

    return result;
}

#endif
