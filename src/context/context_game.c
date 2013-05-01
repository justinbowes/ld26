//
//  context_game.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-25.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "xpl.h"

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <errno.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#endif

#include "game/camera.h"
#include "game/game.h"
#include "game/packet.h"
#include "game/palette.h"
#include "game/prefs.h"
#include "game/projectile_config.h"
#include "game/sprites.h"
#include "game/util.h"

#include "context/context_game.h"

#include "xpl_imui.h"
#include "xpl_rand.h"
#include "xpl_text_cache.h"

#include "audio/audio.h"
#include "net/udpnet.h"

#include "models/plane-xy.h"

#include "xpl_text_buffer.h"

#define LOG_LINES 3
#define LOG_LINE_MAX 512
#define LOG_TIMEOUT 10.f
typedef struct log {
	xpl_text_buffer_t	*buffer;
	xpl_markup_t		markup;
	char				lines[LOG_LINES][LOG_LINE_MAX];
	bool				rebuild;
	float				timeout;
} log_t;

#define JIFFY			1.0f / 60.0f

#define RESPAWN_COOLDOWN 3.0f

#define HELLO_TIMEOUT	2.0f
#define ERRORMSG_TIMEOUT	10.0f
#define POSITION_TIMEOUT 5.0f
#define POSITION_TIMEOUT_UNDER_THRUST 0.1f
#define RECEIVE_TIMEOUT	5.0f

#define THRUST			2.0f
#define TORQUE			192.0f
#define INITIAL_HEALTH	255

#define SPAWN_BOX		1024

#define DEFAULT_SCANLINE	0.7f

#define UI_FONT			"Chicago"

#define DAMAGE_VOLUME	0.5f
#define CHAT_VOLUME		0.5f
#define ROTATE_VOLUME	0.2f
#define THRUST_VOLUME	0.4f
#define SELECT_VOLUME	0.2f
#define EXPLODE_VOLUME	1.0f
#define FIRE_VOLUME		0.5f

#define MAX_VELOCITY	6.0f

enum packet_errors {
	pe_client_id
};

static const int up[]		= { GLFW_KEY_UP,		'W',	0};
static const int down[]		= { GLFW_KEY_DOWN,		'S',	0};
static const int left[]		= { GLFW_KEY_LEFT,		'A',	0};
static const int right[]	= { GLFW_KEY_RIGHT,		'D',	0};
static const int fire[]		= { GLFW_KEY_LCTRL,		' ',	GLFW_KEY_ENTER, 0};

game_t									game;
network_t								network;

// tutorial
static int								ui_tutorial_page;
static float							scanline_strength;

// chat
static bool								chat_showing;
static bool								chat_close_wait;
static bool								chat_reset_focus;
static char								chat_buffer[CHAT_MAX];

static xpl_imui_theme_t					*theme;
static xpl_imui_context_t				*imui;


// Overlay
static xpl_shader_t                     *overlay_shader;
static xpl_vao_t                        *effect_vao;
static xpl_bo_t                         *effect_vbo;
static size_t                           effect_elements;
static xvec4                            overlay_color;
static float                            overlay_strength;

// Audio
static audio_t							*bgm_stream = NULL;
static audio_t							*damage_audio = NULL;

static uint32_t							packet_seq = 0;
static int								sock;
static UDPNET_ADDRESS					*server_addr = NULL;

// Text
static log_t							ui_log;
static xpl_text_cache_t					*name_cache;
static xpl_markup_t						name_markup;
static xpl_text_cache_t					*text_particle_cache;
static xpl_markup_t						text_particle_markup;
static xpl_text_cache_t					*ui_cache;
static xpl_markup_t						ui_markup;
static xpl_text_buffer_t				*tutorial_buffer;
static xpl_markup_t						tutorial_markup;

// forward decls
#pragma mark -
#pragma mark Declarations

static void game_destroy(xpl_context_t *self, void *data);
static void game_engine(xpl_context_t *self, double time, void *data);
static void *game_init(xpl_context_t *self);
static void game_init_overlay(void);
static void game_init_text(void);
static void game_render(xpl_context_t *self, double time, void *data);
static void game_render_log(xpl_context_t *self);
static void game_render_playfield(xpl_context_t *self, double time);
static void game_render_ui(xpl_context_t *self);
static void game_reset(void);
static xpl_context_t *game_handoff(xpl_context_t *self, void *data);

static bool key_down(const int *key_array);

static void log_add_text(const char *text, ...);
static void log_advance_line(void);

static void packet_handle(uint16_t client_id, packet_t *packet);
static void packet_handle_chat(uint16_t client_id, packet_t *packet);
static void packet_handle_damage(uint16_t client_id, packet_t *packet);
static void packet_handle_hello(uint16_t client_id, packet_t *packet);
static void packet_handle_goodbye(uint16_t client_id, packet_t *packet);
static void packet_handle_player(uint16_t client_id, packet_t *packet);
static void packet_handle_projectile(uint16_t client_id, packet_t *packet);
static void packet_receive(void);
static void packet_send(packet_t *packet);
static void packet_send_chat(void);
static void packet_send_hello(void);
static void packet_send_player(void);
static void packet_send_projectile(int i);

static void particle_add(position_t position, xvec2 velocity, xvec4 color, int size, float life, bool color_decay);
static int particle_find_new(void);
static void particle_update(int i, double time);

static void player_add_damage_particle(int i);
static void player_add_thrust_particle(int i);
static void player_add_explode_effect(int i);
static xvec2 player_calculate_oriented_thrust(int i);
static xvec2 player_get_direction_vector(int i);
static void player_init(int i);
static void player_local_disconnect(void);
static bool player_local_is_connected(void);
static void player_local_update_chat(void);
static void player_local_update_firing(bool jiffy_elapsed);
static void player_local_update_rotation(double time);
static void player_local_update_thrust(double time);
static void player_local_update_weapon(void);
static const char *player_name(int i, bool as_object);
static void player_update_position(int i);
static int player_with_client_id_get(uint16_t client_id, bool allow_allocate, bool *was_new_player);
static xvec2 player_v2velocity_get(int i);

static void position_mod(position_t *position);

static void projectile_add(void);
static void projectile_explode_effect(int pi, int target);
static bool projectile_type_is_mine(int type);
static void projectile_update(int i, bool jiffy_elapsed);
static int projectile_with_pid_get(uint16_t pid, int ti, bool *was_new);

static void server_resolve_addr(void);

static float text_get_length(xpl_font_t *font, const char *text, size_t position);
static void text_particle_add(position_t position, xvec2 velocity, const char *text, xvec4 color, float life);
static int text_particle_find_new(void);
static void text_particle_update(int i, double time);

static void ui_error_set(const char *msg, ...);
static void ui_error_packet_set(int eno);
static void ui_error_show(xpl_context_t *self);
static void ui_pilot_config_show(xpl_context_t *self);
static void ui_tutorial_show(xpl_context_t *self, double time);
static void ui_window_end(void);
static void ui_window_start(xpl_context_t *self, xvec2 size, const char *title, float *scroll);

static xvec2 v2_for_velocity(velocity_t velocity);
static velocity_t velocity_for_v2(xvec2 v);
static xvec3 v3_relative_audio(position_t position);

#pragma mark -
#pragma mark Implementations


// ------------------------------------------------------------------------------


static xvec4 color_variant(uint32_t color, float variance) {
	xvec4 max = RGBA_F(color);
	xvec4 min = xvec4_scale(max, 1.0 - variance);
	return xvec4_set(xpl_frand_range(min.r, max.r),
					 xpl_frand_range(min.g, max.g),
					 xpl_frand_range(min.b, max.b),
					 xpl_frand_range(min.a, max.a));
}

// ------------------------------------------------------------------------------

static void game_destroy(xpl_context_t *self, void *data) {
	xpl_imui_context_destroy(&imui);
	xpl_imui_theme_destroy(&theme);
	
	udp_socket_exit();
}

