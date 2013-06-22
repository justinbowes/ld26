/*
 * context_menu.c
 *
 *  Created on: 2013-01-04
 *      Author: Justin
 */

#include "xpl.h"
#include "xpl_gl.h"
#include "xpl_imui.h"
#include "xpl_l10n.h"
#include "xpl_context.h"
#include "xpl_app_params.h"
#include "xpl_hash.h"
#include "xpl_rand.h"
#include "xpl_sprite.h"

#include "context/context_logo.h"
#include "context/context_game.h"
#include "models/plane-xy.h"
#include "audio/audio.h"

#include "context/context_menu.h"

#include "game/prefs.h"

typedef void(* menu_func)(xpl_app_t *app, xrect area);

#define MENU_STACK_MAX  64
static menu_select_t                    menu_stack[MENU_STACK_MAX];
static int                              menu_index = -1;

// Config menu
static float                            configure_scroll = 0.f;
static xpl_app_params_t                 app_config;
#ifndef XPL_PLATFORM_IOS
#define MAX_VID_MODES   32
static GLFWvidmode                      supported_video_modes[MAX_VID_MODES];
static size_t                           supported_video_mode_count;
static size_t                           selected_video_mode_index = -1;
static int								preferences_is_expanded = 0;
static int                              resolution_is_expanded = 0;
static bool                             video_config_changed = false;
#endif

static xpl_imui_theme_t                 *imui_theme;
static xpl_imui_context_t               *imui_context;
static xpl_context_t                    *context_next;

// Background
// static xpl_shader_t                     *background_shader;
static xpl_shader_t                     *overlay_shader;
static xpl_vao_t                        *effect_vao;
static xpl_bo_t                         *effect_vbo;
static size_t                           effect_elements;
static xvec4                            overlay_color;
static float                            overlay_strength;

static double                           total_time = 0.0;

static xpl_sprite_batch_t				*bg_batch;
static xpl_sprite_t						*bg_sprite;
static xpl_sprite_t						*up_sprite;
#define BG_PARTICLE_COUNT				4096
static xvec2							bg_particle[BG_PARTICLE_COUNT];
static xvec2							bg_velocity[BG_PARTICLE_COUNT];
static float							bg_orient[BG_PARTICLE_COUNT];
static float							bg_rotate[BG_PARTICLE_COUNT];

#define HORIZ(w, dx, ay) 	(w).x += (dx); \
                            (w).y = (ay); \
                            xpl_imui_context_area_set(w);

static prefs_t prefs;

static void reset_particle(int i) {
	bg_particle[i] = xvec2_all(0.5f);
	bg_velocity[i] = xpl_rand_xvec2(-0.5, 0.5);
	bg_orient[i] = xpl_frand_range(0.f, M_2PI);
	bg_rotate[i] = xpl_frand_range(-M_PI_2, M_PI_2);
}

static void create_background() {
    // Quad for effects
    GLsizei element_size = sizeof(plane_xy_vertex_data[0]);
    
    effect_vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    xpl_bo_append(effect_vbo, &plane_xy_vertex_data[0], sizeof(plane_xy_vertex_data));
    xpl_bo_commit(effect_vbo);
    effect_elements = sizeof(plane_xy_vertex_data) / element_size;
    
    effect_vao = xpl_vao_new();
    xpl_vao_define_vertex_attrib(effect_vao, "position", effect_vbo,
                                 3, GL_FLOAT, GL_FALSE, element_size, offsetof(vertex_normal_t, vertex));
    
    // background_shader = xpl_shader_get_prepared("MenuBackground", "Menu.Vertex", "Menu.Background.Fragment");
    overlay_shader = xpl_shader_get_prepared("Overlay", "Overlay.Vertex", "Overlay.Fragment");
	
	bg_batch = xpl_sprite_batch_new();
	bg_sprite = xpl_sprite_new(bg_batch, "star.png", NULL);
	up_sprite = xpl_sprite_new(bg_batch, "tile_up.png", NULL);
	for (int i = 0; i < BG_PARTICLE_COUNT; ++i) {
		bg_particle[i] = xvec2_all(-10.f);
	}
    
    overlay_color = xvec4_set(0.f, 0.f, 0.f, 0.f);
    overlay_strength = 0.f;
}

