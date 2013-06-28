//
//  xpl_input_glfw.c
//  protector
//
//  Created by Justin Bowes on 2013-06-22.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

// #ifdef XPL_TOOLKIT_GLFW

#include <stdio.h>
#include <stdbool.h>

#include <xpl_gl.h>

#include "xpl_input.h"

// we are using a copy of the GLFW key map presently
#define key_translator(x) (x)

static int keys_active[MAX_KEY];

extern void xpl_input__internal__dispatch_character(int character);

static bool keyboard_active = false;
static bool characters_active = false;
static int last_scroll_position = INT16_MIN;

bool xpl_input_key_down(int key) {
	if (! keyboard_active) return false;
	return keys_active[key];
}

static void key_listener(int key, int state) {
	keys_active[key_translator(key)] = (state == GLFW_PRESS);
}

static void char_listener(int character, int state) {
	if (state == GLFW_PRESS) xpl_input__internal__dispatch_character(character);
}

bool xpl_input_keyboard_should_auto_focus(void) {
	return true; // default behavior except where keyboard is intrusive
}

void xpl_input_enable_keyboard(void) {
	if (keyboard_active) return;
	glfwSetKeyCallback(key_listener);
	keyboard_active = true;
}

void xpl_input_disable_keyboard(void) {
	if (! keyboard_active) return;
	glfwSetKeyCallback(NULL);
	keyboard_active = false;
}

void xpl_input_enable_characters(void) {
	if (characters_active) return;
	glfwSetCharCallback(char_listener);
	characters_active = true;
}

void xpl_input_disable_characters(void) {
	if (! characters_active) return;
	glfwSetCharCallback(NULL);
	characters_active = false;
}

void xpl_input_get_mouse_position(xivec2 *position) {
	glfwGetMousePos(&position->x, &position->y);
}

void xpl_input_get_mouse_buttons(xpl_mouse_button_state_t *buttons) {
	int v = ((glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) ? xmb_left : 0) |
			 (glfwGetMouseButton(GLFW_MOUSE_BUTTON_2) ? xmb_right : 0) |
			 (glfwGetMouseButton(GLFW_MOUSE_BUTTON_3) ? xmb_middle : 0));
	*buttons = v;
}

void xpl_input_get_scroll_deltas(xivec2 *deltas) {
	int scroll_position = glfwGetMouseWheel();
	if (last_scroll_position == INT16_MIN) {
		deltas->x = 0;
		deltas->y = 0;
	}
	deltas->x = 0;
	deltas->y = scroll_position - last_scroll_position;
}

bool xpl_input_mouse_down_in(xirect rect) {
	xivec2 position;
	xpl_input_get_mouse_position(&position);
	return xirect_in_bounds(rect, position);
}




// #endif