#define INDICATOR_COOLDOWN_JIFFIES 15
static void game_engine(xpl_context_t *self, double time, void *data) {
	static double jiffy = 0.0;
	bool jiffy_elapsed = false;
	jiffy += time;
	if (jiffy > JIFFY) {
		jiffy = JIFFY;
		jiffy_elapsed = true;
	}
	
	if (game.player_connected[0]) {
		if (! server_addr) {
			server_resolve_addr();
		}
		
		network.hello_timeout -= time;
		if (network.hello_timeout <= 0.f) {
			network.hello_timeout = HELLO_TIMEOUT;
			packet_send_hello();
		}

		packet_receive();

		network.receive_timeout -= time;
		if (network.receive_timeout <= 0.f) {
			if (player_local_is_connected()) {
				ui_error_set("Disconnected from server.");
			} else {
				ui_error_set("Connection to server failed.");
			}
			player_local_disconnect();
		}
	}
	
	ui_log.timeout -= time;
	if (ui_log.timeout <= 0.f) {
		log_advance_line();
		ui_log.timeout = LOG_TIMEOUT;
	}
	if (ui_log.rebuild) {
		xpl_text_buffer_clear(ui_log.buffer);
		xvec2 pen = {{ 0.f, self->size.height - 4.f }};
		for (int i = 0; i < LOG_LINES; ++i) {
			pen.x = 4.f;
			wchar_t line[LOG_LINE_MAX];
			mbstowcs(line, ui_log.lines[i], LOG_LINE_MAX);
			if (wcslen(line)) xpl_text_buffer_add_text(ui_log.buffer, &pen, &ui_log.markup, line, 0);
			pen.y = floorf(pen.y);
		}
		xpl_text_buffer_commit(ui_log.buffer);
		ui_log.rebuild = false;
	}
	
	if (player_local_is_connected()) {
		static bool last_was_thrusting;
		network.position_timeout -= time;
		if (game.respawn_cooldown <= 0.f) {
			if ((network.position_timeout <= 0.f) ||
				(last_was_thrusting != game.player[0].is_thrust)) {
				bool thrust = game.player[0].is_thrust;
				last_was_thrusting = thrust;
				network.position_timeout = thrust ? POSITION_TIMEOUT_UNDER_THRUST : POSITION_TIMEOUT;
				packet_send_player();
			}
		}
		
		if (jiffy_elapsed) {
			if (game.indicators_cooldown == 0) {
				game.indicators_cooldown = INDICATOR_COOLDOWN_JIFFIES;
				game.indicators_on = !game.indicators_on;
			}
			--game.indicators_cooldown;			
		}
		
		if (game.respawn_cooldown > 0.f) {
			damage_audio->volume = 0.f;
			game.respawn_cooldown -= time;
			if (game.respawn_cooldown <= 0.f) {
				player_init(0);
			}
		} else {
			game.player_local[0].visible = true;
			damage_audio->volume = DAMAGE_VOLUME * ((255.f - game.player[0].health) / 255.f);
			player_local_update_chat();
			player_local_update_thrust(time);
			player_local_update_rotation(time);
			player_local_update_firing(jiffy_elapsed);
			player_local_update_weapon();
		}
		
		for (int i = 0; i < MAX_PROJECTILES; ++i) {
			if (game.projectile[i].health) {
				projectile_update(i, jiffy_elapsed);
			}
		}
		
		for (int i = 0; i < MAX_PARTICLES; ++i) {
			if (game.particle[i].life > 0.f) {
				particle_update(i, time);
			}
		}
		
		for (int i = 0; i < MAX_TEXT_PARTICLES; ++i) {
			if (game.text_particle[i].life > 0.f) {
				text_particle_update(i, time);
			}
		}
		
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (! game.player_connected[i]) continue;
			
			player_update_position(i);
			//			LOG_DEBUG("%d: %u,%u", i, game.player[i].position.px, game.player[i].position.py);
		}
		
		int max_nudge_x = camera.draw_area.width / 4;
		int max_nudge_y = camera.draw_area.height / 4;
		int nudge_x = xclamp(game.player[0].velocity.dx / VELOCITY_SCALE, -max_nudge_x, max_nudge_x);
		int nudge_y = xclamp(game.player[0].velocity.dy / VELOCITY_SCALE, -max_nudge_y, max_nudge_y);
		camera_calculate_center(&game.player[0].position, nudge_x, nudge_y);
		
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (game.player[i].health < 255 && game.player[i].health > 0) {
				float frac = game.player[i].health / 255.f;
				if (xpl_frand() > frac) {
					const float frequency = 50.f / 60.f;
					if (xpl_frand() > frequency) player_add_damage_particle(i);
				}
			}
			if (game.player[i].is_thrust && position_in_bounds(game.player[i].position, 500, camera.min, camera.max)) {
				player_add_thrust_particle(i);
			}
		}
		
	}
	
	if (network.error_timeout > 0.f) {
		network.error_timeout -= time;
	}
	
}


static xpl_context_t *game_handoff(xpl_context_t *self, void *data) {
	return self;
}


static void *game_init(xpl_context_t *self) {
	srand((unsigned int)time(0));
	random_word("destroyed");
	random_word("joined");
	random_word("damaged");
	random_word("battle");
	
	theme = xpl_imui_theme_load_new("ld26");
	imui = xpl_imui_context_new(theme);
	
	udp_socket_init();
	sock = udp_create_endpoint(0);
	game_reset();
	
	prefs_t prefs = prefs_get();
	
	strncpy(network.server_host, prefs.server, SERVER_SIZE);
	strncpy(game.player_id[0].name, prefs.name, NAME_SIZE);
	network.server_port = prefs.port;
	
	game_init_overlay();
	game_init_text();

	sprites_init();
	
	camera.draw_area = xirect_set(0, 0, self->size.width, self->size.height);
	// room for bottom bar
	camera.draw_area.y += (TILE_SIZE + 40); camera.draw_area.height -= (TILE_SIZE + 40);
	// room for log
	camera.draw_area.height -= 48;
	
	scanline_strength = 0.7;
	
	bgm_stream = audio_create("bgm.ogg");
	bgm_stream->loop = true;
	if (prefs.bgm_on) bgm_stream->action = aa_play;
	
	damage_audio = audio_create("alert.ogg");
	damage_audio->loop = true;
	damage_audio->volume = 0.f;
	damage_audio->action = aa_play;
	
	
	ui_tutorial_page = 0;
	
	return NULL;
}

static void game_init_overlay(void) {
	
	// Quad for effects
	GLsizei element_size = sizeof(plane_xy_vertex_data[0]);
	
	effect_vbo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	xpl_bo_append(effect_vbo, &plane_xy_vertex_data[0], sizeof(plane_xy_vertex_data));
	xpl_bo_commit(effect_vbo);
	effect_elements = sizeof(plane_xy_vertex_data) / element_size;
	
	effect_vao = xpl_vao_new();
	xpl_vao_define_vertex_attrib(effect_vao, "position", effect_vbo,
								 3, GL_FLOAT, GL_FALSE, element_size, offsetof(vertex_normal_t, vertex));
	
	
	overlay_shader = xpl_shader_get_prepared("Overlay", "Overlay.Vertex", "Overlay.Fragment");
	
	overlay_color = xvec4_set(1.f, 1.f, 1.f, 1.f);
	overlay_strength = 0.f;
}

static void game_init_text(void) {
	ui_log.buffer = xpl_text_buffer_new(1024, 1024, 1);
	xpl_markup_clear(&ui_log.markup);
	xpl_markup_set(&ui_log.markup, UI_FONT, 16.f, FALSE, FALSE, xvec4_set(0.0f, 0.0f, 0.3f, 1.f), xvec4_set(0.f, 0.f, 0.f, 0.0f));
	memset(ui_log.lines, 0, LOG_LINES * LOG_LINE_MAX);
	ui_log.timeout = LOG_TIMEOUT;
	
	name_cache = xpl_text_cache_new();
	xpl_markup_clear(&name_markup);
	xpl_markup_set(&name_markup, UI_FONT, 16.f, FALSE, FALSE, xvec4_set(1.f, 1.f, 1.f, 0.5f), xvec4_all(0.f));
	
	text_particle_cache = xpl_text_cache_new();
	xpl_markup_clear(&text_particle_markup);
	xpl_markup_set(&text_particle_markup, UI_FONT, 16.f, FALSE, FALSE, xvec4_set(1.f, 1.f, 1.f, 1.f), xvec4_all(0.f));

	ui_cache = xpl_text_cache_new();
	xpl_markup_clear(&ui_markup);
	xpl_markup_set(&ui_markup, UI_FONT, 16.f, FALSE, FALSE, xvec4_set(1.f, 1.f, 1.f, 1.f), xvec4_all(0.f));
	
	tutorial_buffer = xpl_text_buffer_new(1024, 1024, 1);
	xpl_markup_clear(&tutorial_markup);
	xpl_markup_set(&tutorial_markup, UI_FONT, 14.f, FALSE, FALSE, xvec4_set(1.f, 1.f, 1.f, 1.f), xvec4_all(0.f));
}

static void game_render(xpl_context_t *self, double time, void *data) {
	assert(self);
	
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	
	if (player_local_is_connected()) {
		game_render_playfield(self, time);
	}
	
	// Effect overlay
	glUseProgram(overlay_shader->id);
	glUniform1f(xpl_shader_get_uniform(overlay_shader, "scanline_amount"), scanline_strength);
	glUniform1f(xpl_shader_get_uniform(overlay_shader, "strength"), overlay_strength);
	glUniform4fv(xpl_shader_get_uniform(overlay_shader, "color"), 1, overlay_color.data);
	xpl_vao_program_draw_arrays(effect_vao, overlay_shader, GL_TRIANGLES, 0, (GLsizei)effect_elements);
	glUseProgram(GL_NONE);

	if (network.error_timeout) {
		ui_error_show(self);
	} else if (ui_tutorial_page) {
		ui_tutorial_show(self, time);
	} else if (! game.player_connected[0] && ! network.error_timeout) {
		ui_pilot_config_show(self);
	} else if (player_local_is_connected()) {
		game_render_ui(self);
	}
	
	game_render_log(self);

	xpl_text_cache_advance_frame(name_cache);
}

