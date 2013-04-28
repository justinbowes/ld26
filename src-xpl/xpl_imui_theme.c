/*
 * File:   xpl_imui_theme.c
 * Author: justin
 *
 * Created on November 22, 2012, 6:19 PM
 */

#include <sys/param.h>

#include "minIni.h"

#include "xpl_color.h"

#include "xpl_imui.h"
#include "xpl_imui_theme.h"

xpl_imui_theme_t *xpl_imui_theme_default_new() {
	xpl_imui_theme_t *t = xpl_alloc_type(xpl_imui_theme_t);

	t->default_spacing = 8.0f;
	t->indent_size = 40.0f;

	t->title_markup = xpl_markup_new();
	xpl_markup_set(t->title_markup, "DroidSansMono", 24.0, FALSE, FALSE,
				xvec4_set(0, 0, 0, 0), xvec4_set(0, 0, 0, 0));

	t->detail_markup = xpl_markup_new();
	xpl_markup_set(t->detail_markup, "DroidSansMono", 16.0, FALSE, FALSE,
				xvec4_set(0, 0, 0, 0), xvec4_set(0, 0, 0, 0));

	t->button.height = 20.0f;
	t->button.corner_radius = 8.0f;
	t->button.back.color.cold = RGBA(0x80, 0x80, 0x80, 0x60);
	t->button.back.color.hot = NO_COLOR;
	t->button.back.color.active = RGBA(0x80, 0x80, 0x80, 0xc0);
	t->button.back.color.disabled = NO_COLOR;
	t->button.caption.color.cold = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->button.caption.color.hot = RGBA(0xff, 0xc0, 0x00, 0xff);
	t->button.caption.color.active = NO_COLOR;
	t->button.caption.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);

	t->item.height = 20.0f;
	t->item.corner_radius = 2.0f;
	t->item.back.color.cold = NO_COLOR;
	t->item.back.color.hot = RGBA(0xff, 0xc0, 0x00, 0x60);
	t->item.back.color.active = RGBA(0xff, 0xc0, 0x00, 0xc0);
	t->item.back.color.disabled = NO_COLOR;
	t->item.value.color.cold = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->item.value.color.hot = NO_COLOR;
	t->item.value.color.active = NO_COLOR;
	t->item.value.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);

	t->slider.height = 20.0f;
	t->slider.track.size = 2.0f;
	t->slider.track.corner_radius = 4.0f;
	t->slider.track.color.cold = RGBA(0x00, 0x00, 0x00, 0x80);
	t->slider.track.color.hot = NO_COLOR;
	t->slider.track.color.active = NO_COLOR;
	t->slider.track.color.disabled = NO_COLOR;
	t->slider.handle.size = 10.0f;
	t->slider.handle.corner_radius = 4.0f;
	t->slider.handle.color.cold = RGBA(0xff, 0xff, 0xff, 0x40);
	t->slider.handle.color.hot = RGBA(0xff, 0xc0, 0x00, 0x80);
	t->slider.handle.color.active = RGBA(0xff, 0xff, 0xff, 0xff);
	t->slider.handle.color.disabled = NO_COLOR;
	t->slider.value.color.cold = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->slider.value.color.hot = RGBA(0xff, 0xc0, 0x00, 0xff);
	t->slider.value.color.active = NO_COLOR;
	t->slider.value.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);

	t->checkbox.height = 20.0f;
	t->checkbox.check.size = 8.0f;
	t->checkbox.check.corner_radius = 2.0f;
	t->checkbox.check.color.cold = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->checkbox.check.color.hot = NO_COLOR;
	t->checkbox.check.color.active = RGBA(0xff, 0xff, 0xff, 0xff);
	t->checkbox.check.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);
	t->checkbox.box.margin = 3.0f;
	t->checkbox.box.corner_radius = 2.0f;
	t->checkbox.box.color.cold = RGBA(0x80, 0x80, 0x80, 0x60);
	t->checkbox.box.color.hot = NO_COLOR;
	t->checkbox.box.color.active = RGBA(0x80, 0x80, 0x80, 0xc0);
	t->checkbox.box.color.disabled = NO_COLOR;
	t->checkbox.label.color.cold = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->checkbox.label.color.hot = RGBA(0xff, 0xc0, 0x00, 0xff);
	t->checkbox.label.color.active = NO_COLOR;
	t->checkbox.label.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);

	t->collapse.height = 20.0f;
	t->collapse.expose_control.checked.color.cold = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->collapse.expose_control.checked.color.hot = NO_COLOR;
	t->collapse.expose_control.checked.color.active = RGBA(0xff, 0xff, 0xff, 0xff);
	t->collapse.expose_control.checked.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);
	t->collapse.expose_control.unchecked.color.cold = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->collapse.expose_control.unchecked.color.hot = NO_COLOR;
	t->collapse.expose_control.unchecked.color.active = RGBA(0xff, 0xff, 0xff, 0xff);
	t->collapse.expose_control.unchecked.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);
	t->collapse.heading.color.cold = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->collapse.heading.color.hot = RGBA(0xff, 0xc0, 0x00, 0xff);
	t->collapse.heading.color.active = NO_COLOR;
	t->collapse.heading.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);
	t->collapse.subheading.color.cold = RGBA(0xff, 0xff, 0xff, 0x80);
	t->collapse.subheading.color.hot = NO_COLOR;
	t->collapse.subheading.color.active = NO_COLOR;
	t->collapse.subheading.color.disabled = NO_COLOR;

	t->area.header.size = 28.0f;
	t->area.header.corner_radius = 4.0f;
	t->area.header.back_color = RGBA(0x00, 0x00, 0x00, 0xc0);
	t->area.header.margin = 4.0f;
	t->area.scroll.padding = 6.0f;
	t->area.scroll.back_color = RGBA(0x00, 0x00, 0x00, 0x80);
	t->area.scroll.handle.color.cold = RGBA(0xff, 0xff, 0xff, 0x40);
	t->area.scroll.handle.color.hot = RGBA(0xff, 0xc0, 0x00, 0xc0);
	t->area.scroll.handle.color.active = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->area.scroll.handle.color.disabled = NO_COLOR;
	t->area.title_color = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->area.back_color = RGBA(0xff, 0xff, 0xff, 0xc0);
	t->area.corner_radius = 4.0f;

	t->textfield.border.size = 2.0f;
	t->textfield.border.color.cold = RGBA(0x80, 0x80, 0x80, 0x60);
	t->textfield.border.color.hot = NO_COLOR;
	t->textfield.border.color.active = RGBA(0x80, 0x80, 0x80, 0xc0);
	t->textfield.border.color.disabled = NO_COLOR;
	t->textfield.back.color.cold = RGBA(0x00, 0x00, 0x00, 0xc0);
	t->textfield.back.color.hot = RGBA(0x00, 0x00, 0x00, 0xc0);
	t->textfield.back.color.active = RGBA(0x00, 0x00, 0x00, 0xc0);
	t->textfield.back.color.disabled = RGBA(0x00, 0x00, 0x00, 0x60);
	t->textfield.text.color.cold = RGBA(0xff, 0xff, 0xff, 0x60);
	t->textfield.text.color.hot = RGBA(0xff, 0xc0, 0x00, 0xff);
	t->textfield.text.color.active = RGBA(0xff, 0xff, 0xff, 0xff);
	t->textfield.text.color.disabled = RGBA(0x80, 0x80, 0x80, 0xc0);
	t->textfield.prompt.color.cold = RGBA(0xff, 0xff, 0xff, 0x60);
	t->textfield.prompt.color.hot = RGBA(0xff, 0xc0, 0x00, 0x60);
	t->textfield.prompt.color.active = RGBA(0xff, 0xff, 0xff, 0x60);
	t->textfield.prompt.color.disabled = RGBA(0x80, 0x80, 0x80, 0x40);

	t->separator.line_height = 1.0f;
	t->separator.color = RGBA(0xff, 0xff, 0xff, 0x20);

	t->label.height = 20.0f;
	t->label.color = RGBA(0xff, 0xff, 0xff, 0xff);

	t->value.height = 20.0f;
	t->value.color = RGBA(0xff, 0xff, 0xff, 0xc0);

	return t;
}

