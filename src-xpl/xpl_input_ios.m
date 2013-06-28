#include <assert.h>

#include "xpl_input.h"
#include "xpl_input_ios.h"
#include "xpl_vec2.h"
#include "xpl_log.h"

#define MAX_TOUCHES 11 // fingers + 1, iOS doesn't support cock touches

extern void xpl_input__internal__dispatch_character(int character);

@implementation XPLKeyInputView

-(id) initWithParentView:(UIView *)view {
	if (self = [super init]) {
		_root_view = view;
		_scale = [UIScreen mainScreen].scale;
		[self.root_view addSubview:self];
		[self setHidden:YES];
	}
	return self;
}

-(void) deleteBackward {
	xpl_input__internal__dispatch_character(XPL_CHAR_BACKSPACE);
}

-(BOOL) hasText {
	return YES;
}

-(void) insertText:(NSString *)text {
	size_t length = [text length];
	for (size_t i = 0; i < length; ++i) {
		xpl_input__internal__dispatch_character([text characterAtIndex:i]);
	}
}

-(BOOL) canBecomeFirstResponder {
	return YES;
}

@end

static xivec2 touches[MAX_TOUCHES];
static xivec2 deltas[MAX_TOUCHES];
static bool states[MAX_TOUCHES];
static XPLKeyInputView *key_input;

void xpl_input_ios_init(UIView *view) {
	key_input = [[XPLKeyInputView alloc] initWithParentView: view];
	for (size_t i = 0; i < MAX_TOUCHES; ++i) {
		states[i] = false;
		touches[i] = xivec2_set(0, 0);
		deltas[i] = xivec2_set(0, 0);
	}
}

void xpl_input_ios_destroy(void) {
	[key_input release];
}

void xpl_input_ios_set_touch_began(CGPoint *point) {
	size_t index = 0;
	for (; index < MAX_TOUCHES; ++index) {
		if (! states[index]) break;
	}
	xivec2 touch_pt = {{
		point->x * key_input.scale,
		point->y * key_input.scale
	}};
	assert(index <= MAX_TOUCHES);
	LOG_DEBUG("Touch %zu down at [%d,%d]", index, touch_pt.x, touch_pt.y);
	states[index] = true;
	touches[index] = touch_pt;
}

static size_t find_nearest_touch(xivec2 touch_pt) {
	size_t index = SIZE_T_MAX;
	int max_distance = INT_MAX;
	for (size_t i = 0; i < MAX_TOUCHES; ++i) {
		if (! states[i]) continue;
		int distance = xivec2_length_sq(touch_pt, touches[i]);
		if (distance < max_distance) {
			index = i;
			max_distance = distance;
		}
	}
	assert(index < MAX_TOUCHES);
	return index;
}

void xpl_input_ios_set_touch_moved(CGPoint *point) {
	xivec2 touch_pt = {{
		point->x * key_input.scale,
		point->y * key_input.scale
	}};
	size_t index = find_nearest_touch(touch_pt);
	LOG_DEBUG("Touch %zu moved at [%d,%d]", index, touch_pt.x, touch_pt.y);
	deltas[index].x = touch_pt.x - touches[index].x;
	deltas[index].y = touch_pt.y - touches[index].y;
	touches[index].x = touch_pt.x;
	touches[index].y = touch_pt.y;
}

void xpl_input_ios_set_touch_ended(CGPoint *point) {
	xivec2 touch_pt = {{
		point->x * key_input.scale,
		point->y * key_input.scale
	}};
	size_t index = find_nearest_touch(touch_pt);
	assert(index <= MAX_TOUCHES);
	LOG_DEBUG("Touch %zu up at [%d,%d]", index, touch_pt.x, touch_pt.y);
	states[index] = false;
	touches[index].x = point->x;
	touches[index].y = point->y;
}

bool xpl_input_keyboard_should_auto_focus(void) {
	return false; // annoying on iOS
}

void xpl_input_enable_keyboard(void) {
	LOG_WARN("Keyboard not implemented on this platform");
}

void xpl_input_disable_keyboard(void) {
	LOG_WARN("Keyboard not implemented on this platform");
}

bool xpl_input_key_down(int key) {
	// Not implemented.
	return false;
}

void xpl_input_enable_characters(void) {
	[key_input becomeFirstResponder];
}

void xpl_input_disable_characters(void) {
	[key_input resignFirstResponder];
}

bool xpl_input_mouse_down_in(xirect rect) {
	for (size_t i = 0; i < MAX_TOUCHES; ++i) {
		if (! states[i]) continue;
		if (xirect_in_bounds(rect, touches[i])) return true;
	}
	return false;
}

void xpl_input_get_mouse_position(xivec2 *position) {
	*position = touches[0];
}

void xpl_input_get_mouse_buttons(xpl_mouse_button_state_t *buttons) {
	int v = 0;
	v |= (states[0] ? xmb_left : 0);
	*buttons = v;
}

void xpl_input_get_scroll_deltas(xivec2 *deltas) {
	*deltas = deltas[0];
}