static void game_render_log(xpl_context_t *self) {
	xmat4 ortho;
	xmat4_ortho(0.f, self->size.width, 0.f, self->size.height, -1.f, 1.f, &ortho);
	xpl_text_buffer_render(ui_log.buffer, ortho.data);
}

static void game_render_playfield(xpl_context_t *self, double time) {
	
	xmat4 ortho;
	xmat4_ortho(0.f, self->size.width, 0.f, self->size.height, -1.f, 1.f, &ortho);
	
	glEnable(GL_SCISSOR_TEST);
	glScissor(camera.draw_area.x, camera.draw_area.y, camera.draw_area.width, camera.draw_area.height);

	// Render player names, excluding self
	for (int i = 1; i < MAX_PLAYERS; ++i) {
		if (! game.player_connected[i]) continue;
		if (! game.player_local[i].visible) continue;
		if (! player_in_bounds(i, PLAYER_SIZE, camera.min, camera.max)) continue;
		
		const char *name = player_name(i, false);
		xpl_cached_text_t *text = xpl_text_cache_get(name_cache, &name_markup, name);
		float text_length = text_get_length(text->managed_font, name, -1);
		xvec2 v = camera_get_draw_position(game.player[i].position);
		xvec3 pen = {{ v.x - text_length / 2, v.y - name_markup.size, 0.f }};
		pen.x = xclamp(pen.x, camera.draw_area.x, camera.draw_area.x + camera.draw_area.width - text_length);
		pen.y = xclamp(pen.y, camera.draw_area.y + name_markup.size, camera.draw_area.y + camera.draw_area.height);
		xmat4 ortho_translate;
		xmat4_translate(&ortho, &pen, &ortho_translate);
		
		xpl_text_buffer_render(text->buffer, ortho_translate.data);
	}
	
	sprites_playfield_render(self, &ortho);
	
	// Render text particles
	for (int i = 0; i < MAX_TEXT_PARTICLES; ++i) {
		if (game.text_particle[i].life <= 0.f) continue;
		if (! position_in_bounds(game.text_particle[i].position, 64, camera.min, camera.max)) continue;
		
		text_particle_markup.foreground_color = game.text_particle[i].color;
		xpl_cached_text_t *text = xpl_text_cache_get(text_particle_cache, &text_particle_markup, game.text_particle[i].text);
		float text_length = text_get_length(text->managed_font, game.text_particle[i].text, -1);
		xvec2 v = camera_get_draw_position(game.text_particle[i].position);
		xvec3 pen = {{ v.x - text_length / 2, v.y - text_particle_markup.size / 2, 0.f }};
		xmat4 ortho_transform;
		xmat4_translate(&ortho, &pen, &ortho_transform);
		xmat4_rotate(&ortho_transform, game.text_particle[i].orientation, &xvec3_z_axis, &ortho_transform);
		
		xpl_text_buffer_render(text->buffer, ortho_transform.data);
	}
	
	glDisable(GL_SCISSOR_TEST);
	
}

static void game_render_ui(xpl_context_t *self) {
	xmat4 ortho;
	xmat4_ortho(0.f, self->size.width, 0.f, self->size.height, -1.f, 1.f, &ortho);

	sprites_ui_render(self, &ortho);
	
	int cash = game.player[0].score;
	// Render prices and keys
	for (int i = 0; i < 8; ++i) {
		int price = projectile_config[i].price;
		char price_str[8];
		char key_str[2];

		snprintf(key_str, 2, "%d", i + 1);
		ui_markup.foreground_color = (price <= cash) ? active_color : expensive_color;
		ui_markup.size = 16.f;   
		xpl_cached_text_t *text = xpl_text_cache_get(ui_cache, &ui_markup, key_str);
		xvec3 pen = {{ 194 + (TILE_SIZE + 8) * i, 38.f + ui_markup.size, 0.f }};
		xmat4 ortho_translate;
		xmat4_translate(&ortho, &pen, &ortho_translate);
		xpl_text_buffer_render(text->buffer, ortho_translate.data);
		
		if (price >= 0) {
			snprintf(price_str, 8, "%d", price);
			ui_markup.foreground_color = (price <= cash) ? available_color : expensive_color;
			ui_markup.size = 16.f;
			xpl_cached_text_t *text = xpl_text_cache_get(ui_cache, &ui_markup, price_str);
			float text_length = text_get_length(text->managed_font, price_str, -1);
			xvec3 pen = {{ 184 + (TILE_SIZE + 8) * (i + 1) - text_length, 4.f + ui_markup.size, 0.f }};
			xmat4 ortho_translate;
			xmat4_translate(&ortho, &pen, &ortho_translate);		
			xpl_text_buffer_render(text->buffer, ortho_translate.data);
		}
	}
	
	
	// Render cash
	char score[20];
	snprintf(score, 20, "%u", game.player[0].score);
	ui_markup.foreground_color = game.player[0].score ? normal_color : broke_color;
	ui_markup.size = 24.f;
	xpl_cached_text_t *text = xpl_text_cache_get(ui_cache, &ui_markup, score);
	float text_length = text_get_length(text->managed_font, score, -1);
	xvec3 pen = {{ self->size.width - text_length - 16, 32.f + ui_markup.size, 0.f }};
	xmat4 ortho_translate;
	xmat4_translate(&ortho, &pen, &ortho_translate);
	xpl_text_buffer_render(text->buffer, ortho_translate.data);
	
	// Render chat if it's up
	if (chat_showing) {
		static int chat_cursor = 0;
		xpl_imui_context_begin(imui, self->app->execution_info, xrect_set(0, self->size.height - 80, self->size.width, 30));
		{
			if (chat_reset_focus) {
				xpl_imui_context_reset_focus();
				chat_reset_focus = false;
				if (! glfwGetKey('T')) 	xpl_imui_control_textfield(xl("chat_message"), chat_buffer, CHAT_MAX, &chat_cursor, "", TRUE);
			} else {
				xpl_imui_control_textfield(xl("chat_message"), chat_buffer, CHAT_MAX, &chat_cursor, "", TRUE);
			}
		}
		xpl_imui_context_end(imui);
	}
}

static void game_reset(void) {
	char name[NAME_SIZE];
	strncpy(name, game.player_id[0].name, NAME_SIZE);
	
	memset(&game, 0, sizeof(game));
	
	strncpy(game.player_id[0].name, name, NAME_SIZE);
	
	chat_showing = false;
}

// ------------------------------------------------------------------------------


static bool key_down(const int *key_array) {
	const int *ptr = key_array;
	while (*ptr) {
		if (glfwGetKey(*ptr)) return true;
		ptr++;
	}
	
	return false;
}


// ------------------------------------------------------------------------------

static void log_add_text(const char *text, ...) {
	char *target = NULL;
	if (ui_log.lines[2] == 0) target = ui_log.lines[2];
	if (ui_log.lines[1] == 0) target = ui_log.lines[1];
	if (ui_log.lines[0] == 0) target = ui_log.lines[0];
	
	if (target == NULL) {
		target = ui_log.lines[2];
		log_advance_line();
	}
	
	va_list args;
	va_start(args, text);
	vsnprintf(target, LOG_LINE_MAX, text, args);
	va_end(args);
	
	ui_log.rebuild = true;
	ui_log.timeout = LOG_TIMEOUT;
}

static void log_advance_line(void) {
	// Move lines 1-2 to 0-1
	memmove(ui_log.lines[0], ui_log.lines[1], 2 * LOG_LINE_MAX);
	ui_log.lines[2][0] = '\0';
	ui_log.rebuild = true;
}

// ------------------------------------------------------------------------------


static void packet_handle(uint16_t client_id, packet_t *packet) {
	switch (packet->type) {
		case pt_hello:
			packet_handle_hello(client_id, packet);
			break;
			
		case pt_goodbye:
			packet_handle_goodbye(client_id, packet);
			break;
			
		case pt_player:
			packet_handle_player(client_id, packet);
			break;
			
		case pt_projectile:
			packet_handle_projectile(client_id, packet);
			break;
			
		case pt_damage:
			packet_handle_damage(client_id, packet);
			break;
			
		case pt_chat:
			packet_handle_chat(client_id, packet);
			break;
			
		default:
			break;
	}
}

static void packet_handle_chat(uint16_t client_id, packet_t *packet) {
	packet->chat[63] = '\0';
	LOG_DEBUG("Got chat packet: %u %s", client_id, packet->chat);
	log_add_text(xl("chat_format"),
				 player_name(player_with_client_id_get(client_id, false, NULL), false),
				 packet->chat);
	audio_quickplay_pan("chat.ogg", CHAT_VOLUME, 0.5f);
}

