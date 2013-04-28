//
//  xpl_thread.h
//  p1
//
//  Created by Justin Bowes on 2013-02-14.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_thread_h
#define p1_xpl_thread_h

#include <stdbool.h>
#include <stddef.h>

#define XPL_THREAD_INVALID -1

typedef int xpl_thread_id;

typedef void(*xpl_thread_work_function)(void);
typedef void(*xpl_thread_finalize_function)(void);

// Primary thread only API
bool xpl_threads_initialized(void);
int xpl_threads_pool_size(void);
void xpl_threads_init(size_t thread_pool_size, void *primary_thread_local_data);
void xpl_threads_shutdown();

xpl_thread_id xpl_thread_assign_work(xpl_thread_work_function work_func, xpl_thread_finalize_function finalize_func, void *thread_local_data);
bool xpl_thread_unassign(xpl_thread_id tid);
bool xpl_thread_unassign_block(xpl_thread_id tid, int timeout);
bool xpl_thread_start(xpl_thread_id tid);
bool xpl_thread_stop(xpl_thread_id tid);

// Thread API
void xpl_thread_yield(void);
void xpl_thread_sleep(int milliseconds);
size_t xpl_thread_get_pool_size(void);
void *xpl_thread_get_local_data(void);
bool xpl_thread_is_primary(void);
long xpl_thread_get_and_reset_calls(xpl_thread_id tid);


#endif
