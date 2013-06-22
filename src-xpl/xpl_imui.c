/*
 * File:   xpl_imui.c
 * Author: justin
 *
 * Created on November 11, 2012, 12:54 PM
 */

#include <assert.h>
#include <math.h>

#include <uthash.h>
#include <minIni.h>
#include <sys/param.h>

#include "xpl_gl.h"
#include "xpl_input.h"
#include "xpl_platform.h"
#include "xpl_memory.h"
#include "xpl_vec.h"
#include "xpl_log.h"
#include "xpl_text_buffer.h"
#include "xpl_imui_theme.h"
#include "xpl_command_render.h"
#include "xpl_engine_info.h"
#include "xpl_color.h"

#include "xpl_imui.h"

#define SCROLL_STACK_MAX 10

typedef struct context_scroll {
	control_id id;

	int mouse_in;
	int layout_in;

	xrect clip_area;
	float *val;
} context_scroll_t;

struct xpl_imui_context {

	struct {
		xivec2					ref;
		xivec2					screen;
		xvec3					r2s_xform;
	} bounds;

	struct {
		struct xpl_render_cmd 	queue[MAX_QUEUE_SIZE];
		size_t					length;
	} rq;

	struct {

		struct {

			struct {
				int				down;
				int				pressed;
				int				released;
			} left, right;
		} buttons;

        xivec2				pos;

        struct {
            xivec2			offset;
            xivec2			origin;
        } drag;

		struct {
			int					delta;
			int					pos;
			float				scale;
		} scroll;

	} mouse;

	struct _keyboard {
		int typed[8];

		struct _keystruct {
			int down;
			int active;
		} backspace, cursor_left, cursor_right, tab, left_shift, right_shift, enter, escape, delete;

		int listener_id;
		
	} keyboard;

	struct {
		control_id						active_id;
		control_id						hot_id;
		control_id						hot_to_be_id;

		// The first control with an id >= this will take active.
		control_id						keyboard_active_to_be_id;
		// When 0, the first control that wants the keyboard can set this.
		control_id						keyboard_active_id;

		control_id						area_id;
		control_id						widget_id;

		int								is_hot;
		int								is_active;
		int								went_active;

		xrect							widget_area;
		xrect							widget_area_mark;

		context_scroll_t				*current_scroll;
		int								scroll_frame;
		context_scroll_t				scroll_stack[SCROLL_STACK_MAX];

	} controls;

	float							scale;
	float							blend_amount;

	xpl_imui_theme_t *theme;

} ;

static xpl_imui_context_t *g_context;
static size_t context_count = 0;

XPLINLINE int context_valid() {
	return (g_context != NULL);
}

static void gfx_add_scissor(int enable, xrect area) {
	assert(context_valid());
	if (g_context->rq.length >= MAX_QUEUE_SIZE) return;

	xpl_render_cmd_t *cmd = &(g_context->rq.queue[g_context->rq.length++]);

	cmd->type = XPL_RENDER_CMD_SCISSOR;
	cmd->flags = enable ? 1 : 0;
	cmd->shape.area = area;
}

static void gfx_add_rounded_rect(xrect area, float corner_r, uint32_t color) {
	assert(context_valid());
	if (g_context->rq.length >= MAX_QUEUE_SIZE) return;

	xpl_render_cmd_t *cmd = &(g_context->rq.queue[g_context->rq.length++]);
	cmd->type = XPL_RENDER_CMD_RECT;
	cmd->flags = 0;
	cmd->shape.area = area;
	cmd->shape.radius = corner_r;
	cmd->shape.color = color;
}

static void gfx_add_rect(xrect area, uint32_t color) {
	gfx_add_rounded_rect(area, 0.0f, color);
}

static void gfx_add_line(xvec4 line, float width, uint32_t color) {
	assert(context_valid());
	if (g_context->rq.length >= MAX_QUEUE_SIZE) return;

	xpl_render_cmd_t *cmd = &(g_context->rq.queue[g_context->rq.length++]);
	cmd->type = XPL_RENDER_CMD_LINE;
	cmd->flags = 0;
	cmd->shape.line = line;
	cmd->shape.radius = width;
	cmd->shape.color = color;
}

static void gfx_add_triangle(xrect area, char flags, uint32_t color) {
	assert(context_valid());
	if (g_context->rq.length >= MAX_QUEUE_SIZE) return;

	xpl_render_cmd_t *cmd = &(g_context->rq.queue[g_context->rq.length++]);
	cmd->type = XPL_RENDER_CMD_TRIANGLE;
	cmd->flags = flags;
	cmd->shape.area = area;
	cmd->shape.color = color;
	cmd->shape.radius = 0.0f;
}

static void gfx_add_text(xvec2 pos, const char *text, xpl_markup_t *markup, int align) {
	assert(context_valid());
	if (g_context->rq.length >= MAX_QUEUE_SIZE) return;

	xpl_render_cmd_t *cmd = &(g_context->rq.queue[g_context->rq.length++]);
	cmd->type = XPL_RENDER_CMD_TEXT;
	cmd->flags = 0;
	cmd->text.markup = xpl_markup_new();
	xpl_markup_set(cmd->text.markup, markup->family, markup->size,
				markup->bold, markup->italic,
				markup->foreground_color, markup->background_color);
	cmd->text.position = pos;
	cmd->text.align = align;
	cmd->text.text = strdup(text);
}

// ---------------------------------------------------------------------------

static void control_clear_scroll(context_scroll_t *cs) {
	cs->clip_area = xrect_set(0, 0, 0, 0);
	cs->id = 0;
	cs->layout_in = FALSE;
	cs->mouse_in = FALSE;
	cs->val = NULL;
}

struct xpl_imui_context *xpl_imui_context_new(xpl_imui_theme_t *theme) {
	xpl_imui_context_t *context = xpl_calloc_type(xpl_imui_context_t);
	xpl_imui_render_init();

	context->bounds.ref = xivec2_set(1280.0f, 720.0f);
	context->rq.length = 0;

	context->mouse.buttons.left.down = FALSE;
	context->mouse.buttons.left.pressed = FALSE;
	context->mouse.buttons.left.released = FALSE;

	context->mouse.pos = xivec2_set(0.0f, 0.0f);
	context->mouse.drag.offset = xivec2_set(0.0f, 0.0f);
	context->mouse.drag.origin = xivec2_set(0.0f, 0.0f);
	context->mouse.scroll.scale = 4.0f;
	context->mouse.scroll.pos = 0;
	context->mouse.scroll.delta = 0;

