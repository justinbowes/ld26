//
//  text_buffer.c
//  p1
//
//  Created by Justin Bowes on 2013-04-16.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "bstrlib.h"
#include "bstraux.h"

#include "xpl_markup.h"
#include "xpl_text_cache.h"

#include "text/text_buffer.h"

#define HISTORY_SIZE	4096
#define HISTORY_LEN		64
#define PROMPT_LEN		256

#define WCHAR_SPACE		32
#define WCHAR_NEWLINE	10

typedef struct buffer_char {
	wchar_t			wchar;
	char			mbchar[4];
	
	xpl_markup_t	markup;
	
	xpl_glyph_t		*glyph;
	
} buffer_char_t;

struct text_buffer {
	
	buffer_char_t	*buffer;
	size_t			buffer_size;
	
	xpl_markup_t	*default_format;
	xpl_markup_t	*current_format;
	
	xpl_glyph_t		*glyph_proto;
	
	float			line_height;
	xrect			layout;
	xirect			char_grid;
	int				scroll_range_min;
	int				scroll_range_max;
	
	int				tab_width;
	
	buffer_char_t	*cursor;
	
	buffer_char_t	*prompt_home;
	buffer_char_t	*input_home;
	buffer_char_t	*input_end;
	text_buffer_readline_callback readline_callback;
	bool			allow_char;
	
	double			cursor_blink_timeout;
	double			cursor_blink_speed;
	bool			cursor_blink_state;
	
	xvec4			foreground_screen;
	xvec4			background_screen;
	
	char			prompt[PROMPT_LEN];
	char			history[HISTORY_LEN][HISTORY_SIZE];
	int				history_offset;
	int				history_length;
	
	xpl_text_buffer_t *tb;
	
	bool			rebuild;
};

static text_buffer_t *keyboard_bound_buffer = NULL;

static void resize_buffer(text_buffer_t *self, size_t characters) {
	self->buffer_size = characters;
	if (characters) {
		self->buffer = xpl_realloc(self->buffer, self->buffer_size * sizeof(buffer_char_t));
	} else if (self->buffer) {
		xpl_free(self->buffer);
		self->buffer = NULL;
	}
}

text_buffer_t *text_buffer_new(const char *font_name, float font_size, xrect layout, xvec4 foreground, xvec4 background) {
	
	text_buffer_t *buf = xpl_calloc_type(text_buffer_t);
	
	buf->tb = xpl_text_buffer_new(1024, 1024, 3);
	
	buf->default_format = xpl_markup_new();
	xpl_markup_set(buf->default_format, font_name, font_size, FALSE, FALSE, foreground, background);
	buf->default_format->underline_color = foreground;
	
	buf->current_format = xpl_markup_new();
	text_buffer_reset_format(buf);
	
	// Assume monospaced font for a terminal. Right?
	buf->glyph_proto = xpl_font_get_glyph(buf->default_format->font, WCHAR_SPACE); // space
	xvec2 pen = {{ 0.f, 0.f }};
	xpl_text_buffer_add_wchar(buf->tb, &pen, buf->default_format, WCHAR_NEWLINE, 0);
	buf->line_height = ceilf(-pen.y);
	xpl_text_buffer_clear(buf->tb);
	
	buf->layout = layout;
	buf->char_grid = xirect_set(0, 0,
								buf->layout.width / buf->glyph_proto->advance_x,
								buf->layout.height / buf->line_height);
	
	resize_buffer(buf, buf->char_grid.width * buf->char_grid.height);
	buf->cursor = buf->buffer; // (0, 0)
	buf->prompt_home = buf->cursor;
	buf->input_home = buf->cursor;
	buf->input_end = buf->cursor;
	
	buf->tab_width = 4;
	
	strncpy(buf->prompt, ">", 1);
	
	text_buffer_clear_scroll_range(buf);
	text_buffer_add_text(buf, "");
	text_buffer_set_screen_colors(buf, xvec4_all(1.0f), xvec4_all(1.0f));
	
	buf->cursor_blink_speed = 0.2;
	buf->cursor_blink_state = false;
	buf->cursor_blink_timeout = buf->cursor_blink_speed;
	
	buf->history_offset = 0;
	
	return buf;
}