static void destroy_background() {
	xpl_bo_destroy(&effect_vbo);
	xpl_vao_destroy(&effect_vao);
	// xpl_shader_release(&background_shader);

	audio_destroy(&title_bgm);
}

#ifndef XPL_PLATFORM_IOS
static void populate_video_modes() {
    GLFWvidmode current_mode;
    glfwGetDesktopMode(&current_mode);

    GLFWvidmode video_modes[256];
    size_t list_size = glfwGetVideoModes(video_modes, 256);
    supported_video_mode_count = 0;
    for (size_t i = list_size; i-- > 0;) {
        if (video_modes[i].Width % 8)                           continue;
        if (video_modes[i].Height % 8)                          continue;
        if (video_modes[i].RedBits < current_mode.RedBits)      continue;
        if (video_modes[i].GreenBits < current_mode.GreenBits)  continue;
        if (video_modes[i].BlueBits < current_mode.BlueBits)    continue;
        supported_video_modes[supported_video_mode_count++] = video_modes[i];
        
        if (supported_video_mode_count == MAX_VID_MODES) break;
    }
    
    for (size_t i = 0; i < supported_video_mode_count; ++i) {
        if (current_mode.Width != video_modes[i].Width)         continue;
        if (current_mode.Height != video_modes[i].Height)       continue;
        if (current_mode.RedBits != video_modes[i].RedBits)     continue;
        if (current_mode.GreenBits != video_modes[i].GreenBits) continue;
        if (current_mode.BlueBits != video_modes[i].BlueBits)   continue;
        
        selected_video_mode_index = i;
        break;
    }
}
#endif

static void menu_configure_graphics(xpl_app_t *app, xrect area) {
    area = xrect_contract_to(area, 600, 440);
    
#ifndef XPL_PLATFORM_IOS
    int window_clicked = FALSE;
    int save_clicked = FALSE;
#endif
	int reset_clicked;
    int back_clicked;
	int bgm_clicked;
    
    xpl_imui_context_begin(imui_context, app->execution_info, area);
    {
        xpl_imui_control_scroll_area_begin(xl("config_title"), area.size, &configure_scroll);
        {
            xpl_imui_indent_custom(16.0f);
            {
				bgm_clicked = xpl_imui_control_check(xl("config_bgm"), prefs.bgm_on, TRUE);
				reset_clicked = xpl_imui_control_button(xl("config_reset"), 0, TRUE);
#ifndef XPL_PLATFORM_IOS
            	if (xpl_imui_control_collapse(xl("config_preferences"), "", &preferences_is_expanded, TRUE)) {
					bgm_clicked = xpl_imui_control_check(xl("config_bgm"), prefs.bgm_on, TRUE);
					reset_clicked = xpl_imui_control_button(xl("config_reset"), 0, TRUE);
            	}
                if (xpl_imui_control_collapse(xl("config_graphics_resolution"), "",
                                              &resolution_is_expanded, supported_video_mode_count > 0)) {
                    window_clicked = xpl_imui_control_check(xl("config_graphics_window"), !app_config.is_fullscreen, TRUE);
                    for (size_t i = 0; i < supported_video_mode_count; ++i) {
                        char label[64];
                        snprintf(label, 64, "%d x %d, %d bits",
                                 supported_video_modes[i].Width,
                                 supported_video_modes[i].Height,
                                 supported_video_modes[i].RedBits + supported_video_modes[i].GreenBits + supported_video_modes[i].BlueBits);
                        if (xpl_imui_control_check(label, i == selected_video_mode_index, TRUE)) {
                            video_config_changed = (i != selected_video_mode_index);
                            app_config.width = supported_video_modes[i].Width;
                            app_config.height = supported_video_modes[i].Height;
                            app_config.color_bits = (supported_video_modes[i].RedBits +
                                                     supported_video_modes[i].GreenBits +
                                                     supported_video_modes[i].BlueBits);
                            app_config.alpha_bits = app_config.color_bits / 3;
                            selected_video_mode_index = i;
                        }
                    }
					save_clicked = xpl_imui_control_button(xl("config_save"), XPL_IMUI_BUTTON_DEFAULT, video_config_changed);
                }
#endif
                xpl_imui_separator_line();
                back_clicked = xpl_imui_control_button(xl("back"), XPL_IMUI_BUTTON_CANCEL, TRUE);
            }
            xpl_imui_outdent_custom(16.0f);
        }
        xpl_imui_control_scroll_area_end();
    }
    xpl_imui_context_end(imui_context);
    
	if (bgm_clicked) {
		prefs.bgm_on = !prefs.bgm_on;
		if (prefs.bgm_on) {
			title_bgm->volume = 1.f;
		} else {
			title_bgm->volume = 0.f;
		}
		prefs_set(prefs);
	}
	
	if (reset_clicked) {
		prefs_reset();
		prefs = prefs_get();
	}
	
#ifndef XPL_PLATFORM_IOS
    if (window_clicked) {
        app_config.is_fullscreen = !app_config.is_fullscreen;
        video_config_changed = true;
    }
    
    if (save_clicked) {
        xpl_app_params_save(app_config);
        
        if (video_config_changed) {
            context_next = NULL;
            app->restart = true;
        }
    }
#endif
    
    if (back_clicked) {
        menu_pop();
    }
    
}