	context->controls.area_id = CONTROL_NONE;
	context->controls.widget_id = CONTROL_NONE;
	context->controls.active_id = CONTROL_NONE;
	context->controls.hot_id = CONTROL_NONE;
	context->controls.hot_to_be_id = CONTROL_NONE;
	context->controls.keyboard_active_id = CONTROL_NONE;
	context->controls.keyboard_active_to_be_id = CONTROL_NONE;

	context->controls.is_hot = FALSE;
	context->controls.is_active = FALSE;
	context->controls.went_active = FALSE;

	context->controls.widget_area = xrect_set(0.0f, 0.0f, 100.0f, 0.0f);

	for (size_t i = 0; i < SCROLL_STACK_MAX; ++i) {
		control_clear_scroll(&context->controls.scroll_stack[i]);
	}
	context->controls.current_scroll = &context->controls.scroll_stack[0];
	context->controls.scroll_frame = 0;

	context->scale = 1.0f;
	context->blend_amount = 2.0f;

	context->theme = theme;
    
    ++context_count;

	return context;
}

void xpl_imui_context_destroy(struct xpl_imui_context **ppcontext) {
	assert(ppcontext);

	xpl_imui_context_t *context = *ppcontext;
	assert(context);

	xpl_free(context);
    
    --context_count;
    if (context_count == 0) xpl_imui_render_destroy();

	*ppcontext = NULL;
}

// ---------------------------------------------------------------------------

XPLINLINE int control_active_any() {
	assert(context_valid());
	return g_context->controls.active_id != CONTROL_NONE;
}

XPLINLINE int control_active(control_id id) {
	assert(context_valid());
	return g_context->controls.active_id == id;
}

XPLINLINE int control_hot(control_id id) {
	assert(context_valid());
	return g_context->controls.hot_id == id;
}

XPLINLINE int mouse_in_rect(xrect rect, int check_scroll) {
	assert(context_valid());
	if (check_scroll &&
		g_context->controls.current_scroll->layout_in &&
		! g_context->controls.current_scroll->mouse_in) {
		return FALSE;
	}
	return xrect_in_boundsi(rect, g_context->mouse.pos);
}

XPLINLINE void input_clear() {
	assert(context_valid());
	// Clear per-frame mouse state.
	g_context->mouse.buttons.left.pressed = FALSE;
	g_context->mouse.buttons.left.released = FALSE;
	g_context->mouse.scroll.delta = 0;

	g_context->keyboard.typed[0] = 0;
	g_context->keyboard.backspace.active = FALSE;
	g_context->keyboard.cursor_left.active = FALSE;
	g_context->keyboard.cursor_right.active = FALSE;
	g_context->keyboard.enter.active = FALSE;
	g_context->keyboard.escape.active = FALSE;
}

XPLINLINE void control_active_clear() {
	assert(context_valid());
	g_context->controls.active_id = CONTROL_NONE;
	input_clear();
}

XPLINLINE void control_set_keyboard_active(control_id id) {
	assert(context_valid());
	g_context->controls.keyboard_active_id = id;
}

XPLINLINE void control_set_active(control_id id) {
	assert(context_valid());
	g_context->controls.active_id = id;
	g_context->controls.went_active = TRUE;
	control_set_keyboard_active(id);
}

XPLINLINE void control_set_hot(control_id id) {
	assert(context_valid());
	g_context->controls.hot_to_be_id = id;
}

static const int TABBED_ARRIVAL = 2;

XPLINLINE int control_active_keyboard(control_id id) {
	assert(context_valid());

	// If the active_to_be_id is not equal to the active_id, the active_id moves in the direction
	// of the active_to_be_id.
	if (g_context->controls.keyboard_active_to_be_id != CONTROL_NONE) {
		LOG_DEBUG("Keyboard active state changing");
		if (g_context->controls.keyboard_active_to_be_id > g_context->controls.keyboard_active_id) {
			if (id >= g_context->controls.keyboard_active_to_be_id) {
				LOG_DEBUG("Control %ud is taking over", (unsigned int)id);
				// Set the active control.
				control_set_keyboard_active(id);
				g_context->controls.keyboard_active_to_be_id = CONTROL_NONE;
				return TABBED_ARRIVAL;
			}
		} else if (g_context->controls.keyboard_active_to_be_id < g_context->controls.keyboard_active_id) {
			// This takes two frames to resolve.
			if (id <= g_context->controls.keyboard_active_to_be_id) {
				// Set the active control.
				control_set_keyboard_active(id);
				LOG_DEBUG("Control %ud is tentatively taking over", (unsigned int)id);
				return TABBED_ARRIVAL;
			} else {
				// Stop retreating.
				g_context->controls.keyboard_active_to_be_id = CONTROL_NONE;
				LOG_DEBUG("Control %ud is ending the retreat, active control is %ud", (unsigned int)id, (unsigned int)g_context->controls.keyboard_active_id);
			}
		}
	} 
	return g_context->controls.keyboard_active_id == id;
}

XPLINLINE void control_set_keyboard_active_advance() {
	assert(context_valid());
	g_context->controls.keyboard_active_id++;
}

XPLINLINE int control_keyboard_active_any() {
	return g_context->controls.keyboard_active_id != CONTROL_NONE;
}

XPLINLINE void control_keyboard_active_clear() {
	g_context->controls.keyboard_active_id = CONTROL_NONE;
}

static int control_logic_button(control_id id, int is_over, int flags) {
	assert(context_valid());

	int result = FALSE;

	// handle mousedown
	if (! control_active_any()) {
		if (is_over) {
			control_set_hot(id);
		}

		if (control_hot(id) && g_context->mouse.buttons.left.down) {
			control_set_active(id);
		}
	}

	if ((flags & XPL_IMUI_BUTTON_DEFAULT) && g_context->keyboard.enter.active) result = TRUE;
	if ((flags & XPL_IMUI_BUTTON_CANCEL) && g_context->keyboard.escape.active) result = TRUE;

	// If active, react on left up
	if ( control_active(id)) {
		g_context->controls.is_active = TRUE;

		if (is_over) {
			control_set_hot(id);
		}

		if (g_context->mouse.buttons.left.released) {
			if (control_hot(id)) {
				// This is a click.
				result = TRUE;
			}
			control_active_clear();
		}
	}

	if (control_hot(id)) {
		g_context->controls.is_hot = TRUE;
	}

	return result;
}