static void packet_handle_damage(uint16_t client_id, packet_t *packet) {
	char damage[8];
	snprintf(damage, 8, "%d", (int)packet->damage.amount);
	
	int origin = player_with_client_id_get(packet->damage.player_id, false, NULL);
	int target = player_with_client_id_get(client_id, false, NULL);
	int projectile = projectile_with_pid_get(packet->damage.projectile_id, -1, NULL);

	xvec2 velocity = xpl_rand_xvec2(-64.f, 64.f);
	xvec4 color = RGBA_F(0x80ffc0a0);
	text_particle_add(game.player[target].position, velocity, damage, color, 2.0);
	
	
	if (packet->damage.flags & DAMAGE_FLAG_REPAIRS) {
		log_add_text("%s repaired for %d",
					 player_name(origin, false),
					 packet->damage.amount);
		
	} else if (projectile >= 0 && game.projectile[projectile].health > 0) {
		char weapon_name_key[20];
		snprintf(weapon_name_key, 20, "weapon_name_%d", game.projectile[projectile].type);
		
		log_add_text(xl("weapon_damage_format"),
					 player_name(origin, false),
					 random_word("damaged"),
					 packet->damage.amount,
					 player_name(target, origin == target),
					 xl(weapon_name_key));

		
	} else {
		log_add_text(xl("generic_damage_format"),
					 player_name(target, false),
					 random_word("damage"),
					 packet->damage.amount);
	}
	
	if (packet->damage.flags & DAMAGE_FLAG_EXPLODES) {
		player_add_explode_effect(target);
		game.player_local[target].visible = false;
		log_add_text("%s %s %s", player_name(origin, false), random_word("destroyed"), player_name(target, origin == target));
	}
	
	// Destroys projectile, including mines.
	if (projectile) {
		projectile_explode_effect(projectile, target);
		game.projectile[projectile].health = 0;
	}
	
	if (origin == 0 && target != 0) {
		game.player[0].score += packet->damage.amount;
		packet_send_player();
	}
}

static void packet_handle_hello(uint16_t client_id, packet_t *packet) {
	if (client_id == 0) {
		ui_error_packet_set(pe_client_id);
		return;
	}
	
	bool was_new;
	if (packet->hello.nonce) {
		// This is a welcome packet
		if (game.player_id[0].nonce == packet->hello.nonce) {
			LOG_DEBUG("Matching nonce, logged in");
			game.player_id[0] = packet->hello;
			player_init(0);
			log_add_text("You have %s the %s", random_word("joined"), random_word("battle"));
		}
	} else {
		// This is a notify packet
		int pi = player_with_client_id_get(packet->hello.client_id, true, &was_new);
		bool had_name = !! strlen(game.player_id[pi].name);
		game.player_id[pi] = packet->hello;
		LOG_DEBUG("Player alive: %s %u", player_name(pi, false), game.player_id[pi].client_id);
		if (was_new || !had_name) {
			log_add_text("%s has joined", player_name(pi, false));
		}
	}
}

static void packet_handle_goodbye(uint16_t client_id, packet_t *packet) {
	int pi = player_with_client_id_get(client_id, false, NULL);
	if (pi == -1) {
		LOG_DEBUG("We didn't know about player %u who left", client_id);
		return;
	}
	LOG_DEBUG("Player leaving: %s %u", player_name(pi, false), game.player_id[pi].client_id);
	log_add_text("%s quit", player_name(pi, false));
	game.player_connected[pi] = false;
	memset(&game.player_id[pi], 0, sizeof(player_id_t));
	player_add_explode_effect(pi);
	
}

static void packet_handle_player(uint16_t client_id, packet_t *packet) {
	LOG_DEBUG("Updating player %d", client_id);
	int pi = player_with_client_id_get(client_id, true, NULL);
	if (pi == 0) {
		LOG_DEBUG("Got self packet");
	} else {
		if (game.player[pi].orientation != packet->player.orientation) {
			game.player_local[pi].rotate_audio->action = aa_play;
		}
		game.player_local[pi].thrust_audio->action = packet->player.is_thrust ? aa_play : aa_stop;
		game.player[pi] = packet->player;
		game.player_local[pi].visible = true;
	}
}

static void packet_handle_projectile(uint16_t client_id, packet_t *packet) {
	LOG_DEBUG("Updating projectile fired by %d", client_id);
	if (client_id == game.player_id[0].client_id) {
		// Avoid hitting self due to lag
		return;
	}
	bool is_new;
	int pi = projectile_with_pid_get(packet->projectile.pid, packet->projectile.type, &is_new);
	game.projectile_local[pi].owner = client_id;
	game.projectile[pi] = packet->projectile;
	if (is_new) {
		int type = game.projectile[pi].type;
		audio_quickplay_position(projectile_config[type].fire_effect, FIRE_VOLUME, v3_relative_audio(game.projectile[pi].position));
	}
}


static void packet_receive(void) {
	
	uint8_t buffer[1024];
	UDPNET_ADDRESS receive_addr;
	

	while(1) {
		memset(buffer, 0, 1024);
		int n = udp_receive(sock, buffer, 1024, &receive_addr);
		if (n < 0) {
			int udperr = udp_error();
			if (udperr == UN_WOULDBLOCK)	return;
			if (udperr == UN_AGAIN)			return;
			
			ui_error_set("Error receiving: %d", udperr);
			player_local_disconnect();
		}
		if (n <= 0) return;
		
		network.receive_timeout = RECEIVE_TIMEOUT;
		
		if (strcmp(receive_addr.address, server_addr->address) != 0) {
			LOG_WARN("Discarding packet from unknown host %s", receive_addr.address);
		}
		uint16_t packet_source;
		packet_t packet;
		if (! packet_decode(&packet, &packet_source, buffer)) {
			ui_error_set("Invalid response from server.");
			player_local_disconnect();
		}
		packet_handle(packet_source, &packet);
	}
}


static void packet_send(packet_t *packet) {
	if (! server_addr) {
		server_resolve_addr();
	}
	
	uint8_t buffer[1024];
	packet->seq = ++packet_seq;
	size_t len = packet_encode(packet, game.player_id[0].client_id, buffer);
	LOG_TRACE("sending packet");
	int result = udp_send(sock, buffer, (int)len, server_addr->address, server_addr->port);
	if (result == -1) {
		ui_error_set("Invalid remote address");
		return;
	}
	if (result == -2) {
		int udperr = udp_error();
		ui_error_set("Error sending packet (%d)", udperr);
		switch (udperr) {
			case UN_AGAIN:
				// Rate too high?
				LOG_DEBUG("EAGAIN");
				return;
				
			case UN_NOBUFS:
				// Rate too high?
				LOG_DEBUG("ENOBUFS");
				return;
				
			case EBADF:
				ui_error_set("Socket lost");
				break;
				
			case UN_CONNRESET:
				ui_error_set("Connection reset by peer");
				break;
				
			case UN_ACCES:
			case UN_HOSTUNREACH:
			case UN_MSGSIZE:
			case UN_NETDOWN:
			case UN_NETUNREACH:
			case UN_DESTADDRREQ:
			case UN_NOTCONN:
				ui_error_set("Network issues. Check your network connection and the server hostname.");
				break;
				
			case UN_INTR:
				ui_error_set("Send interrupted");
				break;
				
			case UN_FAULT:
				ui_error_set("Programmer error");
				break;
			case UN_NOTSOCK:
				ui_error_set("Programmer error");
				break;
			case UN_OPNOTSUPP:
				ui_error_set("Programmer error");
				break;
				
			default:
				break;
		}
		
		player_local_disconnect();
	}
}

static void packet_send_chat(void) {
	packet_t packet;
	memset(&packet, 0, sizeof(packet));
	packet.type = pt_chat;
	strncpy(packet.chat, chat_buffer, CHAT_MAX);
	packet_send(&packet);
}

static void packet_send_damage(uint16_t origin, uint16_t projectile_id, uint8_t damage, uint8_t flags) {
	assert(origin);
	
	packet_t packet;
	packet.type = pt_damage;
	packet.damage.player_id = origin;
	packet.damage.projectile_id = projectile_id;
	packet.damage.amount = damage;
	packet.damage.flags = flags;
	packet_send(&packet);
}

static void packet_send_hello(void) {
	packet_t packet;
	memset(&packet, 0, sizeof(packet));
	packet.type = pt_hello;
	packet.hello = game.player_id[0];
	packet_send(&packet);
}

static void packet_send_player(void) {
	assert(game.player_id[0].client_id != 0);
	
	packet_t packet;
	packet.type = pt_player;
	packet.player = game.player[0];
	packet_send(&packet);
}