/*
static void menu_configure(xpl_app_t *app, xrect area) {
    area = xrect_contract_to(area, 640, 160);
    
    int graphics_clicked;
    int controls_clicked;
    int back_clicked;
    
    float nullscroll = 0.f;
    xpl_imui_context_begin(imui_context, app->execution_info, area);
    {
        xpl_imui_control_scroll_area_begin(xl("config_title"), area.size, &nullscroll);
        {
            xpl_imui_indent_custom(16.0f);
            {
                graphics_clicked = xpl_imui_control_button(xl("config_button_graphics"), 0, TRUE);
                controls_clicked = xpl_imui_control_button(xl("config_button_controls"), 0, FALSE);
                xpl_imui_separator_line();
                back_clicked = xpl_imui_control_button(xl("back"), XPL_IMUI_BUTTON_CANCEL | XPL_IMUI_BUTTON_DEFAULT, TRUE);
            }
            xpl_imui_outdent_custom(16.0f);
        }
        xpl_imui_control_scroll_area_end();
    }
    xpl_imui_context_end(imui_context);
    
    if (graphics_clicked) {
        menu_push(ms_configure_graphics);
    }
    
    if (back_clicked) {
        menu_pop();
    }
}
 */

static void menu_main(xpl_app_t *app, xrect area) {
    area = xrect_contract_to(area, 600, 210);
    
    int new_clicked;
    int configure_clicked;
    int exit_clicked;
    
    float nullscroll = 0.f;
    xpl_imui_context_begin(imui_context, app->execution_info, area);
    {
        xpl_imui_control_scroll_area_begin(xl("main_title"), area.size, &nullscroll);
        {
            xpl_imui_control_label(xl("main_heading"));
            xpl_imui_indent_custom(16.0f);
            {
                xpl_imui_separator_line();
                new_clicked = xpl_imui_control_button(xl("main_new"), 0, TRUE);
                xpl_imui_separator();
                configure_clicked = xpl_imui_control_button(xl("main_configure"), 0, TRUE);
                xpl_imui_separator_line();
                exit_clicked = xpl_imui_control_button(xl("exit"), 0, TRUE);
            }
            xpl_imui_outdent_custom(16.0f);
        }
        xpl_imui_control_scroll_area_end();
    }
    xpl_imui_context_end(imui_context);
    
    if (new_clicked) {
        menu_push(ms_game_start);
    }
    
    if (configure_clicked) {
        menu_push(ms_configure_graphics);
    }
    
    if (exit_clicked) {
        menu_push(ms_exit);
    }
}

static void engine(xpl_context_t *self, double time, void *data) {
    if (menu_index == -1) {
        menu_push(ms_main);
    }

    switch (menu_stack[menu_index]) {
        case ms_game_start:
        {
            overlay_strength += time;
			if (title_bgm->volume > 0.f) title_bgm->volume = 1.0f - 0.333f * overlay_strength;
            overlay_color = xvec4_set(1.0, 0.67, 0.33, 1.0);
            if (overlay_strength >= 3.0) {
                context_next = xpl_context_new(self->app, &game_context_def);
            }
            break;
        }
            
        case ms_exit:
        {
            overlay_strength += 2.0 * time;
            if (title_bgm->volume > 0.f) title_bgm->volume = 1.0f - overlay_strength;
            overlay_color = xvec4_set(0.f, 0.f, 0.f, 1.f);
            if (overlay_strength >= 1.0) context_next = NULL;
            break;
        }
            
        default:
            break;
    }
	
	xrect bounds = xrect_set(0.f, 0.f, 1.0, 1.0);
	bounds = xrect_contract(bounds, -0.1f);
	int new_count = 0;
	const int new_limit = 5;
	for (int i = 0; i < BG_PARTICLE_COUNT; ++i) {
		bg_particle[i] = xvec2_add(bg_particle[i], xvec2_scale(bg_velocity[i], time));
		bg_orient[i] += bg_rotate[i] * time;
		
		if (new_count < new_limit && ! xrect_in_bounds(bounds, bg_particle[i])) {
			++new_count;
			reset_particle(i);
		}
	}
    
    total_time += time;
}