static int control_logic_textfield(char *mbsinput, size_t max_len, int *pcursor_pos) {
	assert(context_valid());
	// Handle characters

	int changed = FALSE;
	int cursor_pos = *pcursor_pos;

	int *typed_ptr = & g_context->keyboard.typed[0];
	while (*typed_ptr) {

		size_t value_len = strlen(mbsinput);
		cursor_pos = xmin(cursor_pos, (int)value_len);

		if (xpl_input_is_character(*typed_ptr)) {

			if (value_len == max_len) {
				LOG_DEBUG("Too long; not adding to field");
				typed_ptr++;
				continue;
			}

			char c = (char)(*typed_ptr & 0xff);
			if (cursor_pos == value_len) {
				// Append is easier than insert
				mbsinput[cursor_pos++] = c;
				mbsinput[cursor_pos] = '\0';
			} else {
				// Insert is a PITA
				char result[max_len];
				strncpy(&result[0], mbsinput, cursor_pos);
				result[cursor_pos++] = c;
				result[cursor_pos] = '\0';
				strcat(&result[0], mbsinput + cursor_pos - 1);
				strcpy(mbsinput, &result[0]);
			}
			changed = TRUE;

		}

		typed_ptr++;
	}

	// Now apply special modifier keys. If you type lots in a single frame, the
	// backspaces all happen at the end. TBD if this is actually a problem.
	// At 2200 fps, it isn't.
	if (g_context->keyboard.cursor_left.active) {
		cursor_pos = xmax(cursor_pos - 1, 0);
	}

	if (g_context->keyboard.cursor_right.active) {
		cursor_pos = xmin(cursor_pos + 1, (int)strlen(mbsinput));
	}

	if (g_context->keyboard.backspace.active) {
		size_t value_len = strlen(mbsinput);
		if (cursor_pos == value_len) {
			mbsinput[cursor_pos - 1] = '\0';
			cursor_pos--;
		} else if (cursor_pos > 0) {
			char result[value_len];
			strncpy(&result[0], mbsinput, cursor_pos - 1);
			strcpy(&result[cursor_pos - 1], mbsinput + cursor_pos);
			strcpy(mbsinput, &result[0]);
			cursor_pos -= 1;
		}
		changed = TRUE;
	}

	if (g_context->keyboard.delete.active) {
		size_t value_len = strlen(mbsinput);
		if (0 <= cursor_pos && cursor_pos < value_len) {
			char result[value_len];
			strncpy(&result[0], mbsinput, cursor_pos);
			strcpy(&result[cursor_pos], mbsinput + cursor_pos + 1);
			strcpy(mbsinput, &result[0]);
		}
		changed = TRUE;
	}

	*pcursor_pos = cursor_pos;
	return changed;
}

static void control_push_scroll() {
	assert(context_valid());
	assert(g_context->controls.scroll_frame + 1 < SCROLL_STACK_MAX);

	g_context->controls.current_scroll++;
	g_context->controls.scroll_frame++;
}

static void control_pop_scroll() {
	assert(context_valid());
	assert(g_context->controls.scroll_frame > 0);

	g_context->controls.current_scroll--;
	g_context->controls.scroll_frame--;
}

#define MAX_WAITING_KEYS 8
static int g_waiting_keys[MAX_WAITING_KEYS + 1];
static size_t g_waiting_key_count;

static bool input_handle_char(int unicode_char, void *context) {
	if (g_waiting_key_count >= MAX_WAITING_KEYS) return false;
	g_waiting_keys[g_waiting_key_count++] = unicode_char;
	return true;
}

static void input_mouse_update() {
	xpl_mouse_button_state_t buttons_down;
	xpl_input_get_mouse_buttons(&buttons_down);
	bool lmb_down = buttons_down & xmb_left;
	xpl_input_get_mouse_position(&g_context->mouse.pos);
	
	// Invert mouse y
	g_context->mouse.pos.y = g_context->bounds.screen.y - g_context->mouse.pos.y;

	g_context->mouse.buttons.left.pressed =  (! g_context->mouse.buttons.left.down) && (  lmb_down);
	g_context->mouse.buttons.left.released = (  g_context->mouse.buttons.left.down) && (! lmb_down);
	g_context->mouse.buttons.left.down = lmb_down;

	if (g_context->mouse.buttons.left.pressed) {
		g_context->mouse.drag.origin = g_context->mouse.pos;
		LOG_TRACE("Mouse pressed; drag origin is %d,%d",
				g_context->mouse.screen.drag.origin.x, g_context->mouse.screen.drag.origin.y);
	} else if (g_context->mouse.buttons.left.down) {
		g_context->mouse.drag.offset =
				xivec2_set(g_context->mouse.pos.x - g_context->mouse.drag.origin.x,
                           g_context->mouse.pos.y - g_context->mouse.drag.origin.y);
		LOG_TRACE("Mouse down; drag offset is %d,%d",
				g_context->mouse.drag.offset.x, g_context->mouse.drag.offset.y);
	} else {
		LOG_TRACE("Mouse up; reset offset");
		g_context->mouse.drag.origin = xivec2_set(0, 0);
		g_context->mouse.drag.offset = xivec2_set(0, 0);
	}

	xivec2 scroll_deltas;
	xpl_input_get_scroll_deltas(&scroll_deltas);
#    ifdef XPL_PLATFORM_OSX
	// OSX scroll convention is backwards, sorry, "natural"
	scroll_deltas.y = -scroll_deltas.y;
#    endif
	g_context->mouse.scroll.delta = scroll_deltas.y;
	g_context->mouse.scroll.pos += scroll_deltas.y;
}

XPLINLINE void key_state_transition(int xpl_keycode, struct _keystruct *keystruct) {
	int key_down = xpl_input_key_down(xpl_keycode);
	keystruct->active = key_down && ! keystruct->down;
	keystruct->down = key_down;
}