static xpl_imui_theme_t *s_theme = NULL;

void xpl_imui_theme_destroy(xpl_imui_theme_t **pptheme) {
	assert(pptheme);

	xpl_imui_theme_t *theme = *pptheme;
	assert(theme);

	xpl_markup_destroy(&theme->title_markup);
	xpl_markup_destroy(&theme->detail_markup);

	xpl_free(theme);
}

static char s_buf[128];

#define RGBA_FORMAT "#%02x%02x%02x%02x"

static uint32_t ini_getcolori(const char *section, const char *key, uint32_t default_color, const char *resource_file) {
	int r, g, b, a;
	ini_gets(section, key, "", &s_buf[0], 128, resource_file);
	if (sscanf(s_buf, RGBA_FORMAT, &r, &g, &b, &a) == 4) {
		return (r & 0xff) | ((g & 0xff) << 8) | ((b & 0xff) << 16) | ((a & 0xff) << 24);
	}
	// sscanf failure.
	return default_color;
}

static void ini_putcolori(const char *section, const char *key, uint32_t color, const char *resource_file) {
	if (color == NO_COLOR) return;
	int r = color & 0xff;
	int g = (color >> 8) & 0xff;
	int b = (color >> 16) & 0xff;
	int a = (color >> 24) & 0xff;
	snprintf(&s_buf[0], 128, RGBA_FORMAT, r, g, b, a);
	ini_puts(section, key, s_buf, resource_file);
}