static void packet_send_projectile(int i) {
	packet_t packet;
	packet.type = pt_projectile;
	packet.projectile = game.projectile[i];
	packet_send(&packet);
}

// ------------------------------------------------------------------------------

static void particle_add(position_t position, xvec2 velocity, xvec4 color, int size, float life, bool color_decay) {
	int pi = particle_find_new();
	game.particle[pi].position = position;
	game.particle[pi].fposition = xvec2_all(0.f);
	game.particle[pi].velocity = velocity;
	game.particle[pi].size = size;
	game.particle[pi].orientation = xpl_frand() * M_2PI;
	game.particle[pi].rotation = xpl_frand() * M_2PI - M_2PI;
	game.particle[pi].life = life;
	game.particle[pi].decay = color_decay;
	game.particle[pi].initial_life = life;
	game.particle[pi].initial_color = color;
	game.particle[pi].color = color;
	
	position_mod(&game.particle[pi].position);
}

static int particle_find_new(void) {
	static int last_index = 0;
	
	int test_index = (last_index + 1) % MAX_PARTICLES;
	if (game.particle[test_index].life <= 0.f) {
		last_index = test_index;
		return test_index;
	}
	
	float min_life = FLT_MAX;
	int min_index = -1;
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		if (game.particle[i].life <= 0.f) {
			last_index = i;
			return i;
		}
		if (game.particle[i].life < min_life) {
			min_life = game.particle[i].life;
			min_index = 0;
		}
	}
	last_index = min_index;
	return min_index;
}

static void particle_update(int i, double time) {
	game.particle[i].life -= time;
	game.particle[i].life = xmax(game.particle[i].life, 0.f);
	if (game.particle[i].decay) {
		game.particle[i].color.a = game.particle[i].life / game.particle[i].initial_life;
	}
	xvec2 motion = xvec2_scale(game.particle[i].velocity, time);
	game.particle[i].fposition = xvec2_add(game.particle[i].fposition, motion);
	xvec2 trunc = {{ truncf(game.particle[i].fposition.x), truncf(game.particle[i].fposition.y) }};
	game.particle[i].fposition = xvec2_sub(game.particle[i].fposition, trunc);
	game.particle[i].position.px += (int)trunc.x;
	game.particle[i].position.py += (int)trunc.y;
	game.particle[i].orientation += game.particle[i].rotation * time;
	
	position_mod(&game.particle[i].position);
}


// ------------------------------------------------------------------------------

static void player_add_damage_particle(int i) {
	particle_add(game.player[i].position, xpl_rand_xvec2(-50.f, 50.f), xvec4_set(1.f, 0.5f, 0.f, 1.f), 4, 1.f, true);
}
									   
static void player_add_thrust_particle(int i) {
	float radians = player_rotation_rads_get(i);
	radians += M_PI;
	xvec2 backward = {{ cos(radians), sin(radians) }};
	position_t back = game.player[i].position;
	back.px += (int)(backward.x * PLAYER_SIZE * 0.5f);
	back.py += (int)(backward.y * PLAYER_SIZE * 0.5f);
	xvec2 thrust_vector = xpl_rand_xvec2(-10.f, 10.f);
	particle_add(back, thrust_vector, color_variant(0xffc0c0c0, 0.2f), 4, 1.f, true);
}

#define GOODBYE_PARTICLES 100
#define GOODBYE_PARTICLE_SIZE 3
static void player_add_explode_effect(int i) {
	position_t position = game.player[i].position;
	velocity_t nil_velocity = { 0, 0 };
	game.player[i].velocity = nil_velocity;
	for (int i = 0; i < GOODBYE_PARTICLES; ++i) {
		xvec2 random_vector = xpl_rand_xvec2(-100.f, 100.f);
		xvec4 color = xvec4_set(xpl_frand_range(0.5f, 0.7f), xpl_frand_range(0.7f, 1.0f), xpl_frand_range(0.8f, 1.0f), 1.0f);
		particle_add(position, random_vector, color, GOODBYE_PARTICLE_SIZE, 5.f, true);
	}
}

static xvec2 player_calculate_oriented_thrust(int i) {
	xvec2 oriented_thrust = player_get_direction_vector(i);
	oriented_thrust = xvec2_scale(oriented_thrust, THRUST);
	return oriented_thrust;
}

static xvec2 player_get_direction_vector(int i) {
	float radians = player_rotation_rads_get(i);
	xvec2 r = {{ cos(radians), sin(radians) }};
	return r;
}

static void player_init(int i) {
	game.player[i].position.px = (PLAYFIELD_MAX / 2) + SPAWN_BOX * xpl_frand() - (SPAWN_BOX / 2);
	game.player[i].position.py = (PLAYFIELD_MAX / 2) + SPAWN_BOX * xpl_frand() - (SPAWN_BOX / 2);
	game.player[i].health = INITIAL_HEALTH;
	game.player[i].orientation = xpl_irand_range(0, UINT8_MAX);
	game.player_local[i].visible = false;
	
	if (! game.player_local[i].rotate_audio) {
		game.player_local[i].rotate_audio = audio_create("rotate.ogg");
		game.player_local[i].rotate_audio->volume = ROTATE_VOLUME;
		if (i == 0) {
			game.player_local[i].rotate_audio->loop = true;
		}
	}

	if (! game.player_local[i].thrust_audio) {
		game.player_local[i].thrust_audio = audio_create("thrust.ogg");
		game.player_local[i].thrust_audio->volume = THRUST_VOLUME;
		if (i == 0) {
			game.player_local[i].thrust_audio->loop = true;
		}
	}

	
#ifdef DEBUG
	game.player[0].score = 500;
#else
	game.player[0].score = 100;
#endif
}

static void player_local_connect(void) {
	scanline_strength = DEFAULT_SCANLINE;
	game.player_id[0].nonce = xpl_irand_range(0, UINT16_MAX);
	game.player_connected[0] = true;
	network.hello_timeout = 0.f;
	network.receive_timeout = RECEIVE_TIMEOUT;
}

static void player_local_disconnect(void) {
	game.player_connected[0] = false;
	game_reset();
}

static bool player_local_is_connected(void) {
	return game.player_connected[0] && game.player_id[0].client_id;
}

static void player_local_update_chat(void) {
	if (! chat_showing) {
		if (glfwGetKey('T')) {
			chat_buffer[0] = '\0';
			chat_showing = true;
			chat_close_wait = false;
			chat_reset_focus = true;
		}
	} else {
		
		bool key_down = false;
		if (glfwGetKey(GLFW_KEY_ESC)) {
			chat_close_wait = true;
			key_down = true;
		}
		
		if (glfwGetKey(GLFW_KEY_ENTER)) {
			if (strlen(chat_buffer)) {
				packet_send_chat();
				chat_buffer[0] = '\0';
			}
			chat_close_wait = true;
			key_down = true;
		}

		if (chat_close_wait && !key_down) {
			chat_showing = false;
			chat_close_wait = false;
		}
	}
}

static void player_local_update_firing(bool jiffy_elapsed) {
	if (jiffy_elapsed && game.fire_cooldown > 0) --game.fire_cooldown;
	
	if (chat_showing) return;
	
	if (game.fire_cooldown == 0 && key_down(fire)) {
		int w = game.active_weapon;
		int cost = projectile_config[w].price;
		if (cost == -1 || cost <= game.player[0].score) {
			if (cost > 0) game.player[0].score -= cost;
			projectile_add();
		}
	}
}

static void player_local_update_rotation(double time) {
	if (chat_showing) return;
	
	bool audio_on = false;
	if (key_down(left)) {
		game.player[0].orientation += TORQUE * time;
		game.control_indicator_on[1] = true;
		audio_on = true;
	} else {
		game.control_indicator_on[1] = false;
	}
	
	if (key_down(right)) {
		game.player[0].orientation -= TORQUE * time;
		game.control_indicator_on[2] = true;
		audio_on = true;
	} else {
		game.control_indicator_on[2] = false;
	}
	
	game.player_local[0].rotate_audio->action = audio_on ? aa_play : aa_stop;
}

static void player_local_update_thrust(double time) {
	game.player[0].is_thrust = false;
	game.control_indicator_on[0] = false;
	game.player_local[0].thrust_audio->action = aa_stop;
	
	if (chat_showing) return;
	if (! key_down(up)) return;
	
	game.player[0].is_thrust = true;
	game.player_local[0].thrust_audio->action = aa_play;
	
	game.control_indicator_on[0] = true;
	xvec2 velocity = player_v2velocity_get(0);
	xvec2 oriented_thrust = player_calculate_oriented_thrust(0);
	oriented_thrust = xvec2_scale(oriented_thrust, time);
	velocity = xvec2_add(velocity, oriented_thrust);
	LOG_DEBUG("Thrust			: %f, %f", oriented_thrust.x, oriented_thrust.y);
	LOG_DEBUG("Float velocity	: %f, %f", velocity.x, velocity.y);
	
	float speed = xvec2_length(velocity);
	if (speed > MAX_VELOCITY) {
		velocity = xvec2_scale(xvec2_scale(velocity, 1.f / speed), MAX_VELOCITY);
	}
	
	game.player[0].velocity = velocity_for_v2(velocity);
}

