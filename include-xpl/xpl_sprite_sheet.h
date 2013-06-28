//
//  xpl_sprite_sheet.h
//  app
//
//  Created by Justin Bowes on 2013-06-28.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef app_xpl_sprite_sheet_h
#define app_xpl_sprite_sheet_h

#include "xpl_ivec2.h"
#include "xpl_irect.h"

#define XPL_SPRITE_SHEET_END    -1

struct xpl_sprite_batch;

typedef struct xpl_sprite_sheet_entry xpl_sprite_sheet_entry_t;
typedef struct xpl_sprite_sheet xpl_sprite_sheet_t;

xpl_sprite_sheet_t *xpl_sprite_sheet_new(struct xpl_sprite_batch *batch, const char *json_resource);
struct xpl_sprite *xpl_sprite_get(struct xpl_sprite_sheet *sheet, const char *name);
void xpl_sprite_sheet_destroy(xpl_sprite_sheet_t **ppsheet);

#endif