static void input_keyboard_update() {
	key_state_transition(XPL_KEY_BACKSPACE, &g_context->keyboard.backspace);
	key_state_transition(XPL_KEY_DEL, &g_context->keyboard.delete);
	key_state_transition(XPL_KEY_LEFT, &g_context->keyboard.cursor_left);
	key_state_transition(XPL_KEY_RIGHT, &g_context->keyboard.cursor_right);
	key_state_transition(XPL_KEY_ENTER, &g_context->keyboard.enter);
	key_state_transition(XPL_KEY_ESC, &g_context->keyboard.escape);
	key_state_transition(XPL_KEY_TAB, &g_context->keyboard.tab);
	key_state_transition(XPL_KEY_LSHIFT, &g_context->keyboard.left_shift);
	key_state_transition(XPL_KEY_RSHIFT, &g_context->keyboard.right_shift);

	if (g_waiting_key_count && g_context->controls.keyboard_active_id != CONTROL_NONE) {
		memcpy(&g_context->keyboard.typed[0], &g_waiting_keys[0], g_waiting_key_count * sizeof (g_waiting_keys[0]));
		g_context->keyboard.typed[g_waiting_key_count] = 0;
	}
	g_waiting_key_count = 0;

	if (g_context->keyboard.tab.active) {
		if (g_context->keyboard.left_shift.down || g_context->keyboard.right_shift.down) {
			if (g_context->controls.keyboard_active_id == CONTROL_NONE) {
				g_context->controls.keyboard_active_to_be_id = 1;
			} else {
				g_context->controls.keyboard_active_to_be_id = g_context->controls.keyboard_active_id - 1;
			}
		} else {
			g_context->controls.keyboard_active_to_be_id = g_context->controls.keyboard_active_id + 1;
		}
		LOG_DEBUG("New target active control: %ud", (unsigned int)g_context->controls.keyboard_active_id);
	}

}

static void input_update() {

	assert(context_valid());

	input_mouse_update();
	input_keyboard_update();
	// LOG_DEBUG("Mouse: %d,%d", g_context->mouse.ref.pos.x, g_context->mouse.ref.pos.y);
}


// ---------------------------------------------------------------------------

XPLINLINE xvec4 premultiplied_colorf_from_rgba(uint32_t rgba) {

	int r = rgba & 0xff;
	int g = (rgba >> 8) & 0xff;
	int b = (rgba >> 16) & 0xff;
	int a = (rgba >> 24) & 0xff;

	const float scale = 1.0f / 256.0f;
	const float color_scale = scale * ((float) a / 256.0f);
	return xvec4_set(r * color_scale, g * color_scale, b * color_scale, a * scale);
}

static void set_markup(xpl_markup_t *markup, int is_title, uint32_t color) {
	assert(context_valid());

	xpl_markup_t *source = is_title ? g_context->theme->title_markup : g_context->theme->detail_markup;
	xpl_markup_set(markup,
				source->family, source->size,
				source->bold, source->italic,
				premultiplied_colorf_from_rgba(color),
				premultiplied_colorf_from_rgba(0));

}

//----------------------------------------------------------------------------

XPLINLINE uint32_t color_chad(control_id id, int enabled, _color_chad_t *colors) {
	assert(context_valid());
	if (! enabled) {
		if (colors->disabled != NO_COLOR) return colors->disabled;
		if (colors->cold != NO_COLOR) return colors->cold;
		if (colors->hot != NO_COLOR) return colors->hot;
		return colors->active;
	}
	if ( control_active(id)) {
		if (colors->active != NO_COLOR) return colors->active;
		if (colors->hot != NO_COLOR) return colors->hot;
		if (colors->cold != NO_COLOR) return colors->cold;
		return colors->disabled;
	}
	if (control_hot(id)) {
		if (colors->hot != NO_COLOR) return colors->hot;
		if (colors->cold != NO_COLOR) return colors->cold;
		if (colors->active != NO_COLOR) return colors->active;
		return colors->disabled;
	}
	// cold
	if (colors->cold != NO_COLOR) return colors->cold;
	if (colors->hot != NO_COLOR) return colors->hot;
	if (colors->active != NO_COLOR) return colors->active;
	return colors->disabled;
}

XPLINLINE control_id control_gen_default_id() {
	assert(context_valid());
	g_context->controls.widget_id++;
	return (g_context->controls.area_id << 16) | g_context->controls.widget_id;
}


//----------------------------------------------------------------------------

void xpl_imui_context_begin(xpl_imui_context_t *context, xpl_engine_execution_info_t *execution_info, xrect area) {
	if (context_valid()) {
		LOG_ERROR("Already have an active context. Call _end() after begin()");
	}
	assert(!g_context);

	g_context = context;

	input_update();
	context->bounds.screen = execution_info->screen_size;

	context->controls.hot_id = context->controls.hot_to_be_id;
	context->controls.hot_to_be_id = CONTROL_NONE;

	context->controls.went_active = FALSE;
	context->controls.is_active = FALSE;
	context->controls.is_hot = FALSE;

	context->controls.widget_area = xrect_set(area.x, area.y + area.height, area.width, area.height);

	context->controls.area_id = 1;
	context->controls.widget_id = 1;

	for (size_t i = 0; i < g_context->rq.length; ++i) {
		xpl_render_cmd_content_reset(& g_context->rq.queue[i]);
	}
	g_context->rq.length = 0;


}

void xpl_imui_context_area_mark() {
	assert(context_valid());
	g_context->controls.widget_area_mark = g_context->controls.widget_area;
}

xrect xpl_imui_context_area_get() {
	assert(context_valid());
	return g_context->controls.widget_area;
}

void xpl_imui_context_area_set(xrect context_area) {
	assert(context_valid());
	g_context->controls.widget_area = context_area;
}

void xpl_imui_context_area_restore() {
	assert(context_valid());
	g_context->controls.widget_area = g_context->controls.widget_area_mark;
}

void xpl_imui_context_reset_focus(void) {
	assert(context_valid());
	g_context->controls.keyboard_active_id = CONTROL_NONE;
	g_context->controls.keyboard_active_to_be_id = 1;
}