void text_buffer_destroy(text_buffer_t **buf) {
	assert(buf);
	text_buffer_t *b = *buf;
	assert(b);
	
	xpl_free(b->buffer);
	
	xpl_markup_destroy(&b->default_format);
	xpl_markup_destroy(&b->current_format);
	
	xpl_text_buffer_destroy(&b->tb);
	
	if (keyboard_bound_buffer == b) {
		text_buffer_set_keyboard_active(NULL, NULL);
	}
	
	xpl_free(b);
	
	*buf = NULL;
}

XPLINLINE xivec2 character_position(text_buffer_t *self, buffer_char_t *ptr) {
	assert(self && ptr);
	size_t char_index = ptr - self->buffer;
	int row = (int)char_index / self->char_grid.width;
	
	assert(row < self->char_grid.height);
	
	int col = (int)char_index % self->char_grid.width;
	
	return xivec2_set(col, row);
}


static void text_buffer_set_character(text_buffer_t *self, wchar_t character, buffer_char_t *ptr) {
	ptr->wchar = character;
	wctomb(ptr->mbchar, ptr->wchar);
	
	ptr->markup = *self->current_format;
	ptr->glyph = xpl_font_get_glyph(ptr->markup.font, ptr->wchar ? ptr->wchar : WCHAR_SPACE);
	
	self->rebuild = true;
}

XPLINLINE buffer_char_t *character_pointer(text_buffer_t *self, xivec2 grid_position) {
	assert(grid_position.x < self->char_grid.width);
	assert(grid_position.y < self->char_grid.height);
	buffer_char_t *ptr = self->buffer;
	ptr += (grid_position.y * self->char_grid.width);
	ptr += grid_position.x;
	return ptr;
}

XPLINLINE xvec2 character_layout(text_buffer_t *self, buffer_char_t *ptr) {
	xivec2 grid = character_position(self, ptr);
	xvec2 position = {{
		grid.x * self->glyph_proto->advance_x,
		self->layout.height - (grid.y + 1) * self->line_height
	}};
	return position;
}

static xivec2 first_position_in_scroll_range(text_buffer_t *self) {
	if (self->scroll_range_min == -1) {
		return xivec2_set(0, 0);
	}
	return xivec2_set(0, self->scroll_range_min);
}

static xivec2 last_position_in_scroll_range(text_buffer_t *self) {
	if (self->scroll_range_max == -1) {
		return xivec2_set(self->char_grid.width - 1, self->char_grid.height - 1);
	}
	return xivec2_set(self->char_grid.width - 1, self->scroll_range_max);
}

static void scroll_up(text_buffer_t *self) {
	xivec2 scroll_range_start = first_position_in_scroll_range(self);
	xivec2 scroll_range_end = last_position_in_scroll_range(self);
	if (scroll_range_start.y == scroll_range_end.y) return;
	
	xivec2 move_range_start = {{ scroll_range_start.x, scroll_range_start.y + 1 }};
	
	buffer_char_t *first_line = character_pointer(self, scroll_range_start);
	buffer_char_t *second_line = character_pointer(self, move_range_start);
	buffer_char_t *end = character_pointer(self, scroll_range_end);
	
	// assumes scroll range is 0..max columns always
	size_t bytes = (end - second_line + 1) * sizeof(buffer_char_t);
	
	memmove(first_line, second_line, bytes);
}

static void clear_to_end_of_line(text_buffer_t *self) {
	xivec2 cursor_position = character_position(self, self->cursor);
	xivec2 eol_position = {{ self->char_grid.width - 1, cursor_position.y }};
	buffer_char_t *eolptr = character_pointer(self, eol_position);
	for (buffer_char_t *ptr = self->cursor; ptr <= eolptr; ++ptr) {
		text_buffer_set_character(self, WCHAR_SPACE, ptr);
	}
}

