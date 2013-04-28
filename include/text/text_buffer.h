//
//  text_buffer.h
//  p1
//
//  Created by Justin Bowes on 2013-04-16.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_text_buffer_h
#define p1_text_buffer_h

#include "xpl_rect.h"

typedef void(* text_buffer_readline_callback)(const char *line);

struct text_buffer;
typedef struct text_buffer text_buffer_t;

text_buffer_t *text_buffer_new(const char *font_name, float font_size, xrect layout, xvec4 foreground, xvec4 background);
void text_buffer_destroy(text_buffer_t **buf);

xirect text_buffer_get_character_grid(text_buffer_t *self);
xivec2 text_buffer_get_cursor_position(text_buffer_t *self);
void text_buffer_set_cursor_position(text_buffer_t *self, xivec2 cursor_position);
// void text_buffer_set_scroll_range(text_buffer_t *self, int min, int max); ?
void text_buffer_clear_scroll_range(text_buffer_t *self);

void text_buffer_set_fg(text_buffer_t *self, xvec4 color);
void text_buffer_set_bg(text_buffer_t *self, xvec4 color);
void text_buffer_set_underline(text_buffer_t *self, bool underline);
void text_buffer_set_reverse_text(text_buffer_t *self);
void text_buffer_reset_format(text_buffer_t *self);

void text_buffer_clear(text_buffer_t *self);
void text_buffer_add_text(text_buffer_t *self, const char *text);
void text_buffer_add_line(text_buffer_t *self, const char *text);

void text_buffer_set_keyboard_active(text_buffer_t *self, text_buffer_readline_callback readline_callback);
void text_buffer_get_buffered_input(text_buffer_t *self, char *buffer, size_t len);
void text_buffer_set_prompt(text_buffer_t *self, const char *prompt);

void text_buffer_set_screen_colors(text_buffer_t *self, xvec4 fg_screen, xvec4 bg_screen);

void text_buffer_update(text_buffer_t *self, double time);
void text_buffer_render(text_buffer_t *self, xmat4 *mvp);

#endif
