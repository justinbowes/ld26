
#include <stdio.h>
#include <stdlib.h>

#include "xpl.h"
#include "xpl_gl_debug.h"
#include "xpl_app.h"
#include "xpl_engine_info.h"
#include "xpl_imui.h"
#include "xpl_vec.h"

static xpl_imui_theme_t *theme = NULL;
static xpl_imui_context_t *ctx = NULL;

static char title_font_family[128] = { 0 };
static char detail_font_family[128] = { 0 };
static float scroll = 0.0f;

static int spacing_open = FALSE;
static int font_open = FALSE;
static int button_open = FALSE;
static int item_open = FALSE;
static int slider_open = FALSE;
static int checkbox_open = FALSE;
static int collapse_open = FALSE;
static int area_open = FALSE;
static int textfield_open = FALSE;
static int separator_open = FALSE;
static int label_open = FALSE;
static int value_open = FALSE;

static int should_exit = FALSE;
static int done = FALSE;

static void init(xpl_app_t *app) {
	xpl_shaders_init("shaders/", ".glsl");
    
    theme = xpl_imui_theme_load_new("ld26");
    ctx = xpl_imui_context_new(theme);
}

static void destroy(xpl_app_t *app) {
}

static int GLFWCALL glfw_window_close_fun() {
    return TRUE;
}

#define HORIZ(w, dx, ay) 	(w).x += (dx); \
                            (w).y = (ay); \
                            xpl_imui_context_area_set(w);

static void font_section(const char *name, char *name_buffer, xpl_markup_t *markup, int *cursor) {
	char *markup_name = &markup->family[0];
	xpl_imui_control_label(name);
    
	xrect w = xpl_imui_context_area_get();
	float initial_x = w.x;
    
	xpl_imui_context_area_mark();
	float shift = w.width / 4.0f;
	w.width = shift - 4.0f;
    
	HORIZ(w, 0, w.y);
	xpl_imui_control_textfield("family name", name_buffer, 128, cursor, NULL, true);
	HORIZ(w, shift, w.y);
	if (xpl_imui_control_button("Apply", 0, strcmp(markup_name, name_buffer))) {
		strcpy(markup_name, name_buffer);
	}
	HORIZ(w, shift, w.y);
	if (xpl_imui_control_button("Revert", 0, strcmp(markup_name, name_buffer))) {
		strcpy(name_buffer, markup_name);
	}
	w = xpl_imui_context_area_get();
    
	w.x = initial_x;
	HORIZ(w, 0, w.y);
	xpl_imui_control_slider("size", &markup->size, 5.0, 48.0, 0.5, true);
	HORIZ(w, shift, w.y);
	if (xpl_imui_control_check("bold", markup->bold, true)) {
		markup->bold = !markup->bold;
	}
	HORIZ(w, shift, w.y);
	if (xpl_imui_control_check("italic", markup->italic, true)) {
		markup->italic = !markup->italic;
	}
    
	float final_y = xpl_imui_context_area_get().y;
	xpl_imui_context_area_restore();
	w = xpl_imui_context_area_get();
	w.x = initial_x;
	HORIZ(w, 0, final_y);
}

static void color_section(const char *label, uint32_t *color, bool allow_disable) {
	xrect w = xpl_imui_context_area_get();
	float initial_y = w.y;
    
	xpl_imui_control_label(label);
	xpl_imui_context_area_mark();
	float shift = w.width / 6.0f;
	w.width = shift - 4.0f;
    
	bool color_enable;
	if (allow_disable) {
		HORIZ(w, 1.5 * shift, initial_y);
		color_enable = *color != NO_COLOR;
		if (xpl_imui_control_check("On", color_enable, true)) {
			if (*color == NO_COLOR) {
				*color = 0xff000000;
				color_enable = true;
			} else {
				*color = NO_COLOR;
				color_enable = false;
			}
		}
        
		HORIZ(w, 0.5f * shift, initial_y);
	} else {
		HORIZ(w, 2.0f * shift, initial_y);
		color_enable = true;
	}
    
	float r = (float)(*color & 0xff);
	float g = (float)((*color >> 8) & 0xff);
	float b = (float)((*color >> 16) & 0xff);
	float a = (float)((*color >> 24) & 0xff);
	float disabled_a = 0;
    
	xpl_imui_control_slider("R", &r, 0.0f, 255.0f, 4.0f, color_enable);
    
	HORIZ(w, shift, initial_y);
	xpl_imui_control_slider("G", &g, 0.0f, 255.0f, 4.0f, color_enable);
    
	HORIZ(w, shift, initial_y);
	xpl_imui_control_slider("B", &b, 0.0f, 255.0f, 4.0f, color_enable);
    
	HORIZ(w, shift, initial_y);
	xpl_imui_control_slider("A", color_enable ? &a : &disabled_a, 0.0f, 255.0f, 4.0f, color_enable);
    
	*color = (((int)r) & 0xff) | ((((int)g) & 0xff) << 8) | ((((int)b) & 0xff) << 16) | ((((int)a) & 0xff) << 24);
    
	xpl_imui_context_area_restore();
}