void text_buffer_add_text_inner(text_buffer_t *self, const char *text) {
	wchar_t wide_buffer[2048];
	mbstowcs(wide_buffer, text, 2048);
	size_t text_length = wcslen(wide_buffer);
	
	xivec2 last_char = {{ self->char_grid.width - 1, self->char_grid.height - 1 }};
	for (size_t i = 0; i < text_length; ++i) {
		wchar_t wide_char = wide_buffer[i];
		if (wide_char >= WCHAR_SPACE) {
			text_buffer_set_character(self, wide_char, self->cursor);
		}
		xivec2 char_position = character_position(self, self->cursor);
		
		// tab
		if (wide_char == 9) {
			int spaces = self->tab_width - (char_position.x % self->tab_width);
			char *pad = xpl_alloc(spaces * sizeof(char));
			memset(pad, WCHAR_SPACE, spaces);
			text_buffer_add_text_inner(self, pad);
			continue;
		}
		
		// wrap
		if ((wide_char == WCHAR_NEWLINE) ||
			(char_position.x == last_char.x && char_position.y == last_char.y)) {
			clear_to_end_of_line(self);
			if (char_position.y < last_char.y) {
				char_position = xivec2_set(0, char_position.y + 1);
				self->cursor = character_pointer(self, char_position);
			} else {
				scroll_up(self);
				self->cursor = character_pointer(self, xivec2_set(0, last_char.y));
				clear_to_end_of_line(self);
			}
		} else {
			++self->cursor;
		}
	}
}

void text_buffer_clear(text_buffer_t *self) {
	xivec2 end_char = {{
		self->char_grid.width - 1,
		self->char_grid.height - 1
	}};
	buffer_char_t *home = character_pointer(self, xivec2_set(0, 0));
	buffer_char_t *end = character_pointer(self, end_char);
	for (buffer_char_t *ptr = home; ptr <= end; ++ptr) {
		text_buffer_set_character(self, WCHAR_SPACE, ptr);
	}
	self->cursor = home;
}

void text_buffer_add_text(text_buffer_t *self, const char *text) {
	assert(text); // null pointer has crazy mb*towc* semantics. Don't pass it.
	
	size_t cursor_offset = 0;
	char buffered[2048];
	if (self == keyboard_bound_buffer) {
		text_buffer_get_buffered_input(self, buffered, 2048);
		cursor_offset = self->cursor - self->input_home;
		self->cursor = self->prompt_home;
		self->input_home = self->cursor;
		self->input_end = self->cursor;
	}
	
	text_buffer_add_text_inner(self, text);

	if (self == keyboard_bound_buffer) {
		self->prompt_home = self->cursor;
		if (self == keyboard_bound_buffer) text_buffer_add_text_inner(self, self->prompt);
		self->input_home = self->cursor;
		text_buffer_add_text_inner(self, buffered);
		self->input_end = self->cursor;
		self->cursor = self->input_home + cursor_offset;
	}
	
}

xirect text_buffer_get_character_grid(text_buffer_t *self) {
	return self->char_grid;
}

xivec2 text_buffer_get_cursor_position(text_buffer_t *self) {
	return character_position(self, self->cursor);
}

void text_buffer_set_cursor_position(text_buffer_t *self, xivec2 cursor_position) {
	assert(cursor_position.x >= 0);
	assert(cursor_position.x < self->char_grid.width);
	assert(cursor_position.y >= 0);
	assert(cursor_position.y < self->char_grid.height);
	self->cursor = character_pointer(self, cursor_position);
	self->input_home = self->cursor;
	self->input_end = self->cursor;
}

void text_buffer_set_scroll_range(text_buffer_t *self, int min, int max) {
	assert(min > 0);
	assert(min < self->char_grid.height);
	assert(max >= min);
	assert(max < self->char_grid.height);
	self->scroll_range_min = min;
	self->scroll_range_max = max;
}

void text_buffer_clear_scroll_range(text_buffer_t *self) {
	self->scroll_range_min = -1;
	self->scroll_range_max = -1;
}

void text_buffer_add_line(text_buffer_t *self, const char *text) {
	text_buffer_add_text(self, text);
	text_buffer_add_text(self, "\n");
}

void text_buffer_set_fg(text_buffer_t *self, xvec4 color) {
	self->current_format->foreground_color = color;
}

void text_buffer_set_bg(text_buffer_t *self, xvec4 color) {
	self->current_format->background_color = color;
}