#define MKEY_MAKE(x) snprintf(&s_buf[0], 128, "%s_%s", markup_name, x);

static void ini_getmarkup(const char *resource_file, const char *section, const char *markup_name, xpl_markup_t *markup, const xpl_markup_t *defaults) {
	MKEY_MAKE("family");
	ini_gets(section, s_buf, defaults->family, markup->family, XPL_MARKUP_FAMILY_NAME_MAX, resource_file);

	MKEY_MAKE("size");
	markup->size = ini_getf(section, s_buf, defaults->size, resource_file);

	MKEY_MAKE("bold");
	markup->bold = ini_getbool(section, s_buf, defaults->bold, resource_file);

	MKEY_MAKE("italic");
	markup->italic = ini_getbool(section, s_buf, defaults->italic, resource_file);
}

static void ini_putmarkup(const char *resource_file, const char *section, const char *markup_name, xpl_markup_t *markup) {
	MKEY_MAKE("family");
	ini_puts(section, s_buf, markup->family, resource_file);

	MKEY_MAKE("size");
	ini_putf(section, s_buf, markup->size, resource_file);

	MKEY_MAKE("bold");
	ini_puts(section, s_buf, markup->bold ? "true" : "false", resource_file);

	MKEY_MAKE("italic")
	ini_puts(section, s_buf, markup->italic ? "true" : "false", resource_file);
}

#define GETF(F) theme->F = ini_getf(theme_name, #F, s_theme->F, resource)
#define GETCOLOR(F) theme->F = ini_getcolori(theme_name, #F, s_theme->F, resource)
#define GETCHAD(F)	GETCOLOR(F.cold); \
					GETCOLOR(F.hot); \
					GETCOLOR(F.active); \
					GETCOLOR(F.disabled)

static void ini_gettheme(const char *resource, const char *theme_name, xpl_imui_theme_t *theme) {
	if (s_theme == NULL) s_theme = xpl_imui_theme_default_new();

	GETF(default_spacing);
	GETF(indent_size);

	ini_getmarkup(resource, theme_name, "title", theme->title_markup, s_theme->title_markup);
	ini_getmarkup(resource, theme_name, "detail", theme->detail_markup, s_theme->detail_markup);

	GETF(button.height);
	GETF(button.corner_radius);
	GETCHAD(button.back.color);
	GETCHAD(button.caption.color);

	GETF(item.height);
	GETF(item.corner_radius);
	GETCHAD(item.back.color);
	GETCHAD(item.value.color);

	GETF(slider.height);
	GETF(slider.track.size);
	GETF(slider.track.corner_radius);
	GETCHAD(slider.track.color);
	GETF(slider.handle.size);
	GETF(slider.handle.corner_radius);
	GETCHAD(slider.handle.color);
	GETCHAD(slider.value.color);

	GETF(checkbox.height);
	GETF(checkbox.check.size);
	GETF(checkbox.check.corner_radius);
	GETCHAD(checkbox.check.color);
	GETF(checkbox.box.margin);
	GETF(checkbox.box.corner_radius);
	GETCHAD(checkbox.box.color);
	GETCHAD(checkbox.label.color);

	GETF(collapse.height);
	GETCHAD(collapse.expose_control.checked.color);
	GETCHAD(collapse.expose_control.unchecked.color);
	GETCHAD(collapse.heading.color);
	GETCHAD(collapse.subheading.color);

	GETF(area.header.size);
	GETF(area.header.corner_radius);
	GETF(area.header.margin);
	GETCOLOR(area.header.back_color);
	GETF(area.scroll.padding);
	GETCHAD(area.scroll.handle.color);
	GETCOLOR(area.scroll.back_color);
	GETF(area.corner_radius);
	GETCOLOR(area.back_color);
	GETCOLOR(area.title_color);

	GETF(textfield.border.size);
	GETCHAD(textfield.border.color);
	GETCHAD(textfield.text.color);
	GETCHAD(textfield.prompt.color);
	GETCHAD(textfield.back.color);

	GETF(separator.line_height);
	GETCOLOR(separator.color);

	GETF(label.height);
	GETCOLOR(label.color);

	GETF(value.height);
	GETCOLOR(value.color);
}

