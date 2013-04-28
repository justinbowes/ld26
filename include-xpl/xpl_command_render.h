/*
 * File:   xpl_imui_render.h
 * Author: justin
 *
 * Created on November 12, 2012, 10:00 AM
 */

#ifndef XPL_COMMAND_RENDER_H
#define	XPL_COMMAND_RENDER_H

#include "xpl_vec.h"
#include "xpl_markup.h"

enum xpl_render_cmd_type {
	XPL_RENDER_CMD_INVALID,
	XPL_RENDER_CMD_RECT,
	XPL_RENDER_CMD_TRIANGLE,
	XPL_RENDER_CMD_POLYGON,
	XPL_RENDER_CMD_LINE,
	XPL_RENDER_CMD_TEXT,
	XPL_RENDER_CMD_SCISSOR,
} ;

typedef struct xpl_render_cmd {

	uint8_t type;
	uint8_t flags;
	xmat4 matrix;
	uint8_t pad[2];

	union {

		struct {
			union {
				xrect area;
				xvec4 line;
			};
			uint32_t color;
			float radius;

		} shape;

		struct {

			xpl_markup_t	*markup;
			xvec2			position;
			int				align;
			char			*text;

		} text;

		struct {
			xvec2 *points;
			size_t points_len;
			uint32_t color;
		} polygon;

	} ;

} xpl_render_cmd_t;

typedef struct xpl_font_position {

	float left_offset;
	float right_offset;
	int char_offset;
	
} xpl_font_position_t;

void xpl_imui_render_init(void);
void xpl_imui_render_destroy(void);
void xpl_imui_render_draw(xivec2 *screen, xpl_render_cmd_t *commands, size_t command_length, const float scale, const float blend_amount);
xpl_font_position_t xpl_imui_text_get_position(xpl_markup_t *markup, const char *text, const float offset);
xpl_font_position_t xpl_imui_text_offsets_for_position(xpl_markup_t *ref_markup, const char *text, const int pos);
void xpl_render_cmd_content_reset(xpl_render_cmd_t *cmd);



#endif	/* XPL_COMMAND_RENDER_H */