void text_buffer_set_reverse_text(text_buffer_t *self) {
	self->current_format->foreground_color = self->default_format->background_color;
	self->current_format->foreground_color.a = 1.0f; // or else it won't show up.
	self->current_format->background_color = self->default_format->foreground_color;
}

void text_buffer_set_underline(text_buffer_t *self, bool underline) {
	if (underline) {
		self->current_format->underline = TRUE;
		self->current_format->underline_color = self->current_format->foreground_color;
	} else {
		self->current_format->underline = FALSE;
	}
}

void text_buffer_reset_format(text_buffer_t *self) {
	memmove(self->current_format, self->default_format, sizeof(xpl_markup_t));
	self->default_format->font = xpl_font_manager_get_from_markup(self->tb->font_manager, self->default_format);
	self->current_format->font = xpl_font_manager_get_from_markup(self->tb->font_manager, self->current_format);
}

void text_buffer_get_buffered_input(text_buffer_t *self, char *buffer, size_t len) {
	size_t input = self->input_end - self->input_home;
	++input; // trailing null.
	buffer[0] = '\0';
	
	int output_len_remaining = (int)len - 1;
	for (buffer_char_t *ptr = self->input_home; ptr < self->input_end; ++ ptr) {
		int append_len = (int)strlen(ptr->mbchar);
		if (output_len_remaining - append_len <= 0) break;
		strncat(buffer, ptr->mbchar, output_len_remaining);
		output_len_remaining -= append_len;
	}

}

static void input_character(int character) {
	assert(keyboard_bound_buffer);
	
	
	wchar_t current_char = (wchar_t)character;
	wchar_t next_char = 0;
	buffer_char_t *ptr;
	xivec2 last_char = {{
		keyboard_bound_buffer->char_grid.width - 1,
		keyboard_bound_buffer->char_grid.height - 1
	}};
	buffer_char_t *buffer_end = character_pointer(keyboard_bound_buffer, last_char);
	
	for (ptr = keyboard_bound_buffer->cursor; ptr <= keyboard_bound_buffer->input_end; ++ ptr) {
		next_char = ptr->wchar;
		text_buffer_set_character(keyboard_bound_buffer, current_char, ptr);
		current_char = next_char;
	}
	text_buffer_set_character(keyboard_bound_buffer, next_char, ++ptr);
	
	if (keyboard_bound_buffer->cursor < buffer_end) {
		++keyboard_bound_buffer->cursor;
	}
	
	if (keyboard_bound_buffer->input_end < buffer_end) {
		++keyboard_bound_buffer->input_end;
	}
}

static void input_delete() {
	assert(keyboard_bound_buffer);
	for (buffer_char_t *ptr = keyboard_bound_buffer->cursor; ptr < keyboard_bound_buffer->input_end; ++ptr) {
		buffer_char_t *next = ptr + 1;
		text_buffer_set_character(keyboard_bound_buffer, next->wchar, ptr);
	}
	text_buffer_set_character(keyboard_bound_buffer, WCHAR_SPACE, keyboard_bound_buffer->input_end);
	--keyboard_bound_buffer->input_end;
}

static void input_backspace() {
	assert(keyboard_bound_buffer);
	
	if (keyboard_bound_buffer->cursor == keyboard_bound_buffer->input_home) return;
	
	--keyboard_bound_buffer->cursor;
	input_delete();
}

static void input_home() {
	assert(keyboard_bound_buffer);
	
	keyboard_bound_buffer->cursor = keyboard_bound_buffer->input_home;
}

static void input_end() {
	assert(keyboard_bound_buffer);
	
	keyboard_bound_buffer->cursor = keyboard_bound_buffer->input_end;
}

static void input_left() {
	assert(keyboard_bound_buffer);

	if (keyboard_bound_buffer->cursor == keyboard_bound_buffer->input_home) return;
	
	--keyboard_bound_buffer->cursor;
}

static void input_right() {
	assert(keyboard_bound_buffer);
	
	if (keyboard_bound_buffer->cursor == keyboard_bound_buffer->input_end) return;
	
	++keyboard_bound_buffer->cursor;
}

