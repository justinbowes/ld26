//
//  engine_info.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-01.
//  Copyright (c) 2012 Justin Bowes. All rights reserved.
//

#include <minIni.h>
#include <utlist.h>
#include <math.h>

#include "xpl_memory.h"
#include "xpl_app.h"
#include "xpl_engine_info.h"

// --- xpl_engine_info

xpl_engine_info_t *xpl_engine_info_new() {
	xpl_engine_info_t *engine_info = xpl_calloc_type(xpl_engine_info_t);
	engine_info->timestep = 1.0
			/ (double) (ini_getl("Engine", "frequency", 60,
								"engine.ini"));
	engine_info->max_engine_interval = 1.0
			/ (double) (ini_getl("Engine", "min_fps", 4,
								"engine.ini"));
	return engine_info;
}

void xpl_engine_info_destroy(xpl_engine_info_t **engine_info) {
	xpl_free(*engine_info);
	*engine_info = NULL;
}

// --- xpl_engine_execution_info

void xpl_log_times(xpl_engine_execution_info_t *execution_info,
				   double engine_time, double interpolation_time, double render_time,
				   double all_time) {
	size_t retain_sample_count = execution_info->retain_execution_sample_count;
	xpl_execution_stats_t *head = execution_info->execution_stats, *it = NULL, *tmp = NULL;

	xpl_execution_stats_t *new_node = xpl_alloc_type(xpl_execution_stats_t);
	new_node->all_time = all_time;
	new_node->engine_time = engine_time;
	new_node->interpolation_time = interpolation_time;
	new_node->render_time = render_time;
    
	if (execution_info->frame_count > 0) {
		execution_info->total_stats.all_time += all_time;
		execution_info->total_stats.engine_time += engine_time;
		execution_info->total_stats.interpolation_time += interpolation_time;
		execution_info->total_stats.render_time += render_time;
	}
    
    execution_info->frame_count++;

	DL_APPEND(head, new_node);
	size_t count = 0;

	DL_FOREACH(head, it) {
		count++;
	}
    if (count > retain_sample_count && head) {
        DL_FOREACH_SAFE(head, it, tmp) {
            DL_DELETE(head, it);
            xpl_free(it);
            count--;
            if (count <= retain_sample_count) break;
        }
	}
	execution_info->execution_stats = head;
}

void xpl_average_times(xpl_engine_execution_info_t *execution_info,
					   xpl_execution_stats_t *stats_out) {
	xpl_execution_stats_t *stats = execution_info->execution_stats, *head =
			stats;

	stats_out->engine_time = 0.;
	stats_out->render_time = 0.;
	stats_out->all_time = 0.;
	if (head == NULL)
		return;

	double count = 0.0;

	DL_FOREACH(head, stats) {
		stats_out->all_time += stats->all_time;
		stats_out->engine_time += stats->engine_time;
		stats_out->interpolation_time += stats->interpolation_time;
		stats_out->render_time += stats->render_time;
		count += 1.0;
	}
	stats_out->all_time /= count;
	stats_out->engine_time /= count;
	stats_out->interpolation_time /= count;
	stats_out->render_time /= count;
}

void xpl_last_times(xpl_engine_execution_info_t *execution_info,
					xpl_execution_stats_t *stats_out) {

	xpl_execution_stats_t *stats = execution_info->execution_stats;
	if (stats == NULL) {
		stats_out->all_time = 0.;
		stats_out->engine_time = 0.;
		stats_out->interpolation_time = 0.;
		stats_out->render_time = 0.;
		return;
	}

	// UT doubly-linked lists have the tail linked to head->prev.
	stats = stats->prev;

	stats_out->all_time = stats->all_time;
	stats_out->engine_time = stats->engine_time;
	stats_out->interpolation_time = stats->interpolation_time;
	stats_out->render_time = stats->render_time;
}

xpl_engine_execution_info_t *xpl_engine_execution_info_new() {
	xpl_engine_execution_info_t *execution_info =
			xpl_calloc_type(xpl_engine_execution_info_t);
	execution_info->remaining_time_to_process = (double) (ini_getf("Engine", "runahead", 1.0, "engine.ini"));
	execution_info->time_delta = 0.0;

	execution_info->execution_stats = NULL;
	execution_info->retain_execution_sample_count = 5;

	return execution_info;
}

void xpl_engine_execution_info_destroy(
									   xpl_engine_execution_info_t **execution_info) {
	xpl_execution_stats_t *stats = (*execution_info)->execution_stats;
	while (stats != NULL ) {
		DL_DELETE(stats, stats);
	}

	xpl_free(*execution_info);
	*execution_info = NULL;
}