static void color_CHAD_section(const char *label, _color_chad_t *colors) {
	xpl_imui_control_label(label);
	color_section("cold", &colors->cold, true);
	color_section("hot", &colors->hot, true);
	color_section("active", &colors->active, true);
	color_section("disabled", &colors->disabled, true);
}

static void save_theme() {
	xpl_imui_theme_save(theme, "working");
}

static void render(xpl_engine_execution_info_t *execution_info) {
    static int title_font_cursor;
	static int detail_font_cursor;
	float nullscroll;
    
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	if (done) {
		// poke IMUI
		xpl_imui_context_begin(ctx, execution_info, xrect_set(0, 0, 0, 0));
		xpl_imui_context_end(ctx);
		return;
	}
    
    xrect screen = {{ 0.f, 0.f, execution_info->screen_size.width, execution_info->screen_size.height }};
	xrect panel1 = xrect_set(0.0f, screen.height, execution_info->screen_size.x / 2 - 4.0, execution_info->screen_size.y);
	xrect panel2 = xrect_set(panel1.width + 100.0f, screen.height - 300.0f, 400.0f, 300.0f);
	xpl_imui_context_begin(ctx, execution_info, screen);
	{
		xpl_imui_context_area_set(panel1);
		xpl_imui_control_scroll_area_begin("Configuration", xvec2_set(panel1.width, panel1.height), &scroll);
		{
			if (xpl_imui_control_collapse("Spacing", "theme wide spacing", &spacing_open, true)) {
				xpl_imui_control_slider("default spacing", & theme->default_spacing, 0.0f, 50.0f, 0.5f, true);
				xpl_imui_control_slider("indent size", & theme->indent_size, 0.0f, 50.0f, 0.5f, true);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Fonts", "theme wide fonts", &font_open, true)) {
				font_section("Title font", title_font_family, theme->title_markup, &title_font_cursor);
				font_section("Detail font", detail_font_family, theme->detail_markup, &detail_font_cursor);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Buttons", "button properties", &button_open, true)) {
				xpl_imui_control_slider("height", & theme->button.height, theme->detail_markup->size, 50.0f, 0.5f, true);
				xpl_imui_control_slider("corner radius", & theme->button.corner_radius, 0.0f, theme->button.height / 2, 0.5f, true);
				color_CHAD_section("back color", &theme->button.back.color);
				color_CHAD_section("caption", &theme->button.caption.color);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Item", "clickable list item", &item_open, true)) {
				xpl_imui_control_slider("height", &theme->item.height, theme->detail_markup->size, 50.0f, 0.5f, true);
				color_CHAD_section("back color", &theme->item.back.color);
				color_CHAD_section("value color", &theme->item.value.color);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Slider", "value slider properties", &slider_open, true)) {
				xpl_imui_control_slider("track height", &theme->slider.track.size, theme->detail_markup->size, 50.0f, 0.5f, true);
				xpl_imui_control_slider("track corner radius", &theme->slider.track.corner_radius, 0.0f, theme->slider.track.size / 2, 0.5f, true);
				xpl_imui_control_slider("handle size", &theme->slider.handle.size, 2.0f, 50.0f, 0.5f, true);
				xpl_imui_control_slider("handle corner radius", &theme->slider.handle.corner_radius, 0.0f, theme->slider.handle.size / 2, 0.5f, true);
				color_CHAD_section("handle color", &theme->slider.handle.color);
				color_CHAD_section("value color", &theme->slider.value.color);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Checkbox", "checkbox properties", &checkbox_open, true)) {
				xpl_imui_control_slider("height", &theme->checkbox.height, theme->detail_markup->size, 50.0f, 0.5f, true);
				xpl_imui_control_slider("checkbox check size", &theme->checkbox.check.size, 2.0f, 50.0f, 0.5f, true);
				xpl_imui_control_slider("checkbox check corner radius", &theme->checkbox.check.corner_radius, 0.0f, theme->checkbox.check.size / 2, 0.5f, true);
				color_CHAD_section("checkbox check color", &theme->checkbox.check.color);
				xpl_imui_control_slider("checkbox box margin", &theme->checkbox.box.margin, 0.0f, 20.0f, 0.5f, true);
				xpl_imui_control_slider("checkbox box corner radius", &theme->checkbox.box.corner_radius, 0.0f, theme->checkbox.check.size + theme->checkbox.box.margin * 2, 0.5f, true);
				color_CHAD_section("checkbox box color", &theme->checkbox.box.color);
				color_CHAD_section("checkbox label color", &theme->checkbox.label.color);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Collapsing section", "what you're looking at", &collapse_open, true)) {
				xpl_imui_control_slider("height", &theme->collapse.height, theme->detail_markup->size, 50.0f, 0.5f, true);
				color_CHAD_section("collapse expanded color", &theme->collapse.expose_control.checked.color);
				color_CHAD_section("collapse collapsed color", &theme->collapse.expose_control.unchecked.color);
				color_CHAD_section("collapse title color", &theme->collapse.heading.color);
				color_CHAD_section("collapse subtitle color", &theme->collapse.subheading.color);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Scrollable area", "windows and scroll bars", &area_open, true)) {
				xpl_imui_control_slider("header size", &theme->area.header.size, theme->title_markup->size, 80.0f, 0.5f, true);
				xpl_imui_control_slider("header corner radius", &theme->area.header.corner_radius, 0.0f, theme->area.header.size / 2, 0.5f, true);
				color_section("header back color", &theme->area.header.back_color, FALSE);
				xpl_imui_control_slider("scroll area padding", &theme->area.scroll.padding, 0.0f, 20.0f, 0.5f, true);
				color_CHAD_section("scroll handle color", &theme->area.scroll.handle.color);
				color_section("scroll background", &theme->area.scroll.back_color, FALSE);
				xpl_imui_control_slider("area corner radius", &theme->area.corner_radius, 0.0f, theme->area.header.size / 2, 0.5f, true);
				color_section("area background color", &theme->area.back_color, FALSE);
				color_section("area title color", &theme->area.title_color, FALSE);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Text field", "text input area properties", &textfield_open, true)) {
				xpl_imui_control_slider("border width", &theme->textfield.border.size, 0.0f, 5.0f, 0.5f, true);
				color_CHAD_section("border color", &theme->textfield.border.color);
				color_CHAD_section("text color", &theme->textfield.text.color);
				color_CHAD_section("prompt color", &theme->textfield.prompt.color);
				color_CHAD_section("back color", &theme->textfield.back.color);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Separator", "separator properties", &separator_open, true)) {
				xpl_imui_control_slider("line width", &theme->separator.line_height, 0.5f, 5.0f, 0.5f, true);
				color_section("line color", &theme->separator.color, FALSE);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Label", "label properties", &label_open, true)) {
				xpl_imui_control_slider("height", &theme->label.height, theme->detail_markup->size, 50.0f, 0.5f, true);
				color_section("color", &theme->label.color, FALSE);
			}
			xpl_imui_separator_line(); // ----------------------------------------------------------------
			if (xpl_imui_control_collapse("Value", "generic value display properties", &value_open, true)) {
				xpl_imui_control_slider("height", &theme->value.height, theme->detail_markup->size, 50.0f, 0.5f, true);
				color_section("value color", &theme->value.color, FALSE);
			}
		}
		xpl_imui_control_scroll_area_end();
        
		xpl_imui_context_area_set(panel2);
		xpl_imui_control_scroll_area_begin("Operations", xvec2_set(panel2.width, panel2.height), &nullscroll);
		{
			if (xpl_imui_control_button("Save", 0, true)) {
				save_theme();
			}
			if (xpl_imui_control_button("Close", 0, true)) {
				done = true;
				should_exit = true;
			}
		}
		xpl_imui_control_scroll_area_end();
	}
	xpl_imui_context_end(ctx);
}

static void main_loop(xpl_app_t *app) {
	glfwSwapInterval(app->display_params.is_framelimit ? 1 : 0);
    glfwSetWindowCloseCallback(glfw_window_close_fun);
    
	while(glfwGetWindowParam(GLFW_OPENED) && ! should_exit) {
		glfwGetWindowSize(&app->execution_info->screen_size.x, &app->execution_info->screen_size.y);
		render(app->execution_info);
        glfwSwapBuffers();
    }
    
}

int main(int argc, char *argv[]) {
	xpl_app_t *app = xpl_app_new(argc, argv);
	app->allow_resize = true;
	app->init_func = init;
	app->destroy_func = destroy;
	app->main_loop_func = main_loop;
	app->title = "XPL IMUI editor";
    
	xpl_start_app(app);
    
	return EXIT_SUCCESS;
}
