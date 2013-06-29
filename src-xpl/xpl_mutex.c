//
//  xpl_mutex.c
//  p1
//
//  Created by Justin Bowes on 2013-02-14.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <assert.h>

#include "xpl_platform.h"
#include "xpl_memory.h"
#include "xpl_mutex.h"

#ifdef XPL_PLATFORM_WINDOWS
#include <process.h>
typedef CRITICAL_SECTION    mutex_t;
#else
#include <pthread.h>
#include <unistd.h>
typedef pthread_mutex_t     *mutex_t;
#endif

struct xpl_mutex {
    mutex_t  mutex;
    bool     valid;
};

xpl_mutex_t *xpl_mutex_new() {
    xpl_mutex_t *self = xpl_calloc_type(xpl_mutex_t);
    
    self->valid = false;
#ifdef XPL_PLATFORM_WINDOWS
    InitializeCriticalSection(&self->mutex);
    self->valid = true;
#else
    self->mutex = xpl_alloc_type(pthread_mutex_t);
    if (self->mutex) {
        if (pthread_mutex_init(self->mutex, NULL) == 0) {
            self->valid = true;
        }
    }
#endif
    return self;
}

void xpl_mutex_destroy(xpl_mutex_t **ppsection) {
    assert(ppsection);
    xpl_mutex_t *self = *ppsection;
    assert(self);
    
#ifdef XPL_PLATFORM_WINDOWS
    DeleteCriticalSection(&self->mutex);
#else
    if (self->mutex) {
        pthread_mutex_destroy(self->mutex);
        xpl_free(self->mutex);
    }
#endif
    self->valid = false;
    
    xpl_free(self);
    *ppsection = NULL;
}

bool xpl_mutex_is_valid(xpl_mutex_t *self) {
    return self->valid;
}

bool xpl_mutex_enter(xpl_mutex_t *self) {
    assert(self);
    assert(self->valid);
    
    bool result = false;
    
#ifdef XPL_PLATFORM_WINDOWS
    EnterCriticalSection(&self->mutex);
#else
    if (pthread_mutex_lock(self->mutex) == 0) {
        result = true;
    }
#endif
    
    return result;
}

bool xpl_mutex_leave(xpl_mutex_t *self) {
    assert(self);
    assert(self->valid);
    
    bool result = false;
    
#ifdef XPL_PLATFORM_WINDOWS
    LeaveCriticalSection(&self->mutex);
#else
    if (pthread_mutex_unlock(self->mutex) == 0) {
        result = true;
    }
#endif
    
    return result;
}