static void player_local_update_weapon(void) {
	const int weapon_keys[] = { '1', '2', '3', '4', '5', '6', '7', '8' };
	const int keycount = sizeof(weapon_keys) / sizeof(weapon_keys[0]);
	
	if (! chat_showing) {
		for (int i = 0; i < keycount; ++i) {
			if (glfwGetKey(weapon_keys[i]) && game.active_weapon != i) {
				game.active_weapon = i;
				audio_quickplay_pan("select.ogg", SELECT_VOLUME, 0.5f);
			}
		}
	}
	
	if (projectile_config[game.active_weapon].price > 0 &&
		projectile_config[game.active_weapon].price > game.player[0].score) {
		game.active_weapon = 0;
		audio_quickplay_pan("reject.ogg", SELECT_VOLUME, 0.5f);
	}
}

static const char *player_name(int i, bool as_object) {
	if (i == 0) {
		return as_object ? "yourself" : "You";
	}
	if (strlen(game.player_id[i].name)) return game.player_id[i].name;
	return "A Player";
}


static void player_update_position(int i) {
	game.player_position_buffer[i] = xvec2_add(game.player_position_buffer[i],
											   xvec2_set(game.player[i].velocity.dx / VELOCITY_SCALE,
														 game.player[i].velocity.dy / VELOCITY_SCALE));
	
	int dx = (int)truncf(game.player_position_buffer[i].x);
	int dy = (int)truncf(game.player_position_buffer[i].y);
	if (dx) {
		game.player[i].position.px += dx;
		game.player_position_buffer[i].x -= dx;
	}
	if (dy) {
		game.player[i].position.py += dy;
		game.player_position_buffer[i].y -= dy;
	}
	position_mod(&game.player[i].position);
}

static xvec2 player_v2velocity_get(int i) {
	return v2_for_velocity(game.player[i].velocity);
}

static int player_with_client_id_get(uint16_t client_id, bool allow_allocate, bool *was_new_player) {
	int i;
	int empty_slot = -1;
	for (i = 0; i < MAX_PLAYERS; ++i) {
		if (game.player_connected[i]) {
			if (game.player_id[i].client_id == client_id) {
				if (was_new_player) *was_new_player = false;
				return i;
			}
		} else if (empty_slot == -1) {
			empty_slot = i;
		}
	}
	
	if (! allow_allocate) {
		if (was_new_player) *was_new_player = false;
		return -1;
	}
	
	// Not found. Allocate a player slot.
	assert(empty_slot >= 1); // Shouldn't be overwriting player 0.
	
	game.player_connected[empty_slot] = true;
	game.player_id[empty_slot].client_id = client_id;
	player_init(empty_slot);
	
	if (was_new_player) *was_new_player = true;
	
	return empty_slot;
}


// ------------------------------------------------------------------------------
static void prefs_save_connect(void) {
	prefs_t prefs = prefs_get();
	strncpy(prefs.name, game.player_id[0].name, NAME_SIZE);
	strncpy(prefs.server, network.server_host, SERVER_SIZE);
	prefs_set(prefs);
}

// ------------------------------------------------------------------------------
static void projectile_add(void) {
	int weapon = game.active_weapon;
	xvec2 direction_vector = player_get_direction_vector(0);
	
	uint16_t pid = xpl_irand_range(0, UINT16_MAX);
	int i = projectile_with_pid_get(pid, weapon, NULL);
	// Add a little margin to get slow projectiles clear of the nose
	float position = projectile_type_is_mine(weapon) ? -0.7 : 0.7;
	xvec2 front = xvec2_scale(direction_vector, position * PLAYER_SIZE);
	
	game.projectile[i].position = game.player[0].position;
	game.projectile[i].position.px += (int)front.x;
	game.projectile[i].position.py += (int)front.y;
	
	position_mod(&game.projectile[i].position);
	
	velocity_t velocity = {
		roundf(projectile_config[weapon].velocity * direction_vector.x),
		roundf(projectile_config[weapon].velocity * direction_vector.y)
	};
	
	if (! projectile_type_is_mine(weapon)) {
		velocity.dx += game.player[0].velocity.dx;
		velocity.dy += game.player[0].velocity.dy;
	}
	game.projectile[i].velocity = velocity;
	
	game.fire_cooldown += projectile_config[weapon].fire_cooldown;
	
	game.projectile_local[i].owner = game.player_id[0].client_id;
	
	audio_quickplay_position(projectile_config[weapon].fire_effect, FIRE_VOLUME, v3_relative_audio(game.projectile[i].position));
	
	if (network.position_timeout > 0.1f) packet_send_player();
	packet_send_projectile(i);
}

// Create an explosion effect for the current projectile.
// If the target is nonnegative, the explosion occurs at the target.
// Otherwise the explosion occurs at the projectile.
static void projectile_explode_effect(int pi, int target) {
	
	int pt = game.projectile[pi].type;
	position_t position;
	if (projectile_type_is_mine(pt) || target == -1) {
		position = game.projectile[pi].position;
	} else {
		position = game.player[target].position;
	}
	
	for (int i = 0; i < projectile_config[pt].explode_particle_count; ++i) {
		float vel = projectile_config[pt].explode_particle_velocity;
		particle_add(position, xpl_rand_xvec2(-vel, vel),
					 color_variant(projectile_config[pt].explode_particle_color,
								   projectile_config[pt].explode_particle_color_variance),
					 projectile_config[pt].explode_particle_size,
					 projectile_config[pt].explode_particle_life, true);
	}
	game.projectile_local[pi].exploded = true;
	audio_quickplay_position(projectile_config[pt].explode_effect, EXPLODE_VOLUME, v3_relative_audio(position));

}

static void projectile_initialize(uint16_t pid, int pi, int ti) {
	game.projectile[pi].pid = pid;
	game.projectile[pi].health = projectile_config[ti].initial_health;
	game.projectile[pi].type = (uint8_t)ti;
	game.projectile_local[pi].trail_timeout = projectile_config[ti].trail_timeout;
	game.projectile_local[pi].color = color_variant(projectile_config[ti].color, projectile_config[ti].variance);
	game.projectile_local[pi].force_detonate = false;
	game.projectile_local[pi].exploded = false;
}

// Get a projectile. Pass a negative projectile type to prevent creation.
static int projectile_with_pid_get(uint16_t pid, int ti, bool *was_new) {
	int allocate_index = -1;    
	for (int i = 0; i < MAX_PROJECTILES; ++i) {
		if (pid == game.projectile[i].pid) {
			if (game.projectile[i].health > 0) {
				if (was_new) *was_new = false;
				return i;
			} else if (ti >= 0 && allocate_index == -1) {
				allocate_index = i;
				break;
			}
		} else if (ti >= 0 && allocate_index == -1 && game.projectile[i].health == 0) {
			allocate_index = i;
			// can't break, not finished searching for pid
		}
	}
	
	if (ti >= 0) {
		projectile_initialize(pid, allocate_index, ti);
		if (was_new) *was_new = true;
		return allocate_index;
	} else {
		if (was_new) *was_new = false;
		return -1;
	}
}

static bool projectile_type_is_mine(int type) {
	return (type == pt_mine || type == pt_blackhole);
}

