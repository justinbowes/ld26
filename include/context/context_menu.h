//
//  context_menu.h
//  p1
//
//  Created by Justin Bowes on 2013-02-10.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_context_menu_h
#define p1_context_menu_h

#include "xpl_context.h"

typedef enum menu_select {
    ms_main = 0,
    ms_load,
    ms_configure,
    ms_configure_graphics,
    ms_game_start,
    ms_exit
} menu_select_t;

extern xpl_context_def_t menu_context_def;

void menu_push(menu_select_t menu);
bool menu_pop(void);

#endif
