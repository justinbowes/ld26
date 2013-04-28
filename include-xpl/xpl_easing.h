//
//  xpl_easing.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-10-06.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_easing_h
#define xpl_osx_xpl_easing_h

#include "xpl.h"

#ifdef XPL_EASING_USE_DBL_PRECIS
#define xpl_ease_float double
#else
#define xpl_ease_float float
#endif

typedef xpl_ease_float (*xpl_easing_function)(xpl_ease_float);

// Linear interpolation (no easing)
xpl_ease_float xpl_ease_linear(xpl_ease_float p);

// Quadratic easing; p^2
xpl_ease_float xpl_ease_quadratic_in(xpl_ease_float p);
xpl_ease_float xpl_ease_quadratic_out(xpl_ease_float p);
xpl_ease_float xpl_ease_quadratic_in_out(xpl_ease_float p);

// Cubic easing; p^3
xpl_ease_float xpl_ease_cubic_in(xpl_ease_float p);
xpl_ease_float xpl_ease_cubic_out(xpl_ease_float p);
xpl_ease_float xpl_ease_cubic_in_out(xpl_ease_float p);

// Quartic easing; p^4
xpl_ease_float xpl_ease_quartic_in(xpl_ease_float p);
xpl_ease_float xpl_ease_quartic_out(xpl_ease_float p);
xpl_ease_float xpl_ease_quartic_in_out(xpl_ease_float p);

// Quintic easing; p^5
xpl_ease_float xpl_ease_quintic_in(xpl_ease_float p);
xpl_ease_float xpl_ease_quintic_out(xpl_ease_float p);
xpl_ease_float xpl_ease_quintic_in_out(xpl_ease_float p);

// Sine wave easing; sin(p * PI/2)
xpl_ease_float xpl_ease_sine_in(xpl_ease_float p);
xpl_ease_float xpl_ease_sine_out(xpl_ease_float p);
xpl_ease_float xpl_ease_sine_in_out(xpl_ease_float p);

// Circular easing; sqrt(1 - p^2)
xpl_ease_float xpl_ease_circular_in(xpl_ease_float p);
xpl_ease_float xpl_ease_circular_out(xpl_ease_float p);
xpl_ease_float xpl_ease_circular_in_out(xpl_ease_float p);

// Exponential easing, base 2
xpl_ease_float xpl_ease_exponential_in(xpl_ease_float p);
xpl_ease_float xpl_ease_exponential_out(xpl_ease_float p);
xpl_ease_float xpl_ease_exponential_in_out(xpl_ease_float p);

// Exponentially-damped sine wave easing
xpl_ease_float xpl_ease_elastic_in(xpl_ease_float p);
xpl_ease_float xpl_ease_elastic_out(xpl_ease_float p);
xpl_ease_float xpl_ease_elastic_in_out(xpl_ease_float p);

// Overshooting cubic easing;
xpl_ease_float xpl_ease_back_in(xpl_ease_float p);
xpl_ease_float xpl_ease_back_out(xpl_ease_float p);
xpl_ease_float xpl_ease_back_in_out(xpl_ease_float p);

// Exponentially-decaying bounce easing
xpl_ease_float xpl_ease_bounce_in(xpl_ease_float p);
xpl_ease_float xpl_ease_bounce_out(xpl_ease_float p);
xpl_ease_float xpl_ease_bounce_in_out(xpl_ease_float p);

typedef enum xpl_ease_func_id {
	xef_linear = 0,
	xef_quadratic_in,
	xef_quadratic_out,
	xef_quadratic_in_out,
	xef_cubic_in,
	xef_cubic_out,
	xef_cubic_in_out,
	xef_quartic_in,
	xef_quartic_out,
	xef_quartic_in_out,
	xef_quintic_in,
	xef_quintic_out,
	xef_quintic_in_out,
	xef_sine_in,
	xef_sine_out,
	xef_sine_in_out,
	xef_circular_in,
	xef_circular_out,
	xef_circular_in_out,
	xef_exponential_in,
	xef_exponential_out,
	xef_exponential_in_out,
	xef_elastic_in,
	xef_elastic_out,
	xef_elastic_in_out,
	xef_back_in,
	xef_back_out,
	xef_back_in_out,
	xef_bounce_in,
	xef_bounce_out,
	xef_bounce_in_out,
	xef_last = xef_bounce_in_out
} xpl_ease_func_id_t;

extern xpl_easing_function xpl_easing_functions[];

#endif