void xpl_imui_context_end(xpl_imui_context_t *context) {
	if (! context_valid()) {
		LOG_ERROR("No context active!");
	}
	assert (context_valid());

	if (g_context != context) {
		LOG_ERROR("Passed context to end is not the active context!");
	}
	assert (g_context == context);

	// If we didn't end the scroll area, the scissor test stays on.
	assert(g_context->controls.scroll_frame == 0);

	xpl_imui_render_draw(&g_context->bounds.screen, g_context->rq.queue, g_context->rq.length, g_context->scale, g_context->blend_amount);

	input_clear();
	if (g_context->controls.keyboard_active_id && g_context->controls.went_active) {
		g_context->keyboard.listener_id = xpl_input_add_character_listener(input_handle_char, g_context);
	} else if (! g_context->controls.keyboard_active_id) {
		if (g_context->keyboard.listener_id) xpl_input_remove_character_listener(g_context->keyboard.listener_id);
		g_context->keyboard.listener_id = 0;
	}
	if (g_context->controls.keyboard_active_to_be_id > g_context->controls.widget_id) {
		LOG_DEBUG("Keyboard target is too high, stopping advance");
		g_context->controls.keyboard_active_to_be_id = CONTROL_NONE;
	}
	g_context = NULL;
}

//----------------------------------------------------------------------------

int xpl_imui_control_scroll_area_begin(const char *title, xvec2 area, float *scroll) {
	assert(context_valid());

	g_context->controls.area_id++;
	g_context->controls.widget_id++;
	control_push_scroll();
	g_context->controls.current_scroll->id = (g_context->controls.area_id << 16) | g_context->controls.widget_id;

	xrect rect = {{
		g_context->controls.widget_area.x,
		g_context->controls.widget_area.y - area.y,
		area.width,
		area.height
	}};
	xrect header = {{
		rect.x + g_context->theme->area.header.margin,
		rect.y + rect.height - g_context->theme->area.header.size - g_context->theme->area.header.margin,
		rect.width - 2 * g_context->theme->area.header.margin,
		g_context->theme->area.header.size
	}};
	
	if (! (title && title[0])) {
		header.height = 0.0f;
	}

	// height reduced by 3 * padding: above title, between title and content, bottom.
	xrect client_area = {{
		rect.x + g_context->theme->area.scroll.padding,
		rect.y + g_context->theme->area.scroll.padding,
		rect.width - g_context->theme->area.scroll.padding * 5,
		rect.height - header.height - 3 * g_context->theme->area.scroll.padding
	}};
	//	LOG_DEBUG("Client area: %f %f %f %f", client_area.x, client_area.y, client_area.width, client_area.height);

	g_context->controls.widget_area = client_area;
	// Shift down by scroll plus one because we seem to clip a pixel at the top.
	g_context->controls.widget_area.y = client_area.y + client_area.height + (*scroll);
	//	LOG_DEBUG("Widget area: %f %f %f %f",
	//			g_context->controls.widget_area.x, g_context->controls.widget_area.y,
	//			g_context->controls.widget_area.width, g_context->controls.widget_area.height);

	// Clip area (scissor) is apparently exclusive, so expand it by one pixel all round.
	g_context->controls.current_scroll->clip_area = client_area;
	g_context->controls.current_scroll->clip_area.x -= 1;
	g_context->controls.current_scroll->clip_area.y -= 1;
	g_context->controls.current_scroll->clip_area.width += 2;
	g_context->controls.current_scroll->clip_area.height += 2;

	g_context->controls.current_scroll->val = scroll;
	g_context->controls.current_scroll->mouse_in = mouse_in_rect(rect, FALSE);

	gfx_add_rounded_rect(rect, g_context->theme->area.corner_radius, g_context->theme->area.back_color);
	if (title && title[0]) {
		gfx_add_rounded_rect(header, g_context->theme->area.header.corner_radius, g_context->theme->area.header.back_color);
		xpl_markup_t markup;
		set_markup(&markup, TRUE, g_context->theme->area.title_color);
        xvec2 text_rect = xvec2_set(rect.x + g_context->theme->area.header.size / 2,
                                    rect.y + rect.height - g_context->theme->area.header.size / 2 - markup.size * 0.75f);
		gfx_add_text(text_rect,
                     title,
                     &markup,
                     XPL_IMUI_ALIGN_LEFT);
	}

	gfx_add_scissor(TRUE, g_context->controls.current_scroll->clip_area);

	g_context->controls.current_scroll->layout_in = TRUE;
	return g_context->controls.current_scroll->mouse_in;
}

void xpl_imui_control_scroll_area_end() {
	assert(context_valid());

	gfx_add_scissor(FALSE, xrect_set(0, 0, 0, 0));

	xrect clip_area = g_context->controls.current_scroll->clip_area;
	xrect scroll_bar_area = {{
		clip_area.x + clip_area.width + g_context->theme->area.scroll.padding,
		clip_area.y,
		g_context->theme->area.scroll.padding * 2,
		clip_area.height
	}};
	//	LOG_DEBUG("Scroll bar area: %f %f %f %f",
	//			scroll_bar_area.x, scroll_bar_area.y, scroll_bar_area.width, scroll_bar_area.height);


	float *val = g_context->controls.current_scroll->val;
	const float content_top = (clip_area.y + *val) + clip_area.height;
	const float content_bottom = g_context->controls.widget_area.y; // widget layout advance
	const float content_height = content_top - content_bottom;
	const float max_offset = content_height - clip_area.height;

	if (max_offset > 0) {
		// Scrolling logic
		control_id handle_id = g_context->controls.current_scroll->id;

		const float visible_frac = fmaxf(0.0f, fminf(1.0f, clip_area.height / content_height));
		assert(0 <= visible_frac && visible_frac <= 1);
		// LOG_DEBUG("Content is %f%% visible", visible_frac * 100);

		const float scroll_frac = fminf(fmaxf(*val / max_offset, 0.0f), 1.0f);
		// val may be out of sync if the controls drawn within just changed
		// assert(0 <= scroll_frac  && scroll_frac <= 1);
		// LOG_DEBUG("Content is %f%% scrolled", scroll_frac * 100);

		const float track_height = scroll_bar_area.height;
		const float handle_height = visible_frac * scroll_bar_area.height;

		const float max_handle_offset = track_height - handle_height;
		const float handle_offset = (1 - scroll_frac) * max_handle_offset;
		const float handle_y =  scroll_bar_area.y + handle_offset;

		xrect handle_area = {{
			scroll_bar_area.x, handle_y,
			scroll_bar_area.width, handle_height
		}};

		int is_over = mouse_in_rect(handle_area, FALSE);
		control_logic_button(handle_id, is_over, 0);
		if (control_active(handle_id) && (g_context->controls.went_active || g_context->mouse.drag.offset.y)) {
			float mouse_handle_offset = scroll_bar_area.y + track_height - (handle_height / 2) - g_context->mouse.pos.y;
			LOG_DEBUG("New handle offset: %f", mouse_handle_offset);
			float u = mouse_handle_offset / max_handle_offset;
			if (u < 0) u = 0;
			if (u > 1) u = 1;
			*val = u * max_offset;
			LOG_DEBUG("Scroll val: %f from u=%f", *val, u);
		}

		// Draw bar
		// Background
		gfx_add_rounded_rect(scroll_bar_area, scroll_bar_area.width / 2 - 1,
							g_context->theme->area.scroll.back_color);
		gfx_add_rounded_rect(handle_area, handle_area.width / 2 - 1,
							color_chad(handle_id, TRUE, & g_context->theme->area.scroll.handle.color));

		if (g_context->controls.current_scroll->mouse_in) {
			if (g_context->mouse.scroll.delta) {
				LOG_DEBUG("Scrolling via wheel, delta %d", g_context->mouse.scroll.delta);
				*val += g_context->mouse.scroll.scale * g_context->mouse.scroll.delta;

				if (*val < 0) *val = 0;
				if (*val > max_offset) *val = max_offset;
			}
		}
	} else {
		*val = 0;
	}
	g_context->controls.widget_area.y = g_context->controls.current_scroll->clip_area.y + g_context->theme->default_spacing;
	control_pop_scroll();
}

