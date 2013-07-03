//
//  xpl_engine_info.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-01.
//  Copyright (c) 2012 Justin Bowes. All rights reserved.
//

#ifndef xpl_osx_engine_info_h
#define xpl_osx_engine_info_h

#include <stdbool.h>

#include "xpl.h"
#include "xpl_vec.h"

// ---- engine info -- configuration parameters for the engine
typedef struct xpl_engine_info {
	double                  timestep;
	double                  max_engine_interval;
	double					time;
} xpl_engine_info_t;

xpl_engine_info_t *xpl_engine_info_new(void);
void xpl_engine_info_destroy(xpl_engine_info_t **engine_info);


// ---- execution info -- tracking execution of the engine

typedef struct xpl_execution_stats {
	double                  engine_time;
	double                  interpolation_time;
	double                  render_time;
	double                  all_time;

	struct xpl_execution_stats *prev;
	struct xpl_execution_stats *next;
} xpl_execution_stats_t;

typedef struct xpl_engine_execution_info {

	double                  remaining_time_to_process;
	double                  time_delta;
	double					current_time;
	size_t                  retain_execution_sample_count;
	xpl_execution_stats_t   *execution_stats;
    xpl_execution_stats_t   total_stats;
    uint64_t                frame_count;
	xivec2					screen_size;

} xpl_engine_execution_info_t;



xpl_engine_execution_info_t *xpl_engine_execution_info_new(void);
void xpl_engine_execution_info_destroy(xpl_engine_execution_info_t **execution_info);
void xpl_log_times(xpl_engine_execution_info_t *execution_info, double engine_time, double interpolation_time, double render_time, double all_time);
void xpl_average_times(xpl_engine_execution_info_t *execution_info, xpl_execution_stats_t *stats_out);
void xpl_last_times(xpl_engine_execution_info_t *execution_info, xpl_execution_stats_t *stats_out);




#endif
