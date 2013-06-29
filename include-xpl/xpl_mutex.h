//
//  xpl_mutex.h
//  p1
//
//  Created by Justin Bowes on 2013-02-14.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_mutex_h
#define p1_xpl_mutex_h

#include <stdbool.h>

typedef struct xpl_mutex xpl_mutex_t;

xpl_mutex_t *xpl_mutex_new(void);
void xpl_mutex_destroy(xpl_mutex_t **ppmutex);

bool xpl_mutex_is_valid(xpl_mutex_t *self);
bool xpl_mutex_enter(xpl_mutex_t *self);
bool xpl_mutex_leave(xpl_mutex_t *self);

#endif