int xpl_imui_control_button(const char *text, int flags, int enabled) {
	control_id id = control_gen_default_id();
	xrect button_area = {{
		g_context->controls.widget_area.x,
		g_context->controls.widget_area.y - g_context->theme->button.height,
		g_context->controls.widget_area.width,
		g_context->theme->button.height
	}};
	g_context->controls.widget_area.y -= g_context->theme->button.height + g_context->theme->default_spacing;

	int is_over = enabled && mouse_in_rect(button_area, TRUE);
	int result = enabled && control_logic_button(id, is_over, flags);

	gfx_add_rounded_rect(button_area, g_context->theme->button.corner_radius,
						color_chad(id, enabled, &g_context->theme->button.back.color));
	if (text && text[0]) {

		xpl_markup_t label;
		set_markup(&label, FALSE, color_chad(id, enabled, &g_context->theme->button.caption.color));
		gfx_add_text(xvec2_set(button_area.x + g_context->theme->button.height / 2,
							button_area.y + g_context->theme->button.height / 2 - label.size / 2),
					text,
					&label,
					XPL_IMUI_ALIGN_LEFT);
	}
	return result;
}

int xpl_imui_control_item(const char *text, int enabled) {
	control_id id = control_gen_default_id();
	xrect item_area = {{
		g_context->controls.widget_area.x,
		g_context->controls.widget_area.y - g_context->theme->item.height,
		g_context->controls.widget_area.width,
		g_context->theme->item.height
	}};
	g_context->controls.widget_area.y -= g_context->theme->item.height + g_context->theme->default_spacing;

	int is_over = enabled && mouse_in_rect(item_area, TRUE);
	int result = enabled && control_logic_button(id, is_over, 0);

	if (control_hot(id)) {
		gfx_add_rounded_rect(item_area, g_context->theme->item.corner_radius,
							color_chad(id, enabled, &g_context->theme->item.back.color));
	}

	if (text && text[0]) {

		xpl_markup_t label;
		set_markup(&label, FALSE, color_chad(id, enabled, &g_context->theme->item.value.color));
		gfx_add_text(xvec2_set(item_area.x + g_context->theme->item.height / 2,
							item_area.y + g_context->theme->item.height / 2 - label.size / 2),
					text,
					&label,
					XPL_IMUI_ALIGN_LEFT);
	}

	return result;
}

int xpl_imui_control_check(const char *label, int checked, int enabled) {
	control_id id = control_gen_default_id();
	xrect control_area = {{
		g_context->controls.widget_area.x,
		g_context->controls.widget_area.y - g_context->theme->checkbox.height,
		g_context->controls.widget_area.width,
		g_context->theme->checkbox.height
	}};
	g_context->controls.widget_area.y -= g_context->theme->checkbox.height + g_context->theme->default_spacing;

	int is_over = enabled && mouse_in_rect(control_area, TRUE);
	int result = enabled && control_logic_button(id, is_over, 0);

	xrect check_area = {{
		control_area.x + g_context->theme->checkbox.height / 2 - g_context->theme->checkbox.check.size / 2,
		control_area.y + g_context->theme->checkbox.height / 2 - g_context->theme->checkbox.check.size / 2,
		g_context->theme->checkbox.check.size,
		g_context->theme->checkbox.check.size
	}};
	xrect check_box_area = {{
		check_area.x - g_context->theme->checkbox.box.margin,
		check_area.y - g_context->theme->checkbox.box.margin,
		check_area.width + 2 * g_context->theme->checkbox.box.margin,
		check_area.height + 2 * g_context->theme->checkbox.box.margin
	}};
	gfx_add_rounded_rect(check_box_area, g_context->theme->checkbox.box.margin + 1,
						color_chad(id, enabled, &g_context->theme->checkbox.box.color));
	if (checked) {
		gfx_add_rounded_rect(check_area, g_context->theme->checkbox.check.size / 2 - 1,
							color_chad(id, enabled, &g_context->theme->checkbox.check.color));
	}

	if (label && label[0]) {

		xpl_markup_t label_markup;
		set_markup(&label_markup, FALSE,
				color_chad(id, enabled, &g_context->theme->checkbox.label.color));
		xvec2 label_pos = xvec2_set(control_area.x + g_context->theme->checkbox.height,
									control_area.y + g_context->theme->checkbox.height / 2 - label_markup.size / 2);
		gfx_add_text(label_pos, label, &label_markup, XPL_IMUI_ALIGN_LEFT);
	}

	return result;
}

