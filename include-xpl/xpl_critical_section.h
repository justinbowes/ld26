//
//  xpl_critical_section.h
//  p1
//
//  Created by Justin Bowes on 2013-02-14.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_critical_section_h
#define p1_xpl_critical_section_h

#include <stdbool.h>

typedef struct xpl_critical_section xpl_critical_section_t;

xpl_critical_section_t *xpl_critical_section_new(void);
void xpl_critical_section_destroy(xpl_critical_section_t **ppcsection);

bool xpl_critical_section_is_valid(xpl_critical_section_t *self);
bool xpl_critical_section_enter(xpl_critical_section_t *self);
bool xpl_critical_section_leave(xpl_critical_section_t *self);

#endif
