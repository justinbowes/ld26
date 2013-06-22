#include <assert.h>

#include "xpl_input.h"
#include "xpl_input_ios.h"
#include "xpl_vec2.h"
#include "xpl_log.h"

#define MAX_TOUCHES 1

extern void xpl_input__internal__dispatch_character(int character);

@implementation XPLKeyInputView

-(id) initWithParentView:(UIView *)view {
	if (self = [super init]) {
		self.root_view = view;
		[self.root_view addSubview:self];
		[self setHidden:YES];
	}
	return self;
}

-(void) deleteBackward {
	xpl_input__internal__dispatch_character(XPL_KEY_BACKSPACE);
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

static xivec2 touches[1];
static xivec2 deltas[1];
static bool states[1];
static XPLKeyInputView *key_input;

void xpl_input_ios_init(UIView *view) {
	key_input = [[XPLKeyInputView alloc] initWithParentView: view];
}

void xpl_input_ios_destroy(void) {
	[key_input release];
}

void xpl_input_ios_set_touch_began(CGPoint *point, size_t index) {
	assert(index <= MAX_TOUCHES);
	states[index] = true;
	touches[index].x = point->x;
	touches[index].y = point->y;
}

void xpl_input_ios_set_touch_moved(CGPoint *point, size_t index) {
	assert(index <= MAX_TOUCHES);
	states[index] = true;
	deltas[index].x = point->x - touches[index].x;
	deltas[index].y = point->y - touches[index].y;
	touches[index].x = point->x;
	touches[index].y = point->y;
}

void xpl_input_ios_set_touch_ended(CGPoint *point, size_t index) {
	assert(index <= MAX_TOUCHES);
	states[index] = false;
	touches[index].x = point->x;
	touches[index].y = point->y;
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