int xpl_imui_control_collapse(const char *title, const char *subtitle, int *checked, int enabled) {
	control_id id = control_gen_default_id();

	// Determine formatting for collapse header
	float header_size = g_context->theme->collapse.height;
	xpl_markup_t title_markup;
	xpl_markup_t subtitle_markup;
	if (title && title[0]) {
		set_markup(&title_markup, TRUE, color_chad(id, enabled, &g_context->theme->collapse.heading.color));
		header_size = fmaxf(header_size, title_markup.size);
	}

	if (subtitle && subtitle[0]) {
		set_markup(&subtitle_markup, FALSE, color_chad(id, enabled, &g_context->theme->collapse.subheading.color));
		header_size = fmaxf(header_size, subtitle_markup.size);
	}

	header_size += g_context->theme->default_spacing;

	xrect control_area = {{
		g_context->controls.widget_area.x,
		g_context->controls.widget_area.y - header_size,
		g_context->controls.widget_area.width,
		header_size
	}};
	g_context->controls.widget_area.y -= header_size + g_context->theme->default_spacing;

	xrect check_area = {{
		control_area.x + g_context->theme->collapse.height * 0.5f - g_context->theme->checkbox.check.size * 0.5f,
		control_area.y + header_size * 0.5f - g_context->theme->checkbox.check.size * 0.5f,
		g_context->theme->checkbox.check.size,
		g_context->theme->checkbox.check.size
	}};

	int is_over = enabled && mouse_in_rect(control_area, TRUE);
	int result = enabled && control_logic_button(id, is_over, 0);

	_color_chad_t *chad = *checked ? &g_context->theme->collapse.expose_control.checked.color :
			&g_context->theme->collapse.expose_control.unchecked.color;
	gfx_add_triangle(check_area, *checked ? 2 : 1,
					color_chad(id, enabled, chad));
	if (title && title[0]) {
		gfx_add_text(xvec2_set(control_area.x + g_context->theme->collapse.height,
							control_area.y + header_size * 0.5f - title_markup.size * 0.5f),
					title,
					&title_markup,
					XPL_IMUI_ALIGN_LEFT);
	}

	if (subtitle && subtitle[0]) {
		gfx_add_text(xvec2_set(control_area.x + control_area.width - g_context->theme->collapse.height / 2,
							control_area.y + header_size * 0.5f - subtitle_markup.size * 0.5f),
					subtitle,
					&subtitle_markup,
					XPL_IMUI_ALIGN_RIGHT);
	}

	if (result) {
		LOG_DEBUG("Collapse toggled");
		*checked = !(*checked);
		LOG_DEBUG("Checked: %s", *checked ? "true" : "false");
	}
	return *checked;
}

void xpl_imui_control_label(const char *text) {

	assert(context_valid());
	xvec2 v = xvec2_set(g_context->controls.widget_area.x,
						g_context->controls.widget_area.y - g_context->theme->label.height);

	xpl_markup_t markup;
	set_markup(&markup, FALSE, g_context->theme->label.color);
    
    const char *label_text = (text ? text : "");
	gfx_add_text(v, label_text, &markup, XPL_IMUI_ALIGN_LEFT);

	g_context->controls.widget_area.y -= markup.size + g_context->theme->default_spacing;
}

void xpl_imui_control_value(const char *text) {

	assert(context_valid());

	xpl_markup_t markup;
	set_markup(&markup, FALSE, g_context->theme->value.color);

	xvec2 v = xvec2_set(g_context->controls.widget_area.x + g_context->controls.widget_area.width - g_context->theme->value.height / 2,
						g_context->controls.widget_area.y - g_context->theme->value.height / 2 - markup.size / 2);
	g_context->controls.widget_area.y -= g_context->theme->value.height; // No padding.

    const char *value_text = (text ? text : "");
	gfx_add_text(v, value_text, &markup, XPL_IMUI_ALIGN_RIGHT);

}

int xpl_imui_control_slider(const char *text, float *value, float value_min, float value_max, float value_increment, int enabled) {
	control_id id = control_gen_default_id();

	xrect control_area = {{
		g_context->controls.widget_area.x,
		g_context->controls.widget_area.y - g_context->theme->slider.height,
		g_context->controls.widget_area.width,
		g_context->theme->slider.height
	}};
	g_context->controls.widget_area.y -= g_context->theme->slider.height + g_context->theme->default_spacing;

	gfx_add_rounded_rect(control_area, g_context->theme->slider.track.corner_radius,
						color_chad(id, enabled, &g_context->theme->slider.track.color));

	const float max_handle_offset = control_area.width - g_context->theme->slider.handle.size;

	float u = (*value - value_min) / (value_max - value_min);
	if (u < 0) u = 0;
	if (u > 1) u = 1;
	float handle_offset = u * max_handle_offset;

	xrect handle_area = {{
		control_area.x + handle_offset,
		control_area.y,
		g_context->theme->slider.handle.size,
		g_context->theme->slider.height
	}};
	int is_over = enabled && mouse_in_rect(handle_area, TRUE);
	int result = enabled && control_logic_button(id, is_over, 0);
	int value_changed = FALSE;

	if (control_active(id) && (g_context->controls.went_active || g_context->mouse.drag.offset.x)) {
		float mouse_handle_offset = g_context->mouse.pos.x - control_area.x - handle_area.width / 2;
		LOG_DEBUG("New handle offset: %f", mouse_handle_offset);
		u = mouse_handle_offset / max_handle_offset;
		if (u < 0) u = 0;
		if (u > 1) u = 1;
		*value = value_min + u * (value_max - value_min);

		// Snap by value_increment
		*value = floorf(*value / value_increment + 0.5f) * value_increment;

		// Re-limit in case the limits aren't a multiple of the increment
		*value = fminf(value_max, *value);
		*value = fmaxf(value_min, *value);
		handle_offset = u * max_handle_offset;
		value_changed = TRUE;
	}

	gfx_add_rounded_rect(handle_area, g_context->theme->slider.handle.corner_radius,
						color_chad(id, enabled, &g_context->theme->slider.handle.color));

	int digits = (int)(ceilf(log10f(value_increment)));
	char format[16] = {0};
	snprintf(format, 16, "%%.%df", digits ? 0 : -digits);
	char value_text[128] = {0};
	snprintf(value_text, 128, format, *value);

	xpl_markup_t markup;
	set_markup(&markup, FALSE, color_chad(id, enabled, &g_context->theme->slider.value.color));
	if (text && text[0]) {

		xvec2 text_pos = xvec2_set(control_area.x + g_context->theme->slider.height / 2,
								control_area.y + g_context->theme->slider.height / 2 - markup.size / 2);
		gfx_add_text(text_pos, text, &markup, XPL_IMUI_ALIGN_LEFT);
	}

	xvec2 value_pos = xvec2_set(control_area.x + control_area.width - g_context->theme->slider.height / 2,
								control_area.y + g_context->theme->slider.height / 2 - markup.size / 2);
	gfx_add_text(value_pos, value_text, &markup, XPL_IMUI_ALIGN_RIGHT);

	return result || value_changed;
}

