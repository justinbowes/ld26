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
#include "xpl_irect.h"

#define MAX_KEY 4096

typedef bool (* xpl_input_character_func)(int character, void *context);

typedef enum xpl_mouse_button_state {
	xmb_left	= 1 << 1,
	xmb_middle	= 1 << 2,
	xmb_right	= 1 << 3
} xpl_mouse_button_state_t;

bool xpl_input_character_is_printable(int character, bool include_crlf);
bool xpl_input_key_down(int key);
int xpl_input_add_character_listener(xpl_input_character_func func, void *context);
void xpl_input_remove_character_listener(int listener_id);

bool xpl_input_keyboard_should_auto_focus(void);
void xpl_input_enable_keyboard(void);
void xpl_input_disable_keyboard(void);
void xpl_input_enable_characters(void);
void xpl_input_disable_characters(void);

bool xpl_input_mouse_down_in(xirect rect, xivec2 *coord, int *iid);
bool xpl_input_interaction_active(int iid);
void xpl_input_get_mouse_position(xivec2 *position);
void xpl_input_get_mouse_buttons(xpl_mouse_button_state_t *buttons);
void xpl_input_get_scroll_deltas(xivec2 *deltas);


#endif
