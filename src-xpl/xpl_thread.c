//
//  xpl_thread.c
//  p1
//
//  Created by Justin Bowes on 2013-02-14.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <assert.h>

#include "xpl_log.h"
#include "xpl_platform.h"
#include "xpl_memory.h"
#include "xpl_mutex.h"

#include "xpl_thread.h"

// ----------------------------------------------------------------------------

#define THREAD_SLEEP_DURATION   50                  // ms

#ifdef XPL_PLATFORM_WINDOWS
#include <windows.h>
#include <process.h>
#define INVALID_LOCAL_DATA_KEY  TLS_OUT_OF_INDEXES
#define INVALID_THREAD          NULL
typedef DWORD                   local_data_key_t;
typedef HANDLE                  thread_t;
typedef unsigned int            thread_callback_t;
typedef long                    thread_calls_counter_t;
#else
#   ifdef XPL_PLATFORM_OSX
#       include <libkern/OSAtomic.h>
#       define xpl_atomic_increment(x) OSAtomicIncrement64Barrier(x)
#   else
#       define xpl_atomic_increment(x) __sync_add_and_fetch(x, 1);
#   endif
#include <pthread.h>
#include <unistd.h>
#define INVALID_LOCAL_DATA_KEY  0
#define INVALID_THREAD          0
typedef pthread_key_t           local_data_key_t;
typedef pthread_t               thread_t;
typedef void *                  thread_callback_t;
typedef int64_t                 thread_calls_counter_t;
#endif

typedef enum thread_state {
    ts_all                 = ~0,
    ts_unassigned          = 1 << 0,
    ts_stopped             = 1 << 1,
    ts_running             = 1 << 2,
    ts_running_request     = 1 << 3,
    ts_unassign_request    = 1 << 4,
    ts_terminate_request   = 1 << 5,
    ts_status_max          = ts_terminate_request
} thread_state_t;

typedef struct thread_info {
    xpl_thread_id                   id;
    xpl_thread_work_function        work_function;
    xpl_thread_finalize_function    finalize_function;

    thread_state_t                  state;
    thread_calls_counter_t          calls;
    thread_t                        thread;
    xpl_mutex_t						*thread_mutex;
    void                            *local_data;
} thread_info_t;

typedef struct thread_context {
    xpl_mutex_t						*singleton_mutex;
    local_data_key_t                local_data_key;
    void                            *primary_thread_local_data;
    
    size_t                          thread_pool_size;
    thread_info_t                   *thread_pool;
} thread_context_t;

// ----------------------------------------------------------------------------

static thread_context_t *ctx = NULL;

// ----------------------------------------------------------------------------

void xpl_thread_yield() {
#ifdef XPL_PLATFORM_WINDOWS
    Sleep(0);
#else
    sched_yield();
#endif
}

