/*
 * File:   xpl_imui.h
 * Author: justin
 *
 * Created on November 11, 2012, 11:43 AM
 */

#ifndef XPL_IMUI_H
#define	XPL_IMUI_H

#include <stdint.h>
#include <uthash.h>
#include <utlist.h>

#include "xpl_vao.h"
#include "xpl_vec.h"
#include "xpl_text_buffer.h"
#include "xpl_imui_theme.h"
#include "xpl_engine_info.h"


enum xpl_imui_text_align {
	XPL_IMUI_ALIGN_LEFT = 0,
	XPL_IMUI_ALIGN_CENTER,
	XPL_IMUI_ALIGN_RIGHT

} ;

enum xpl_imui_button_flags {
	XPL_IMUI_BUTTON_DEFAULT = 0x01,
	XPL_IMUI_BUTTON_CANCEL	= 0x02

} ;

#define MAX_QUEUE_SIZE 5000

typedef uint64_t control_id;
static const control_id CONTROL_NONE = 0;

struct xpl_imui_context;
typedef struct xpl_imui_context xpl_imui_context_t;


struct xpl_imui_context *xpl_imui_context_new(xpl_imui_theme_t *theme);
void xpl_imui_context_destroy(struct xpl_imui_context **ppcontext);

void xpl_imui_context_begin(struct xpl_imui_context *context, xpl_engine_execution_info_t *execution_info, xrect area);
void xpl_imui_context_end(struct xpl_imui_context *context);
void xpl_imui_context_area_mark(void);
void xpl_imui_context_area_restore(void);
xrect xpl_imui_context_area_get(void);
void xpl_imui_context_area_set(xrect area);


int xpl_imui_control_scroll_area_begin(const char *name, xvec2 area, float *scroll);
void xpl_imui_control_scroll_area_end(void);

int xpl_imui_control_button(const char *text, int flags, int enabled);
int xpl_imui_control_check(const char *label, int checked, int enabled);
int xpl_imui_control_collapse(const char *title, const char *subtitle, int *checked, int enabled);
int xpl_imui_control_item(const char *text, int enabled);
void xpl_imui_control_label(const char *text);
int xpl_imui_control_slider(const char *text, float *value, float value_min, float value_max, float value_increment, int enabled);
int xpl_imui_control_textfield(const char *prompt, char *mbsinput, size_t input_len, int *cursor_pos, const char *password_char, int enabled);
void xpl_imui_control_value(const char *text);

void xpl_imui_indent(void);
void xpl_imui_outdent(void);
void xpl_imui_indent_custom(float size);
void xpl_imui_outdent_custom(float size);

void xpl_imui_separator(void);
void xpl_imui_separator_line(void);

void xpl_imui_draw_text(xvec2 pos, char *text, int align, uint32_t color);
void xpl_imui_draw_title(xvec2 pos, char *text, int align, uint32_t color);
void xpl_imui_draw_line(xvec4 line, float width, uint32_t color);
void xpl_imui_draw_rect(xrect rect, uint32_t color);
void xpl_imui_draw_rounded_rect(xrect rect, float corner_r, uint32_t color);


#endif	/* XPL_IMUI_H */