static void projectile_update(int i, bool jiffy_elapsed) {
	if (game.projectile_local[i].exploded) {
		game.projectile[i].health = 0;
		return;
	}
	
	int64_t pdx, pdy;
	game.projectile_position_buffer[i] = xvec2_add(game.projectile_position_buffer[i],
												   xvec2_set(game.projectile[i].velocity.dx / VELOCITY_SCALE,
															 game.projectile[i].velocity.dy / VELOCITY_SCALE));
	
	int dx = (int)truncf(game.projectile_position_buffer[i].x);
	int dy = (int)truncf(game.projectile_position_buffer[i].y);
	if (dx) {
		game.projectile[i].position.px += dx;
		game.projectile_position_buffer[i].x -= dx;
	}
	if (dy) {
		game.projectile[i].position.py += dy;
		game.projectile_position_buffer[i].y -= dy;
	}
	position_mod(&game.projectile[i].position);
	
	int type = game.projectile[i].type;
	if (projectile_type_is_mine(type) || type == pt_repair) {
		// Are there any non-mine projectiles too close?
		for (int j = i + 1; j < MAX_PROJECTILES; ++j) {
			int other_type = game.projectile[j].type;
			if (other_type)
			if (game.projectile[j].health) {
				pdx = (int64_t)game.projectile[i].position.px - (int64_t)game.projectile[j].position.px;
				pdy = (int64_t)game.projectile[i].position.py - (int64_t)game.projectile[j].position.py;
				if (pdx < 10 && pdy < 10) {
					// Set both to explode or disappear.
					game.projectile_local[i].force_detonate = true;
					game.projectile_local[j].force_detonate = true;
				}
			}
		}
	}
	
	float current_explosion_radius = 0.f;
	if (jiffy_elapsed && type != pt_repair) {
		if (projectile_type_is_mine(type) && game.projectile[i].health == 2) {
			// Mines linger at 2 health
			game.projectile[i].health++;
			// They change to trail color when armed
			game.projectile_local[i].color = color_variant(projectile_config[type].trail_color, projectile_config[type].trail_life);
		}
		--game.projectile[i].health;
		--game.projectile_local[i].trail_timeout;
		if (game.projectile_local[i].trail_timeout) {
			xvec4 trail_color = color_variant(projectile_config[type].trail_color, projectile_config[type].trail_variance);
			particle_add(game.projectile[i].position, v2_for_velocity(game.projectile[i].velocity),
						 trail_color, projectile_config[type].trail_size,
						 projectile_config[type].trail_life, true);
		}
	}
	if (game.projectile[i].health == 0 || game.projectile_local[i].force_detonate) {
		current_explosion_radius = projectile_config[type].explode_radius;
	}
	
	pdx = (int64_t)game.projectile[i].position.px - (int64_t)game.player[0].position.px;
	pdy = (int64_t)game.projectile[i].position.py - (int64_t)game.player[0].position.py;
	int projectile_owner = player_with_client_id_get(game.projectile_local[i].owner, false, NULL);

	float dtt = sqrtf(labs(pdx * pdx) + labs(pdy * pdy));
	dtt -= PLAYER_SIZE * 0.5f;
	dtt = xmax(dtt, 0.f);
	
	// Black hole accelerates you in
	if (type == pt_blackhole && game.projectile[i].health == 2) {
		xvec2 direction = {{ pdx, pdy }};
		float length = dtt;
		// Fudge because black hole throws you around so fast
		if (length < 20.f) dtt = 0.f;
		xvec2 acceleration = xvec2_scale(xvec2_normalize(direction), 4.f * VELOCITY_SCALE * VELOCITY_SCALE / (length * length));
		game.player[0].velocity.dx += xclamp(acceleration.x, -512, 512);
		game.player[0].velocity.dy += xclamp(acceleration.y, -512, 512);
		LOG_DEBUG("Black hole acceleration: %f %f", acceleration.x, acceleration.y);
	}
	
	bool in_contact = (type == pt_repair) || (dtt <= current_explosion_radius);
	bool could_hit = in_contact;
	could_hit = could_hit && (projectile_config[type].can_hit_self || projectile_owner);
	
	if (current_explosion_radius > 0.f) {
		// Explode on expire is here. Otherwise wait to receive a damage packet.
		projectile_explode_effect(i, -1);
	}
	
	could_hit = could_hit || game.projectile_local[i].force_detonate;

	// Don't allow mines to double-damage.
	if (projectile_type_is_mine(type) && game.projectile[i].health != 2) could_hit = false;

	if (could_hit) {
		current_explosion_radius = projectile_config[type].explode_radius;
		
		// Apply damage
		uint8_t damage;
		if (projectile_config[type].explode_radius || projectile_type_is_mine(type)) {
			damage = (type == pt_blackhole) ? 255 : projectile_config[type].initial_health;
		} else {
			damage = in_contact ? game.projectile[i].health : 0;
		}
		if (current_explosion_radius) {
			float damage_scale = (current_explosion_radius - dtt) / current_explosion_radius;
			damage_scale = xclamp(damage_scale, 0.f, 1.f);
			damage *= damage_scale;
		}
		
		uint8_t flags = 0;
		if (type != pt_repair) {
			if (game.player[0].health <= damage) {
				game.respawn_cooldown = RESPAWN_COOLDOWN; 
				damage = game.player[0].health;
				flags |= DAMAGE_FLAG_EXPLODES;
				game.player[0].health = 0;
			} else {
				game.player[0].health -= damage;
			}
		} else {
			int health = game.player[0].health;
			health += damage;
			health = xclamp(health, 0, UINT8_MAX);
			game.player[0].health = health;
			flags |= DAMAGE_FLAG_REPAIRS;
		}
		LOG_DEBUG("Player health remaining: %u", game.player[0].health);
		
		if (damage)	{
			packet_send_damage(game.projectile_local[i].owner, game.projectile[i].pid, damage, flags);
			if (projectile_type_is_mine(type)) game.projectile[i].health = 1;

		} else {
			// Just kill the projectile
			game.projectile[i].health = 0;
			// Don't propagate this. The other clients will figure it out too.
		}
	}
}


// ------------------------------------------------------------------------------


static void server_resolve_addr(void) {
	struct addrinfo hints, *res;
	struct in_addr addr;
	int err;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_INET;
	if ((err = getaddrinfo(network.server_host, NULL, &hints, &res)) != 0) {
		LOG_ERROR("Error looking up server name %s: %d", network.server_host, err);
		ui_error_set("Server lookup failed.");
	}
	
	addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
	
	server_addr = xpl_calloc_type(UDPNET_ADDRESS);
	strncpy(server_addr->address, inet_ntoa(addr), 20);
	server_addr->port = network.server_port;
}


// ------------------------------------------------------------------------------

static float text_get_length(xpl_font_t *font, const char *text, size_t position) {
	assert(font);
	
	float len = 0;
	size_t charno = 0;
	if (position == -1)
		position = strlen(text);
	while (*text && (charno <= position)) {
		char c = (char) *text;
		if (c == '\t') {
			len += text_get_length(font, "    ", -1);
			continue;
		}
		
		xpl_glyph_t *glyph = xpl_font_get_glyph(font, c);
		len += glyph->advance_x;
		
		++text;
		++charno;
	}
	return len;
}

// ------------------------------------------------------------------------------

static void text_particle_add(position_t position, xvec2 velocity, const char *text, xvec4 color, float life) {
#ifndef XPL_PLATFORM_WINDOWSXX
	int i = text_particle_find_new();
	strncpy(game.text_particle[i].text, text, MAX_TEXT_PARTICLE_CHARS);
	game.text_particle[i].position = position;
	game.text_particle[i].fposition = xvec2_all(0.f);
	game.text_particle[i].velocity = velocity;
	game.text_particle[i].initial_color = color;
	game.text_particle[i].color = color;
	game.text_particle[i].orientation = xpl_frand_range(-M_PI_4, M_PI_4);
	game.text_particle[i].initial_life = life;
	game.text_particle[i].life = life;
	
	position_mod(&game.text_particle[i].position);
#endif
}

static int text_particle_find_new(void) {
	static int last_index = 0;
	
	int test_index = (last_index + 1) % MAX_TEXT_PARTICLES;
	if (game.text_particle[test_index].life <= 0.f) {
		last_index = test_index;
		return test_index;
	}
	
	float min_life = FLT_MAX;
	int min_index = -1;
	for (int i = 0; i < MAX_TEXT_PARTICLES; ++i) {
		if (game.text_particle[i].life <= 0.f) {
			last_index = i;
			return i;
		}
		if (game.text_particle[i].life < min_life) {
			min_life = game.text_particle[i].life;
			min_index = i;
		}
	}
	last_index = min_index;
	return min_index;
}

static void text_particle_update(int i, double time) {
	game.text_particle[i].life -= time;
	game.text_particle[i].color = xvec4_scale(game.text_particle[i].initial_color,
											  game.text_particle[i].initial_color.a * game.text_particle[i].life / game.text_particle[i].initial_life);
	xvec2 motion = xvec2_scale(game.text_particle[i].velocity, time);
	game.text_particle[i].fposition = xvec2_add(game.text_particle[i].fposition, motion);
	xvec2 trunc = {{ truncf(game.text_particle[i].fposition.x), truncf(game.text_particle[i].fposition.y) }};
	game.text_particle[i].fposition = xvec2_sub(game.text_particle[i].fposition, trunc);
	game.text_particle[i].position.px += (int)trunc.x;
	game.text_particle[i].position.py += (int)trunc.y;
	
	position_mod(&game.text_particle[i].position);
}

// ------------------------------------------------------------------------------

static void ui_error_packet_set(int eno) {
	ui_error_set("Invalid packet (%d).", eno);
	player_local_disconnect();
}

static void ui_error_set(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	vsnprintf(network.error, 256, msg, args);
	va_end(args);
	
	log_add_text(network.error);
	
	network.error_timeout = ERRORMSG_TIMEOUT;
}

static void ui_error_show(xpl_context_t *self) {
	static float scroll = 0.f;
	ui_window_start(self, xvec2_set(400, 120), xl("error"), &scroll);
	{
		xpl_imui_control_label(network.error);
		xpl_imui_separator_line();
		if (xpl_imui_control_button(xl("ok"), XPL_IMUI_BUTTON_CANCEL | XPL_IMUI_BUTTON_DEFAULT,
									network.error_timeout + 2.0f < ERRORMSG_TIMEOUT)) {
			network.error_timeout = 0.0f;
		}
	}
	ui_window_end();
}