static void render(xpl_context_t *self, double time, void *vdata) {
    xrect area = {{ 0, 0, self->size.width, self->size.height }};
    
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	const float size = 0.02;
	
	xpl_sprite_batch_begin(bg_batch); {
		xmat4 *ortho = xpl_sprite_batch_matrix_push(bg_batch);
		xmat4_ortho(0.f, 1.f, 0.f, 1.f, -1.f, 1.f, ortho);
		
		xvec4 color = xvec4_all(1.0f);
		
		for (int i = 0; i < BG_PARTICLE_COUNT; ++i) {
			float f = xvec2_length(xvec2_sub(bg_particle[i], xvec2_all(0.5f)));
			bool special = i % 2048 == 0;
			xpl_sprite_t *sprite = special ? up_sprite : bg_sprite;
			if (special) f *= 4.f;
			color.a = f;
			xpl_sprite_draw_transformed(sprite,
										bg_particle[i].x, bg_particle[i].y,
										-size * 0.5f * f, -size * 0.5f * f,
										size * f, size * f,
										1.f, 1.f, bg_orient[i], &color);
		}
		
		xpl_sprite_batch_matrix_pop(bg_batch);
	}
	xpl_sprite_batch_end(bg_batch);
    
//    glUseProgram(background_shader->id);
//    glUniform1f(xpl_shader_get_uniform(background_shader, "time"), total_time);
//    xpl_vao_program_draw_arrays(effect_vao, background_shader, GL_TRIANGLES, 0, (GLsizei)effect_elements);
//    glUseProgram(GL_NONE);
    
    menu_func mf;
    
    switch (menu_stack[menu_index]) {
           
        case ms_configure_graphics:
            mf = menu_configure_graphics;
            break;
            
        case ms_main:
        default:
            mf = menu_main;
            break;
    }
    mf(self->app, area);
    
    glUseProgram(overlay_shader->id);
    glUniform1f(xpl_shader_get_uniform(overlay_shader, "scanline_amount"), 0.7f);
    glUniform1f(xpl_shader_get_uniform(overlay_shader, "strength"), overlay_strength);
    glUniform4fv(xpl_shader_get_uniform(overlay_shader, "color"), 1, overlay_color.data);
    xpl_vao_program_draw_arrays(effect_vao, overlay_shader, GL_TRIANGLES, 0, (GLsizei)effect_elements);
    glUseProgram(GL_NONE);
    
}

static void *init(xpl_context_t *self) {
	
    imui_theme = xpl_imui_theme_load_new("ld26");
    imui_context = xpl_imui_context_new(imui_theme);
    context_next = self;
    
    app_config = xpl_app_params_load(FALSE);
    
#ifndef XPL_PLATFORM_IOS
    populate_video_modes();
    resolution_is_expanded = FALSE;
    preferences_is_expanded = FALSE;
#endif
    create_background();

	prefs = prefs_get();
    
#ifndef XPL_PLATFORM_IOS
    glfwEnable(GLFW_MOUSE_CURSOR);
#endif
    
    return NULL;
}

static void destroy(xpl_context_t *self, void *vdata) {
    xpl_imui_context_destroy(&imui_context);
    xpl_imui_theme_destroy(&imui_theme);
    
    destroy_background();
    
    while (menu_pop());

	xpl_context_destroy(&self);
}

static xpl_context_t *handoff(xpl_context_t *self, void *vdata) {
	return context_next;
}

void menu_push(menu_select_t menu) {
    menu_stack[++menu_index] = menu;
}

bool menu_pop() {
    if (menu_index <= 0) return false;
    --menu_index;
    return true;
}

xpl_context_def_t menu_context_def = {
    init,
    engine,
    render,
    destroy,
    handoff
};
