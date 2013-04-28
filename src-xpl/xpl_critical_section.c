//
//  xpl_critical_section.c
//  p1
//
//  Created by Justin Bowes on 2013-02-14.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <assert.h>

#include "xpl_platform.h"
#include "xpl_memory.h"
#include "xpl_critical_section.h"

#ifdef XPL_PLATFORM_WINDOWS
#include <process.h>
typedef CRITICAL_SECTION    critical_section_t;
#else
#include <pthread.h>
#include <unistd.h>
typedef pthread_mutex_t     *critical_section_t;
#endif

struct xpl_critical_section {
    critical_section_t  critical_section;
    bool                valid;
};

xpl_critical_section_t *xpl_critical_section_new() {
    xpl_critical_section_t *self = xpl_calloc_type(xpl_critical_section_t);
    
    self->valid = false;
#ifdef XPL_PLATFORM_WINDOWS
    InitializeCriticalSection(&self->critical_section);
    self->valid = true;
#else
    self->critical_section = xpl_alloc_type(pthread_mutex_t);
    if (self->critical_section) {
        if (pthread_mutex_init(self->critical_section, NULL) == 0) {
            self->valid = true;
        }
    }
#endif
    
    return self;
}

void xpl_critical_section_destroy(xpl_critical_section_t **ppsection) {
    assert(ppsection);
    xpl_critical_section_t *self = *ppsection;
    assert(self);
    
#ifdef XPL_PLATFORM_WINDOWS
    DeleteCriticalSection(&self->critical_section);
#else
    if (self->critical_section) {
        pthread_mutex_destroy(self->critical_section);
        xpl_free(self->critical_section);
    }
#endif
    self->valid = false;
    
    xpl_free(self);
    *ppsection = NULL;
}

bool xpl_critical_section_is_valid(xpl_critical_section_t *self) {
    return self->valid;
}

bool xpl_critical_section_enter(xpl_critical_section_t *self) {
    assert(self);
    assert(self->valid);
    
    bool result = false;
    
#ifdef XPL_PLATFORM_WINDOWS
    EnterCriticalSection(&self->critical_section);
#else
    if (pthread_mutex_lock(self->critical_section) == 0) {
        result = true;
    }
#endif
    
    return result;
}

bool xpl_critical_section_leave(xpl_critical_section_t *self) {
    assert(self);
    assert(self->valid);
    
    bool result = false;
    
#ifdef XPL_PLATFORM_WINDOWS
    LeaveCriticalSection(&self->critical_section);
#else
    if (pthread_mutex_unlock(self->critical_section) == 0) {
        result = true;
    }
#endif
    
    return result;
}

