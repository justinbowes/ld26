//
//  ILViewController.m
//  UltraPew-IOS
//
//  Created by Justin Bowes on 2013-05-13.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include "xpl.h"
#include "xpl_gl.h"
#include "xpl_app.h"
#include "xpl_thread.h"
#include "xpl_engine_info.h"
#include "xpl_text_buffer.h"
#include "xpl_vec.h"

#include "audio/audio.h"

#include "context/context_snapshot.h"
#include "context/context_logo.h"
#include "context/context_game.h"

#include "xpl_input_ios.h"

#import "ILViewController.h"

#define SKIP_MENU

@interface ILViewController () {
	xpl_app_t *app;
	
	xpl_context_t *draw_context;
	void *context_data;
	bool context_needs_init;

	xpl_context_t *snapshot_context;
	
	double total_time;
	int frame_counter;
	bool isDrawingSnapshot;
}

@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;
- (void)createInitialContext;

@end

@implementation ILViewController

- (void)dealloc
{
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [_context release];
    [super dealloc];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
	[view setMultipleTouchEnabled:YES];
    view.context = self.context;
	view.delegate = self;
    view.drawableDepthFormat = GLKViewDrawableDepthFormatNone;
	
	isDrawingSnapshot = NO;
	
	xpl_input_ios_init(view);
	root_view_controller = self;
	
	total_time = 0.0;
	frame_counter = 0;
	app = xpl_app_new(0, NULL);
	
	app->execution_info = xpl_engine_execution_info_new();
	app->engine_info = xpl_engine_info_new();
	
	xpl_l10n_set_fallback_locale("en");
	xpl_l10n_load_saved_locale();
	
    xpl_threads_init(4, NULL);
	xpl_init_timer();

	audio_startup();

    [self setupGL];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];

    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
        
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }

	if (! context_needs_init) {
		draw_context->functions.destroy(draw_context, context_data);
		context_needs_init = true;
	}
}

- (void)updateAppAreaWithRect:(CGRect)rect {
	float scaleFactor = [UIScreen mainScreen].scale;
	app->execution_info->screen_size.x = rect.size.width * scaleFactor;
	app->execution_info->screen_size.y = rect.size.height * scaleFactor;
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
	xpl_shaders_init("shaders/", ".glsl");
	
	snapshot_context = xpl_context_new(app, &snapshot_context_def);
	snapshot_context->functions.init(snapshot_context);
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];

	if (draw_context) {
		draw_context->functions.destroy(draw_context, context_data);
		draw_context = NULL;
	}
	snapshot_context->functions.destroy(snapshot_context, NULL);
	
	audio_shutdown();
	xpl_threads_shutdown();
	xpl_shaders_shutdown();
	xpl_input_ios_destroy();
}

- (void)createInitialContext
{
	assert(! draw_context);
#ifndef SKIP_MENU
	draw_context = xpl_context_new(app, &logo_context_def);
#else
	draw_context = xpl_context_new(app, &game_context_def);
#endif
	context_needs_init = true;
}

#pragma mark - GLKView and GLKViewController delegate methods

- (UIImage *)snapshot {
	isDrawingSnapshot = YES;
	GLKView *view = (GLKView *)self.view;
	UIImage *result = [view snapshot];
	isDrawingSnapshot = NO;
	return result;
}

- (void)update
{
	if (! draw_context) {
		// Annoyingly, we don't really have any guaranteee that the rect
		// we're using here for initialization is the same as the one
		// used later for rendering.
		[self createInitialContext];
	}
	
	if (context_needs_init) {
		context_data = draw_context->functions.init(draw_context);
		context_needs_init = false;
		return;
	}
	
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
	
	app->execution_info->current_time = xpl_get_time();
	
	app->execution_info->remaining_time_to_process = self.timeSinceLastUpdate;
	
	audio_update();

	int engine_steps = 0;
	while ((app->execution_info->remaining_time_to_process >= app->engine_info->timestep) &&
		   (engine_steps < 10)) {
		xpl_context_t *next_context = draw_context->functions.handoff(draw_context, context_data);
		if (next_context != draw_context) {
			draw_context->functions.destroy(draw_context, context_data);
			draw_context = next_context;
			if (! draw_context) {
				[self createInitialContext];
			}
			draw_context->functions.init(draw_context);
		}
		draw_context->functions.engine(draw_context, app->engine_info->timestep, context_data);
		app->execution_info->remaining_time_to_process -= app->engine_info->timestep;
		++engine_steps;
	}
	LOG_DEBUG("Engine steps: %d, engine lag %f", engine_steps, app->execution_info->remaining_time_to_process);
}

- (void)drawSnapshot {
	snapshot_context->functions.engine(snapshot_context, 0.0, NULL);
	snapshot_context->functions.render(snapshot_context, 0.0, NULL);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
	[self updateAppAreaWithRect:rect];
	glViewport(0, 0, app->execution_info->screen_size.x, app->execution_info->screen_size.y);

	if (isDrawingSnapshot) {
		[self drawSnapshot];
		return;
	}
	
	// Draw is called before update. I have a problem with this design.
	if (! draw_context) return;
	if (context_needs_init) return;

	double initial_time = app->execution_info->current_time;
	double current_time = xpl_get_time();
	double engine_time = current_time - initial_time;
	double render_interval = self.timeSinceLastDraw;
	app->execution_info->current_time = current_time;
	draw_context->size = app->execution_info->screen_size;
	draw_context->functions.render(draw_context, render_interval, context_data);
	
	current_time = xpl_get_time();
	double render_time = current_time - app->execution_info->current_time;
	
	total_time = current_time - initial_time;
	app->execution_info->time_delta = total_time;
	
	xpl_log_times(app->execution_info, engine_time, 0.0, render_time, total_time);
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	size_t index = 0;
	for (UITouch *touch in touches) {
		CGPoint location = [touch locationInView:self.view];
		xpl_input_ios_set_touch_began(&location);
		++index;
	}
	[super touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	size_t index = 0;
	for (UITouch *touch in touches) {
		CGPoint location = [touch locationInView:self.view];
		xpl_input_ios_set_touch_moved(&location);
		++index;
	}
	[super touchesMoved:touches withEvent:event];
}


- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	size_t index = 0;
	for (UITouch *touch in touches) {
		CGPoint location = [touch locationInView:self.view];
		xpl_input_ios_set_touch_ended(&location);
		++index;
	}
	[super touchesCancelled:touches withEvent:event];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	size_t index = 0;
	for (UITouch *touch in touches) {
		CGPoint location = [touch locationInView:self.view];
		xpl_input_ios_set_touch_ended(&location);
		++index;
	}
	[super touchesEnded:touches withEvent:event];
}

@end