#define PUTF(F) ini_putf(theme_name, #F, theme->F, resource);
#define PUTCOLOR(F) ini_putcolori(theme_name, #F, theme->F, resource);
#define PUTCHAD(F) \
					PUTCOLOR(F.cold); \
					PUTCOLOR(F.hot); \
					PUTCOLOR(F.active); \
					PUTCOLOR(F.disabled)

static void ini_puttheme(const char *resource, const char *theme_name, xpl_imui_theme_t *theme) {
	PUTF(default_spacing);
	PUTF(indent_size);

	ini_putmarkup(resource, theme_name, "title", theme->title_markup);
	ini_putmarkup(resource, theme_name, "detail", theme->detail_markup);


	PUTF(button.height);
	PUTF(button.corner_radius);
	PUTCHAD(button.back.color);
	PUTCHAD(button.caption.color);

	PUTF(item.height);
	PUTF(item.corner_radius);
	PUTCHAD(item.back.color);
	PUTCHAD(item.value.color);

	PUTF(slider.height);
	PUTF(slider.track.size);
	PUTF(slider.track.corner_radius);
	PUTCHAD(slider.track.color);
	PUTF(slider.handle.size);
	PUTF(slider.handle.corner_radius);
	PUTCHAD(slider.handle.color);
	PUTCHAD(slider.value.color);

	PUTF(checkbox.height);
	PUTF(checkbox.check.size);
	PUTF(checkbox.check.corner_radius);
	PUTCHAD(checkbox.check.color);
	PUTF(checkbox.box.margin);
	PUTF(checkbox.box.corner_radius);
	PUTCHAD(checkbox.box.color);
	PUTCHAD(checkbox.label.color);

	PUTF(collapse.height);
	PUTCHAD(collapse.expose_control.checked.color);
	PUTCHAD(collapse.expose_control.unchecked.color);
	PUTCHAD(collapse.heading.color);
	PUTCHAD(collapse.subheading.color);

	PUTF(area.header.size);
	PUTF(area.header.corner_radius);
	PUTF(area.header.margin);
	PUTCOLOR(area.header.back_color);
	PUTF(area.scroll.padding);
	PUTCHAD(area.scroll.handle.color);
	PUTCOLOR(area.scroll.back_color);
	PUTF(area.corner_radius);
	PUTCOLOR(area.back_color);
	PUTCOLOR(area.title_color);

	PUTF(textfield.border.size);
	PUTCHAD(textfield.border.color);
	PUTCHAD(textfield.text.color);
	PUTCHAD(textfield.prompt.color);
	PUTCHAD(textfield.back.color);

	PUTF(separator.line_height);
	PUTCOLOR(separator.color);

	PUTF(label.height);
	PUTCOLOR(label.color);

	PUTF(value.height);
	PUTCOLOR(value.color);
}

xpl_imui_theme_t *xpl_imui_theme_load_new(const char *name) {
	const char* filename = "imui.ini";
	char resource[PATH_MAX];
	xpl_resolve_resource(resource, filename, PATH_MAX); // can fail; will return default
	xpl_imui_theme_t *theme = xpl_imui_theme_default_new();
	ini_gettheme(resource, name, theme);
	return theme;
}

void xpl_imui_theme_save(xpl_imui_theme_t *self, const char *name) {
	const char* filename = "imui.ini";
	char resource[PATH_MAX];
	xpl_resolve_resource(resource, filename, PATH_MAX);
	ini_puttheme(resource, name, self);
}