void xpl_thread_sleep(int milliseconds) {
    assert(milliseconds > 0);
    
#ifdef XPL_PLATFORM_WINDOWS
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// ----------------------------------------------------------------------------

void associate_local_data_with_thread(xpl_thread_id tid) {
    assert(ctx);
    assert(tid >= 0);
    assert(tid < ctx->thread_pool_size);
    
#ifdef XPL_PLATFORM_WINDOWS
    TlsSetValue(ctx->local_data_key, ctx->thread_pool[tid].local_data);
#else
    pthread_setspecific(ctx->local_data_key, ctx->thread_pool[tid].local_data);
#endif
}

void *get_local_data_raw() {
    assert(ctx);
    
    void *result = NULL;
#ifdef XPL_PLATFORM_WINDOWS
    result = TlsGetValue(ctx->local_data_key);
#else
    result = pthread_getspecific(ctx->local_data_key);
#endif
    
    return result;
}

bool is_primary_thread() {
    return (get_local_data_raw() == ctx);
}

void lock_thread(xpl_thread_id tid) {
    assert(ctx);
    assert(tid >= 0);
    assert(tid < ctx->thread_pool_size);
    assert(ctx->thread_pool[tid].thread_mutex);
    
    xpl_mutex_enter(ctx->thread_pool[tid].thread_mutex);
}

void unlock_thread(xpl_thread_id tid) {
    assert(ctx);
    assert(tid >= 0);
    assert(tid < ctx->thread_pool_size);
    assert(ctx->thread_pool[tid].thread_mutex);
    
    xpl_mutex_leave(ctx->thread_pool[tid].thread_mutex);
}

int get_thread_state(xpl_thread_id tid) {
    assert(ctx);
    assert(tid >= 0);
    assert(tid < ctx->thread_pool_size);
    
    return ctx->thread_pool[tid].state;
}

int test_and_set_thread_state(xpl_thread_id tid, int condition, int value) {
    assert(ctx);
    assert(tid >= 0);
    assert(tid < ctx->thread_pool_size);

    lock_thread(tid);
    
    int result = ctx->thread_pool[tid].state;
    if (result & condition) {
        ctx->thread_pool[tid].state = value;
        result = value;
    }
    
    unlock_thread(tid);
    return result;
}

void call_work_function_for_thread(xpl_thread_id tid) {
    assert(ctx);
    assert(tid >= 0);
    assert(tid < ctx->thread_pool_size);

    // No need to lock the thread because work_func is guaranteed to be valid
    // for the duration of the function.
    xpl_thread_work_function func = ctx->thread_pool[tid].work_function;
    if (func) {
        func();
        
#ifdef XPL_PLATFORM_WINDOWS
        InterlockedIncrement(&ctx->thread_pool[tid].calls);
#else
        /*
        lock_thread(tid);
        ctx->thread_pool[tid].calls++;
        unlock_thread(tid);
         */
        xpl_atomic_increment(&ctx->thread_pool[tid].calls);
#endif
    }
}

void call_finalize_function_for_thread(xpl_thread_id tid) {
    assert(ctx);
    assert(tid >= 0);
    assert(tid < ctx->thread_pool_size);

    xpl_thread_finalize_function func = ctx->thread_pool[tid].finalize_function;
    if (func && ctx->thread_pool[tid].calls) {
        func();
    }
}

void callback_handle_status(xpl_thread_id tid, int status) {
    int result = ts_unassigned;
    switch (status) {
        case ts_running_request:
            result = test_and_set_thread_state(tid, ts_running_request, ts_running);
            if (result != ts_running) {
                // sleep it off and try again later
                xpl_thread_sleep(THREAD_SLEEP_DURATION);
                break;
            }
            associate_local_data_with_thread(tid);
            // no break
            
        case ts_running:
            call_work_function_for_thread(tid);
            xpl_thread_yield();
            break;
            
        case ts_unassign_request:
            call_finalize_function_for_thread(tid);
            test_and_set_thread_state(tid, ts_unassign_request, ts_unassigned);
            // no break
            
        default:
            xpl_thread_sleep(THREAD_SLEEP_DURATION);
            break;
    }
}

thread_callback_t XPLCALL thread_callback(void *param) {
    assert(param);
    
    xpl_thread_id tid = *(xpl_thread_id *)param;
    
    int state = get_thread_state(tid);
    while (state != ts_terminate_request) {
        callback_handle_status(tid, state);
        state = get_thread_state(tid);
    }
    
#ifdef XPL_PLATFORM_WINDOWS
    _endthreadex(0);
#endif
    
    return 0;
}


void create_thread(size_t i) {
	assert(ctx);
	
	ctx->thread_pool[i].id                  = (int)i;
	ctx->thread_pool[i].state               = ts_unassigned;
	ctx->thread_pool[i].work_function       = NULL;
	ctx->thread_pool[i].calls               = 0L;
	ctx->thread_pool[i].thread              = INVALID_THREAD;
	ctx->thread_pool[i].local_data          = NULL;
	
	// Initialize the thread's critical section
	ctx->thread_pool[i].thread_mutex = xpl_mutex_new();
	if ((ctx->thread_pool[i].thread_mutex == NULL) ||
		(!xpl_mutex_is_valid(ctx->thread_pool[i].thread_mutex))) {
		// Thread allocation failed here. Remember this and quit trying.
		LOG_ERROR("Failed to create mutex %llu", (long long unsigned)i);
		ctx->thread_pool_size = i;
		return;
	}
	
	// Create the thread
#ifdef XPL_PLATFORM_WINDOWS
	ctx->thread_pool[i].thread = (thread_t)_beginthreadex(NULL, 0,
														  thread_callback,
														  &(ctx->thread_pool[i].id),
														  0,
														  NULL);
	if (ctx->thread_pool[i].thread == INVALID_THREAD) {
		LOG_ERROR("Failed to create windows thread %d", i);
		assert(false);
		ctx->thread_pool_size = i;
		return;
	}
#else
	if (pthread_create(&(ctx->thread_pool[i].thread), NULL, thread_callback, &(ctx->thread_pool[i].id)) != 0) {
		LOG_ERROR("Failed to create posix thread %llu", (long long unsigned)i);
		assert(false);
		ctx->thread_pool_size = i;
		return;
	}
#endif
}


void cleanup(bool wait_for_termination) {
    assert(ctx);
    for (xpl_thread_id i = 0; i < ctx->thread_pool_size; ++i) {
        test_and_set_thread_state(i, ts_all, ts_terminate_request);
    }
    
#ifndef XPL_PLATFORM_WINDOWS
    struct timespec join_delay;
    join_delay.tv_sec = 1;
#endif
    
    for (xpl_thread_id i = 0; i < ctx->thread_pool_size; ++i) {
#ifdef XPL_PLATFORM_WINDOWS
        WaitForSingleObject(ctx->thread_pool[i].thread, wait_for_termination ? INFINITE : 1000);
        CloseHandle(ctx->thread_pool[i].thread);
#else
        if (wait_for_termination) {
            pthread_join(ctx->thread_pool[i].thread, NULL);
        } else {
            pthread_timedjoin_np(ctx->thread_pool[i].thread, NULL, &join_delay);
        }
        
        pthread_detach(ctx->thread_pool[i].thread);
#endif
        xpl_mutex_destroy(&ctx->thread_pool[i].thread_mutex);
    }
    
    xpl_free(ctx->thread_pool);
    ctx->thread_pool = NULL;
    ctx->thread_pool_size = 0;
    
    if (ctx->local_data_key != INVALID_LOCAL_DATA_KEY) {
#ifdef XPL_PLATFORM_WINDOWS
        TlsFree(ctx->local_data_key);
#else
        pthread_key_delete(ctx->local_data_key);
#endif
    }
    ctx->local_data_key = INVALID_LOCAL_DATA_KEY;
}

// ----------------------------------------------------------------------------

bool xpl_threads_initialized() {
    return (ctx != NULL);
}

int xpl_threads_pool_size() {
    return (ctx ? (int)ctx->thread_pool_size : 0);
}

void xpl_threads_init(size_t thread_pool_size, void *primary_thread_local_data) {
    assert(ctx == NULL);
    
    ctx = xpl_calloc_type(thread_context_t);
    
    ctx->singleton_mutex = xpl_mutex_new();
    assert(xpl_mutex_is_valid(ctx->singleton_mutex));
    
    xpl_mutex_enter(ctx->singleton_mutex);
    {
        ctx->thread_pool_size = thread_pool_size;
        ctx->thread_pool = xpl_calloc(thread_pool_size * sizeof(*ctx->thread_pool));
        assert(ctx->thread_pool);
        
        for (xpl_thread_id i = 0; i < thread_pool_size; ++i) {
			create_thread(i);
        }
        
        // Create the local data key
#ifdef XPL_PLATFORM_WINDOWS
        ctx->local_data_key = TlsAlloc();
        if (ctx->local_data_key == INVALID_LOCAL_DATA_KEY) {
            LOG_ERROR("Couldn't create Windows primary TLS");
            assert(false);
        }
        TlsSetValue(ctx->local_data_key, ctx);
#else
        if (pthread_key_create(&ctx->local_data_key, NULL) != 0) {
            LOG_ERROR("Couldn't create posix primary TLS");
            assert(false);
        }
        if (pthread_setspecific(ctx->local_data_key, ctx) != 0) {
            LOG_ERROR("Couldn't set value for posix primary TLS");
            assert(false);
        }
        
        ctx->primary_thread_local_data = primary_thread_local_data;
#endif
    }
    xpl_mutex_leave(ctx->singleton_mutex);
}

void xpl_threads_shutdown() {
    assert(ctx);
    xpl_mutex_enter(ctx->singleton_mutex);
    {
        assert(is_primary_thread());
        cleanup(true);
    }
    xpl_mutex_leave(ctx->singleton_mutex);
    
    xpl_free(ctx);
    ctx = NULL;
}

xpl_thread_id xpl_thread_assign_work(xpl_thread_work_function work_func, xpl_thread_finalize_function finalize_func, void *thread_local_data) {
    assert(ctx);
    assert(is_primary_thread());
    
    xpl_thread_id id = XPL_THREAD_INVALID;
    
    xpl_mutex_enter(ctx->singleton_mutex);
    {
        for (xpl_thread_id i = 0; i < ctx->thread_pool_size; ++i) {
            if (ctx->thread_pool[i].state == ts_unassigned) {
                id = i;
                break;
            }
        }
        
        if (id != XPL_THREAD_INVALID) {
            ctx->thread_pool[id].state              = ts_stopped;
            ctx->thread_pool[id].work_function      = work_func;
            ctx->thread_pool[id].finalize_function  = finalize_func;
            ctx->thread_pool[id].calls              = 0;
            ctx->thread_pool[id].local_data         = thread_local_data;
        }
    }
    xpl_mutex_leave(ctx->singleton_mutex);
    
    return id;
}

bool xpl_thread_unassign(xpl_thread_id tid) {
    assert(ctx);
    assert(is_primary_thread());
    
    // Invalidate the thread.
    // Don't unassign if already unassigned.
    int state = test_and_set_thread_state(tid, ~ts_unassigned, ts_unassign_request);
    
    return (state == ts_unassign_request) || (state == ts_unassigned);
}

bool xpl_thread_unassign_block(xpl_thread_id tid, int timeout) {
    assert(ctx);
    assert(is_primary_thread());
    
    int state;
    for (int t = 0; t < timeout; t += 100) {
        state = test_and_set_thread_state(tid, ~ts_unassigned, ts_unassign_request);
        if (state == ts_unassigned) break;
        xpl_thread_sleep(100);
    }
    
    return state == ts_unassigned;
}

bool xpl_thread_start(xpl_thread_id tid) {
    assert(ctx);
    assert(is_primary_thread());
    
    int state = test_and_set_thread_state(tid, ts_stopped, ts_running_request);
    
    return (state == ts_running_request) || (state == ts_running);
}

bool xpl_thread_stop(xpl_thread_id tid) {
    assert(ctx);
    assert(is_primary_thread());
    
    int state = test_and_set_thread_state(tid, ts_running | ts_running_request, ts_stopped);
    
    return (state == ts_stopped);
}

size_t xpl_thread_get_pool_size() {
    assert(ctx);
    return ctx->thread_pool_size;
}

void *xpl_thread_get_local_data() {
    assert(ctx);
    void *result = get_local_data_raw();
    if (result == ctx) {
        result = ctx->primary_thread_local_data;
    }
    
    return result;
}

bool xpl_thread_is_primary() {
    return (get_local_data_raw() == ctx);
}

int64_t xpl_thread_get_and_reset_calls(xpl_thread_id tid) {
    assert(ctx);
    assert(tid);
    assert(tid < ctx->thread_pool_size);
    
    int64_t result = 0L;
    
    lock_thread(tid);
    result = ctx->thread_pool[tid].calls;
    ctx->thread_pool[tid].calls = 0L;
    unlock_thread(tid);
    
    return result;
}

