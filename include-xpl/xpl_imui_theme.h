/*
 * File:   xpl_imui_theme.h
 * Author: justin
 *
 * Created on November 22, 2012, 6:19 PM
 */

#ifndef XPL_IMUI_THEME_H
#define	XPL_IMUI_THEME_H

#include "xpl_vec.h"
#include "xpl_markup.h"
#include "xpl_color.h"

#define NO_COLOR RGBA(0x00, 0x00, 0x00, 0x01)

typedef struct color_chad {
	uint32_t cold;
	uint32_t hot;
	uint32_t active;
	uint32_t disabled;
} _color_chad_t;

typedef struct xpl_imui_theme {
	float default_spacing;
	float indent_size;

	xpl_markup_t *title_markup;
	xpl_markup_t *detail_markup;

	struct {
		float height;
		float corner_radius;

		struct {
			_color_chad_t color;
		} back;

		struct {
			_color_chad_t color;
		} caption;
	} button;

	struct {
		float corner_radius;
		float height;

		struct {
			_color_chad_t color;
		} back;

		struct {
			_color_chad_t color;
		} value;
	} item;

	struct {
		float height;

		struct {
			float size;
			float corner_radius;
			_color_chad_t color;
		} track;

		struct {
			float size;
			float corner_radius;
			_color_chad_t color;
		} handle;

		struct {
			_color_chad_t color;
		} value;

	} slider;

	struct {
		float height;

		struct {
			float size;
			float corner_radius;
			_color_chad_t color;
		} check;

		struct {
			float margin;
			float corner_radius;
			_color_chad_t color;
		} box;

		struct {
			_color_chad_t color;
		} label;

	} checkbox;

	struct {
		float height;

		struct {

			struct {
				_color_chad_t color;
			} checked;

			struct {
				_color_chad_t color;
			} unchecked;
		} expose_control;

		struct {
			_color_chad_t color;
		} heading;

		struct {
			_color_chad_t color;
		} subheading;
	} collapse;

	struct {
		float corner_radius;
		uint32_t back_color;
		uint32_t title_color;

		struct {
			float size;
			float corner_radius;
			float margin;
			uint32_t back_color;
		} header;

		struct {
			float padding;

			struct {
				_color_chad_t color;
			} handle;

			uint32_t back_color;

		} scroll;

	} area;

	struct {

		struct {
			float size;
			_color_chad_t color;
		} border;

		struct {
			_color_chad_t color;
		} text;

		struct {
			_color_chad_t color;
		} prompt;

		struct {
			_color_chad_t color;
		} back;

	} textfield;

	struct {
		float line_height;
		uint32_t color;
	} separator;

	struct {
		float height;
		uint32_t color;
	} label;

	struct {
		float height;
		uint32_t color;
	} value;

} xpl_imui_theme_t;

xpl_imui_theme_t *xpl_imui_theme_default_new(void);
void xpl_imui_theme_destroy(xpl_imui_theme_t **pptheme);

xpl_imui_theme_t *xpl_imui_theme_load_new(const char *name);
void xpl_imui_theme_save(xpl_imui_theme_t *theme, const char *name);

#endif	/* XPL_IMUI_THEME_H */

