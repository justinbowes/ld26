
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xpl.h"
#include "xpl_gl.h"
#include "xpl_app.h"
#include "xpl_thread.h"
#include "xpl_engine_info.h"
#include "xpl_text_buffer.h"
#include "xpl_vec.h"

#include "xpl_context.h"

#include "audio/audio.h"

#include "context/context_logo.h"
#include "context/context_game.h"

static int frame_counter = 0;
static xpl_context_t *context = NULL;
static void *context_data;

static void parse_args(xpl_app_t *app) {
    int c;
    while ((c = getopt(app->argc, app->argv, "rs:x")) != -1) {
        switch (c) {
            case '?':
            {
                if (optopt == '?') {
                    fprintf(stdout, "%s --------------------------------------", app->argv[0]);
                    fprintf(stdout, "-r         Reset graphics settings");
                    fprintf(stdout, "-x         Start new game immediately");
                    exit(0);
                }
                break;
            }
                
            case 'r':
            {
				LOG_INFO("Reset graphics settings");
                xpl_app_params_t defaults = xpl_app_params_load(TRUE);
                xpl_app_params_save(defaults);
                app->restart = true;
                break;
            }
                
            case 'x':
            {
                LOG_INFO("Skipping menu");
                context = xpl_context_new(app, &game_context_def);
                break;
            }
                
            default:
            {
                LOG_DEBUG("Unrecognized option %c ignored", c);
                break;
            }
        }
    }
}

static void init(xpl_app_t *app) {
    // xpl_threads_init(4, NULL);
	xpl_init_timer();
	xpl_input_init();
	xpl_shaders_init("shaders/", ".glsl");
	audio_startup();
    
    if (! app->did_restart) {
        parse_args(app);
    }

    if (context == NULL) {
        context = xpl_context_new(app, &logo_context_def);
    }
    context_data = context->functions.init(context);
}

static void destroy(xpl_app_t *app) {
    if (context) {
        context->functions.destroy(context, context_data);
        context = NULL;
    }
    
	audio_shutdown();
    // xpl_threads_shutdown();
    xpl_shaders_shutdown();
}

static int GLFWCALL glfw_window_close_fun() {
    return TRUE;
}

static void main_loop(xpl_app_t *app) {

	glfwSwapInterval(app->display_params.is_framelimit ? 1 : 0);
    glfwSetWindowCloseCallback(glfw_window_close_fun);

	double initial_time = glfwGetTime();
	app->execution_info->current_time = initial_time;
	app->execution_info->remaining_time_to_process = 0.0;
	
	double engine_time = 0.0, engine_start_time = initial_time;
	double render_interval = 0.0, render_time = 0.0, render_start_time = initial_time;
	
	while(glfwGetWindowParam(GLFW_OPENED) && !app->restart) {
		double current_time = glfwGetTime();
		double last_frame_time = current_time - /* last */ engine_start_time;
		app->execution_info->current_time = current_time;
		app->execution_info->remaining_time_to_process += last_frame_time;
		engine_start_time = current_time;

        if (app->execution_info->remaining_time_to_process > app->engine_info->max_engine_interval) {
            app->execution_info->remaining_time_to_process = app->engine_info->max_engine_interval;
            LOG_DEBUG("Clipping engine time to %f", app->execution_info->remaining_time_to_process);
        }
		
		xpl_log_times(app->execution_info, engine_time, 0.0, render_time, last_frame_time);
		frame_counter++;
		if (frame_counter >= 1000) {
			xpl_execution_stats_t stats_out;
			xpl_average_times(app->execution_info, &stats_out);
			if (stats_out.all_time > 0.0f) {
				LOG_DEBUG("Stats: %f FPS (%f average = %f engine + %f render)",
                          1.0f / stats_out.all_time,
                          stats_out.all_time, stats_out.engine_time, stats_out.render_time);
			}
			frame_counter = 0;
		}
		
		glfwGetWindowSize(&app->execution_info->screen_size.x, &app->execution_info->screen_size.y);

		// Once per frame regardless of the frame rate.
		audio_update();

		while (app->execution_info->remaining_time_to_process >= app->engine_info->timestep) {
			xpl_context_t *next_context = context->functions.handoff(context, context_data);
			if (next_context != context) {
				context->functions.destroy(context, context_data);
				context = next_context;
				if (! context) break;
				context_data = context->functions.init(context);
			}
			// Step();
			context->functions.engine(context, app->engine_info->timestep, context_data);

			app->execution_info->remaining_time_to_process -= app->engine_info->timestep;
		}
		if (! context) break;

		current_time = glfwGetTime();
		engine_time = current_time - engine_start_time;

		// Render();
		render_interval = current_time - render_start_time;
		render_start_time = current_time;
		context->functions.render(context, render_interval, context_data);
		glfwSwapBuffers();

		current_time = glfwGetTime();
		render_time = current_time - render_start_time;
    }

    xpl_execution_stats_t total_stats = app->execution_info->total_stats;
    LOG_DEBUG("TOTAL Stats: %f FPS (%f average = %f engine + %f render, %lu frames)",
              (double)app->execution_info->frame_count / total_stats.all_time,
              total_stats.all_time, total_stats.engine_time, total_stats.render_time,
              (unsigned long)app->execution_info->frame_count);


}

int main(int argc, char *argv[]) {
	xpl_app_t *app = xpl_app_new(argc, argv);
	app->init_func = init;
	app->destroy_func = destroy;
	app->main_loop_func = main_loop;
	app->title = "ULTRAPEW!";

	xpl_start_app(app);

	return EXIT_SUCCESS;
}