static int name_cursor = 0, server_host_cursor = 0, server_port_cursor = 0;
static void ui_pilot_config_show(xpl_context_t *self) {
	static char server_port[32];
	static float scroll = 0.f;
	snprintf(server_port, 31, "%d", network.server_port);
	ui_window_start(self, xvec2_set(400, 280), xl("welcome"), &scroll);
	{
		xpl_imui_control_label(xl("pilot_section"));
		xpl_imui_control_textfield(xl("pilot_name_prompt"), game.player_id[0].name, NAME_SIZE, &name_cursor, "", TRUE);
		xpl_imui_separator_line();
		xpl_imui_control_label(xl("server_section"));
		xpl_imui_control_textfield(xl("server_host_prompt"), network.server_host, 128, &server_host_cursor, "", TRUE);
		xpl_imui_control_textfield(xl("server_port_prompt"), server_port, 32, &server_port_cursor, "", TRUE);
		network.server_port = atoi(server_port);
		xpl_imui_separator_line();
		if (xpl_imui_control_button(xl("ok"), XPL_IMUI_BUTTON_DEFAULT,
									strlen(game.player_id[0].name) >= 3 &&
									strlen(network.server_host) >= 3 &&
									network.server_port > 128)) {
			prefs_save_connect();
			prefs_t prefs = prefs_get();
			if (prefs.skip_tutorial) {
				player_local_connect();
			} else {
				ui_tutorial_page = 1;
			}
		}
		if (xpl_imui_control_button(xl("exit"), XPL_IMUI_BUTTON_CANCEL, TRUE)) exit(0);
	}
	ui_window_end();
}

static void ui_tutorial_highlight(xpl_context_t *self, xrect bmp_area, xrect area, double time) {
	static double accum = 0.0;
	accum += time;
	float amount = cosf(accum * 0.5);
	xvec4 color = xvec4_mix(highlight_color1, highlight_color2, amount);
	xpl_sprite_draw_colored(sprites.solid_sprite, area.x + bmp_area.x, bmp_area.height - (area.y + bmp_area.y), area.width, area.height, color);
}

static void ui_tutorial_show(xpl_context_t *self, double time) {
	scanline_strength = 0.0f;
	static int skip_tutorial = FALSE;
	int next_clicked;
	xmat4 ortho;
	xmat4_ortho(0.f, self->size.width, 0.f, self->size.height, -1.f, 1.f, &ortho);
	xpl_imui_context_begin(imui, self->app->execution_info, xrect_set(0.f, 0.f, self->size.width, self->size.height));
	{
		xpl_imui_context_area_set(xrect_set(4, 32, 320, 32));
		if (xpl_imui_control_check(xl("tutorial_skip"), skip_tutorial, TRUE)) skip_tutorial = !skip_tutorial;
		xpl_imui_context_area_set(xrect_set(self->size.width - 132, 32, 128, 32));
		next_clicked = xpl_imui_control_button(skip_tutorial ? xl("tutorial_skip") : xl("tutorial_next"), XPL_IMUI_BUTTON_CANCEL | XPL_IMUI_BUTTON_DEFAULT, TRUE);
	}
	xpl_imui_context_end(imui);

	xpl_sprite_batch_begin(sprites.ui_batch);
	{
		xmat4 *mat = xpl_sprite_batch_matrix_push(sprites.ui_batch);
		*mat = ortho;
		
		xrect rect = xrect_set(0.f, 0.f, self->size.width, self->size.height);
		rect = xrect_contract_to(rect, 714, 536);
		rect.y = 0;
		xpl_sprite_draw(sprites.ui_tutorial_pages[ui_tutorial_page - 1], rect.x, rect.y, rect.width, rect.height);
		
		switch (ui_tutorial_page) {
			case 1:
				break;
				
			case 2:
				ui_tutorial_highlight(self, rect, xrect_set(61, 438, 121, 41), time);
				ui_tutorial_highlight(self, rect, xrect_set(330, 265, 51, 42), time);
				break;
				
			case 3:
				ui_tutorial_highlight(self, rect, xrect_set(144, 287, 30, 12), time);
				ui_tutorial_highlight(self, rect, xrect_set(177, 80, 98, 22), time);
				ui_tutorial_highlight(self, rect, xrect_set(248, 453, 315, 19), time);
				ui_tutorial_highlight(self, rect, xrect_set(590, 448, 53, 51), time);
				break;

			case 4:
				ui_tutorial_highlight(self, rect, xrect_set(366, 452, 39, 55), time);
				break;

			case 5:
				ui_tutorial_highlight(self, rect, xrect_set(56, 124, 600, 72), time);
				break;
				
			case 6:
				ui_tutorial_highlight(self, rect, xrect_set(57, 117, 600, 12), time);
				ui_tutorial_highlight(self, rect, xrect_set(57, 373, 12, 256), time);
				ui_tutorial_highlight(self, rect, xrect_set(644, 373, 12, 256), time);
				ui_tutorial_highlight(self, rect, xrect_set(57, 385, 600, 12), time);
				break;

			default:
				break;
		}
		
		xpl_sprite_batch_matrix_pop(sprites.ui_batch);
	}
	xpl_sprite_batch_end(sprites.ui_batch);
	
	xvec2 pen = {{ 4.f, 64.f }};
	if (ui_tutorial_page == 1) {
		xpl_text_buffer_clear(tutorial_buffer);
		wchar_t buffer[1024];
		char key_name[64];
		snprintf(key_name, 64, "tutorial_%d", ui_tutorial_page - 1);
		mbstowcs(buffer, xl(key_name), 1024);
		xpl_text_buffer_add_text(tutorial_buffer, &pen, &tutorial_markup, buffer, 0);
		xpl_text_buffer_commit(tutorial_buffer);
	}
	
	xpl_text_buffer_render(tutorial_buffer, ortho.data);
	
	if (next_clicked) {
		if (skip_tutorial) {
			ui_tutorial_page = TUTORIAL_PAGES + 1;
		} else {
			++ui_tutorial_page;
		}
	}

	if (ui_tutorial_page > TUTORIAL_PAGES) {
		prefs_t prefs = prefs_get();
		prefs.skip_tutorial = true;
		prefs_set(prefs);
		ui_tutorial_page = 0;
		player_local_connect();
	} else {
		// Set up text for next page
		xpl_text_buffer_clear(tutorial_buffer);
		wchar_t buffer[1024];
		char key_name[64];
		snprintf(key_name, 64, "tutorial_%d", ui_tutorial_page - 1);
		mbstowcs(buffer, xl(key_name), 1024);
		xpl_text_buffer_add_text(tutorial_buffer, &pen, &tutorial_markup, buffer, 0);
		xpl_text_buffer_commit(tutorial_buffer);
	}
}

static void ui_window_end(void) {
	xpl_imui_control_scroll_area_end();
	xpl_imui_context_end(imui);
}

static void ui_window_start(xpl_context_t *self, xvec2 size, const char *title, float *scroll) {
	xrect area = xrect_set(0.f, 0.f, self->size.width, self->size.height);
	area = xrect_contract_to(area, size.width, size.height);
	xpl_imui_context_begin(imui, self->app->execution_info, area);
	xpl_imui_control_scroll_area_begin(title, area.size, scroll);
}


// ------------------------------------------------------------------------------

static xvec2 v2_for_velocity(velocity_t velocity) {
	xvec2 result = xvec2_set((float)velocity.dx, (float)velocity.dy);
	result = xvec2_scale(result, 1.0f / VELOCITY_SCALE);
	return result;
}

static velocity_t velocity_for_v2(xvec2 v) {
	xvec2 scalev = xvec2_scale(v, VELOCITY_SCALE);
	velocity_t result = { (int16_t)roundf(scalev.x), (int16_t)roundf(scalev.y) };
	
	// handle overflow
	if (xpl_signum(result.dx) == -xpl_signum(v.x)) {
		result.dx = INT16_MAX * (int)xpl_signum(v.x);
	}
	if (xpl_signum(result.dy) == -xpl_signum(v.y)) {
		result.dy = INT16_MAX * (int)xpl_signum(v.y);
	}
	
	return result;
}

#define POSITION_SCALE 0.01625
static xvec3 v3_relative_audio(position_t position) {
	xvec3 r = {{
		POSITION_SCALE * ((int64_t)position.px - (int64_t)camera.center.px),
		POSITION_SCALE * ((int64_t)position.py - (int64_t)camera.center.py),
		0.f
	}};
	return r;
}

// ------------------------------------------------------------------------------


xpl_context_def_t game_context_def = {
	game_init,
	game_engine,
	game_render,
	game_destroy,
	game_handoff
};