static void clear_input() {
	keyboard_bound_buffer->input_end = keyboard_bound_buffer->cursor;
	keyboard_bound_buffer->input_home = keyboard_bound_buffer->input_end;
	text_buffer_add_text(keyboard_bound_buffer, "");
	clear_to_end_of_line(keyboard_bound_buffer);
}

static void replace_input_with_history() {
	clear_input();
	char *ptr = keyboard_bound_buffer->history[keyboard_bound_buffer->history_offset];
	text_buffer_add_text_inner(keyboard_bound_buffer, ptr);
	keyboard_bound_buffer->input_end = keyboard_bound_buffer->cursor;
}

static void input_up() {
	assert(keyboard_bound_buffer);
	
	if (! keyboard_bound_buffer->history_length) return;
	
	keyboard_bound_buffer->history_offset--;
	if (keyboard_bound_buffer->history_offset < 0) keyboard_bound_buffer->history_offset = keyboard_bound_buffer->history_length - 1;
	
	replace_input_with_history();
}

static void input_down() {
	assert(keyboard_bound_buffer);
	
	if (! keyboard_bound_buffer->history_length) return;

	keyboard_bound_buffer->history_offset = (keyboard_bound_buffer->history_offset + 1) % keyboard_bound_buffer->history_length;
	
	replace_input_with_history();
}

static void input_enter() {
	assert(keyboard_bound_buffer);
	
	char *buffer = xpl_alloc(HISTORY_SIZE);
	text_buffer_get_buffered_input(keyboard_bound_buffer, buffer, HISTORY_SIZE);
	text_buffer_add_text(keyboard_bound_buffer, keyboard_bound_buffer->prompt);
	text_buffer_add_text(keyboard_bound_buffer, buffer);
	text_buffer_add_text(keyboard_bound_buffer, "\n");
	
	++keyboard_bound_buffer->history_length;
	if (keyboard_bound_buffer->history_length > HISTORY_LEN) keyboard_bound_buffer->history_length = HISTORY_LEN;
	strncpy(keyboard_bound_buffer->history[keyboard_bound_buffer->history_offset], buffer, HISTORY_SIZE);
	keyboard_bound_buffer->history_offset = (keyboard_bound_buffer->history_offset + 1) % HISTORY_LEN;
	
	keyboard_bound_buffer->readline_callback(buffer);
	xpl_free(buffer);

	clear_input();
}

static void GLFWCALL console_key_callback(int key, int action) {
	assert(keyboard_bound_buffer);
	
	keyboard_bound_buffer->allow_char = false;
	void (* key_func)(void) = NULL;
	
	switch (key) {
		case GLFW_KEY_BACKSPACE:
			key_func = input_backspace;
			break;
			
		case GLFW_KEY_HOME:
			key_func = input_home;
			break;
			
		case GLFW_KEY_END:
			key_func = input_end;
			break;
			
		case GLFW_KEY_DEL:
			key_func = input_delete;
			break;
			
		case GLFW_KEY_LEFT:
			key_func = input_left;
			break;
			
		case GLFW_KEY_RIGHT:
			key_func = input_right;
			break;
			
		case GLFW_KEY_ENTER:
		case GLFW_KEY_KP_ENTER:
			key_func = input_enter;
			break;
			
		case GLFW_KEY_UP:
			key_func = input_up;
			break;
			
		case GLFW_KEY_DOWN:
			key_func = input_down;
			break;
			
		default:
			keyboard_bound_buffer->allow_char = true;
			break;
	}
	
	if ((action == GLFW_RELEASE) && key_func) {
		key_func();
		keyboard_bound_buffer->rebuild = true; // cursor moved
	}
}

static void GLFWCALL console_char_callback(int character, int action) {

	assert(keyboard_bound_buffer);
	
	// Hack because mac returns high unicode characters for keypresses like GLFW_LEFT
	if (! keyboard_bound_buffer->allow_char) return;
	
	switch (action) {
		case GLFW_PRESS:
			input_character(character);
			break;
			
		default:
			// TODO: key down and hold
			break;
	}
}


