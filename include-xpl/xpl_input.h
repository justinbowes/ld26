//
//  xpl_input.h
//  protector
//
//  Created by Justin Bowes on 2013-06-22.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef protector_xpl_input_h
#define protector_xpl_input_h

#include <stdbool.h>

#include "xpl_ivec2.h"

#include "xpl_input_keydefs.h"

#define MAX_KEY 4096

typedef bool (* xpl_input_character_func)(int character, void *context);

typedef enum xpl_mouse_button_state {
	xmb_left	= 1 << 1,
	xmb_middle	= 1 << 2,
	xmb_right	= 1 << 3
} xpl_mouse_button_state_t;

bool xpl_input_is_character(int key);
bool xpl_input_key_down(int key);
int xpl_input_add_character_listener(xpl_input_character_func func, void *context);
void xpl_input_remove_character_listener(int listener_id);

void xpl_input_enable_keyboard(void);
void xpl_input_disable_keyboard(void);
void xpl_input_enable_characters(void);
void xpl_input_disable_characters(void);

void xpl_input_get_mouse_position(xivec2 *position);
void xpl_input_get_mouse_buttons(xpl_mouse_button_state_t *buttons);
void xpl_input_get_scroll_deltas(xivec2 *deltas);


#endif