int xpl_imui_control_textfield(const char *prompt, char *mbsinput, size_t input_len, int *cursor_pos, const char *password_char, int enabled) {
	assert(context_valid());
	control_id id = control_gen_default_id();

	// Configure for input
	// We need the height of the input to lay out the element
	// We also need whatever input we have in wchar format to
	// calculate cursors
	xpl_markup_t text_markup;
	set_markup(&text_markup, FALSE, 0);
	char text[256] = { 0 };

	// Copy user text to output buffer
	if (password_char && password_char[0]) {
		for (size_t i = 0; i < strlen(mbsinput); ++i) {
			strcat(&text[0], &password_char[0]);
		}
	} else {
		snprintf(&text[0], 200, "%s", mbsinput);
	}

	const float height = text_markup.size + 2 * g_context->theme->textfield.border.size;

	xrect control_area = {{
		g_context->controls.widget_area.x,
		g_context->controls.widget_area.y - height,
		g_context->controls.widget_area.width,
		height
	}};
	g_context->controls.widget_area.y -= control_area.height + g_context->theme->default_spacing;

	xrect text_area = {{
		control_area.x + g_context->theme->textfield.border.size,
		control_area.y + g_context->theme->textfield.border.size,
		control_area.width - 2 * g_context->theme->textfield.border.size,
		text_markup.size
	}};

	int is_over = enabled && mouse_in_rect(control_area, TRUE);
	int result = enabled && control_logic_button(id, is_over, 0);
	int value_changed = FALSE;

	if (control_active(id)) {
		control_set_keyboard_active(id);
	}

	int has_keyboard_focus = control_active_keyboard(id);

	if (has_keyboard_focus && enabled) {
		value_changed = control_logic_textfield(mbsinput, input_len, cursor_pos);
	}

	xpl_font_position_t position_info;
	if (g_context->controls.went_active) {
		if (has_keyboard_focus == TABBED_ARRIVAL) {
			// Went active via TAB/S-TAB
			*cursor_pos = (int)strlen(mbsinput);
		} else {
			// Active via click
			float mouse_offset = g_context->mouse.pos.x - text_area.x;
			position_info = xpl_imui_text_get_position(&text_markup, text, mouse_offset);
			*cursor_pos = position_info.char_offset;
		}
	} else {
		if (*cursor_pos < 0) *cursor_pos = 0;
		if (*cursor_pos > strlen(mbsinput) + 1) *cursor_pos = (int)strlen(mbsinput) + 1;
		position_info = xpl_imui_text_offsets_for_position(&text_markup, text, *cursor_pos);
	}
	if (position_info.left_offset == position_info.right_offset) {
		position_info.right_offset += text_markup.size * 0.5f;
	}

	if (has_keyboard_focus) {
		control_set_hot(id);
	}

	gfx_add_rounded_rect(control_area, 2 * g_context->theme->textfield.border.size,
						color_chad(id, enabled, &g_context->theme->textfield.border.color));

	gfx_add_rect(text_area,
				color_chad(id, enabled, &g_context->theme->textfield.back.color));


	// Do we have any text to work with?
	const int has_input = mbsinput && mbsinput[0];
	const int has_prompt = prompt && prompt[0];
	if (has_prompt || has_input) {
		// Choose between prompt and input
		if (has_input) {
			// Configure for user text
			set_markup(&text_markup, FALSE, color_chad(id, enabled,
													&g_context->theme->textfield.text.color));
		} else if (has_keyboard_focus) {
			// Clear prompt when active.
			strncpy(&text[0], "", 200);
		} else {
			// Show prompt when inactive.
			set_markup(&text_markup, FALSE, color_chad(id, enabled,
													&g_context->theme->textfield.prompt.color));
			// Copy prompt to output buffer.
			strncpy(&text[0], prompt, 200);
		}
		gfx_add_text(xvec2_set(text_area.x, text_area.y), text, &text_markup, XPL_IMUI_ALIGN_LEFT);
	}
	if (has_keyboard_focus) {
		// Draw cursor

		gfx_add_line(xvec4_set(text_area.x + position_info.left_offset, text_area.y,
							text_area.x + position_info.right_offset, text_area.y), 2.0f, 0xFF00FFFF);

	}

	return result || value_changed;
}

void xpl_imui_indent() {

	xpl_imui_indent_custom(g_context->theme->indent_size);
}

void xpl_imui_indent_custom(float amount) {

	assert(context_valid());
	g_context->controls.widget_area.x += amount;
	g_context->controls.widget_area.width -= amount;
}

void xpl_imui_outdent() {

	xpl_imui_outdent_custom(g_context->theme->indent_size);
}

void xpl_imui_outdent_custom(float amount) {

	assert(context_valid());
	g_context->controls.widget_area.x -= amount;
	g_context->controls.widget_area.width += amount;
}

void xpl_imui_separator() {

	assert(context_valid());
	g_context->controls.widget_area.y -= g_context->theme->default_spacing * 3;
}

void xpl_imui_separator_line() {

	assert(context_valid());

	xrect area = {{
		g_context->controls.widget_area.x,
		g_context->controls.widget_area.y - g_context->theme->default_spacing * 2,
		g_context->controls.widget_area.width,
		g_context->theme->separator.line_height
	}};
	g_context->controls.widget_area.y -= g_context->theme->default_spacing * 4;

	gfx_add_rect(area, g_context->theme->separator.color);
}

void xpl_imui_draw_text(xvec2 pos, char *text, int align, uint32_t color) {

	assert(context_valid());

	xpl_markup_t markup;
	set_markup(&markup, FALSE, color);

	gfx_add_text(pos, text, &markup, align);
}

void xpl_imui_draw_title(xvec2 pos, char *text, int align, uint32_t color) {

	assert(context_valid());

	xpl_markup_t markup;
	set_markup(&markup, TRUE, color);

	gfx_add_text(pos, text, &markup, align);
}

void xpl_imui_draw_line(xvec4 line, float width, uint32_t color) {

	gfx_add_line(line, width, color);
}

void xpl_imui_draw_rect(xrect rect, uint32_t color) {
	gfx_add_rect(rect, color);
}

void xpl_imui_draw_rounded_rect(xrect rect, float corner_r, uint32_t color) {
	gfx_add_rounded_rect(rect, corner_r, color);
}