void text_buffer_set_keyboard_active(text_buffer_t *self, text_buffer_readline_callback readline_callback) {
	if (self) {
		glfwSetKeyCallback(console_key_callback);
		glfwSetCharCallback(console_char_callback);
		self->readline_callback = readline_callback;
	} else if (keyboard_bound_buffer) {
		glfwSetKeyCallback(NULL);
		glfwSetCharCallback(NULL);
		keyboard_bound_buffer->cursor_blink_state = false;
		keyboard_bound_buffer->readline_callback = NULL;
	}
	
	keyboard_bound_buffer = self;
	if (self) {
		text_buffer_add_text(self, ""); // Redraw the console with the prompt.
	}
}

void text_buffer_set_prompt(text_buffer_t *self, const char *prompt) {
	strncpy(self->prompt, prompt, 64);
}

#define MARKUP_HACK
static void rebuild_buffer(text_buffer_t *self) {
	if (! self->buffer_size) return;
	
	xpl_text_buffer_clear(self->tb);
	
	buffer_char_t *begin = character_pointer(self, xivec2_set(0, 0));
	buffer_char_t *end = character_pointer(self, xivec2_set(self->char_grid.width - 1, self->char_grid.height - 1));
	
#ifdef MARKUP_HACK
	// Render and discard a space to fuckaround the text buffer ascender code
	// It might be required for regular non-grid rendering so I don't want to
	// break it
	xpl_markup_t discard_markup = *self->default_format;
	discard_markup.foreground_color = xvec4_all(0.f);
	discard_markup.background_color = xvec4_all(0.f);
	xvec2 pen = {{ 0.f, 0.f }};
	xpl_text_buffer_add_wchar(self->tb, &pen, &discard_markup, WCHAR_SPACE, 0);
#endif
	
	for (buffer_char_t *ptr = begin; ptr <= end; ++ptr) {
		xvec2 pos = character_layout(self, ptr);
		wchar_t wchar = ptr->wchar;
		xpl_markup_t markup;
		if (ptr == self->cursor && wchar == 0) {
			markup = *self->current_format;
			markup.underline = self->cursor_blink_state;
			wchar = WCHAR_SPACE;
		} else if (wchar) {
			markup = ptr->markup;
			markup.underline = (ptr == self->cursor) && self->cursor_blink_state;
		} else { // else wchar == 0 and not cursor: skip
			markup.underline = false;
		}
		
		
		markup.foreground_color = xvec4_multiply(markup.foreground_color, self->foreground_screen);
		markup.background_color = xvec4_multiply(markup.background_color, self->background_screen);
		
		if (markup.underline) {
			markup.underline_color = self->default_format->underline_color;
			markup.underline_color = xvec4_multiply(markup.underline_color, self->foreground_screen);
		}
		
		if (wchar != 0 || ptr == self->cursor) {
			xpl_text_buffer_add_wchar(self->tb, &pos, &markup, wchar, 0);
		}
	}
	
	xpl_text_buffer_commit(self->tb);
	
	self->rebuild = false;
}

void text_buffer_set_screen_colors(text_buffer_t *self, xvec4 fg_screen, xvec4 bg_screen) {
	self->foreground_screen = fg_screen;
	self->background_screen = bg_screen;
	self->rebuild = true;
}

void text_buffer_update(text_buffer_t *self, double time) {
	if (self == keyboard_bound_buffer) {
		self->cursor_blink_timeout -= time;
		if (self->cursor_blink_timeout < 0.0) {
			self->cursor_blink_timeout = self->cursor_blink_speed;
			self->cursor_blink_state = !self->cursor_blink_state;
			self->rebuild = true;
		}
	}
}

void text_buffer_render(text_buffer_t *self, xmat4 *mvp) {
	if (self->rebuild) {
		rebuild_buffer(self);
	}
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	xvec3 translate_v3 = {{ self->layout.x, self->layout.y, 0.f }};
	xmat4 translate;
	xmat4_identity(&translate);
	xmat4_translate(&translate, &translate_v3, NULL);
	
	xmat4 all;
	xmat4_multiply(mvp, &translate, &all);
	
	xpl_text_buffer_render(self->tb, all.data);
	
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}