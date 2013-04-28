//
//  xpl_easing.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-10-06.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <math.h>

#include "xpl_math.h"
#include "xpl_easing.h"

xpl_easing_function xpl_easing_functions[] = {
	xpl_ease_linear,
	xpl_ease_quadratic_in,
	xpl_ease_quadratic_out,
	xpl_ease_quadratic_in_out,
	xpl_ease_cubic_in,
	xpl_ease_cubic_out,
	xpl_ease_cubic_in_out,
	xpl_ease_quartic_in,
	xpl_ease_quartic_out,
	xpl_ease_quartic_in_out,
	xpl_ease_quintic_in,
	xpl_ease_quintic_out,
	xpl_ease_quintic_in_out,
	xpl_ease_sine_in,
	xpl_ease_sine_out,
	xpl_ease_sine_in_out,
	xpl_ease_circular_in,
	xpl_ease_circular_out,
	xpl_ease_circular_in_out,
	xpl_ease_exponential_in,
	xpl_ease_exponential_out,
	xpl_ease_exponential_in_out,
	xpl_ease_elastic_in,
	xpl_ease_elastic_out,
	xpl_ease_elastic_in_out,
	xpl_ease_back_in,
	xpl_ease_back_out,
	xpl_ease_back_in_out,
	xpl_ease_bounce_in,
	xpl_ease_bounce_out,
	xpl_ease_bounce_in_out
};

// Modeled after the line y = x
xpl_ease_float xpl_ease_linear(xpl_ease_float p)
{
	return p;
}

// Modeled after the parabola y = x^2
xpl_ease_float xpl_ease_quadratic_in(xpl_ease_float p)
{
	return p * p;
}

// Modeled after the parabola y = -x^2 + 2x
xpl_ease_float xpl_ease_quadratic_out(xpl_ease_float p)
{
	return -(p * (p - 2));
}

// Modeled after the piecewise quadratic
// y = (1/2)((2x)^2)             ; [0, 0.5)
// y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
xpl_ease_float xpl_ease_quadratic_in_out(xpl_ease_float p)
{
	if(p < 0.5)
	{
		return 2 * p * p;
	}
	else
	{
		return (-2 * p * p) + (4 * p) - 1;
	}
}

// Modeled after the cubic y = x^3
xpl_ease_float xpl_ease_cubic_in(xpl_ease_float p)
{
	return p * p * p;
}

// Modeled after the cubic y = (x - 1)^3 + 1
xpl_ease_float xpl_ease_cubic_out(xpl_ease_float p)
{
	xpl_ease_float f = (p - 1);
	return f * f * f + 1;
}

// Modeled after the piecewise cubic
// y = (1/2)((2x)^3)       ; [0, 0.5)
// y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
xpl_ease_float xpl_ease_cubic_in_out(xpl_ease_float p)
{
	if(p < 0.5)
	{
		return 4 * p * p * p;
	}
	else
	{
		xpl_ease_float f = ((2 * p) - 2);
		return 0.5 * f * f * f + 1;
	}
}

// Modeled after the quartic x^4
xpl_ease_float xpl_ease_quartic_in(xpl_ease_float p)
{
	return p * p * p * p;
}

// Modeled after the quartic y = 1 - (x - 1)^4
xpl_ease_float xpl_ease_quartic_out(xpl_ease_float p)
{
	xpl_ease_float f = (p - 1);
	return f * f * f * (1 - p) + 1;
}

// Modeled after the piecewise quartic
// y = (1/2)((2x)^4)        ; [0, 0.5)
// y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
xpl_ease_float xpl_ease_quartic_in_out(xpl_ease_float p)
{
	if(p < 0.5)
	{
		return 8 * p * p * p * p;
	}
	else
	{
		xpl_ease_float f = (p - 1);
		return -8 * f * f * f * f + 1;
	}
}

// Modeled after the quintic y = x^5
xpl_ease_float xpl_ease_quintic_in(xpl_ease_float p)
{
	return p * p * p * p * p;
}

// Modeled after the quintic y = (x - 1)^5 + 1
xpl_ease_float xpl_ease_quintic_out(xpl_ease_float p)
{
	xpl_ease_float f = (p - 1);
	return f * f * f * f * f + 1;
}

// Modeled after the piecewise quintic
// y = (1/2)((2x)^5)       ; [0, 0.5)
// y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
xpl_ease_float xpl_ease_quintic_in_out(xpl_ease_float p)
{
	if(p < 0.5)
	{
		return 16 * p * p * p * p * p;
	}
	else
	{
		xpl_ease_float f = ((2 * p) - 2);
		return  0.5 * f * f * f * f * f + 1;
	}
}

// Modeled after quarter-cycle of sine wave
xpl_ease_float xpl_ease_sine_in(xpl_ease_float p)
{
	return sin((p - 1) * M_PI_2) + 1;
}

// Modeled after quarter-cycle of sine wave (different phase)
xpl_ease_float xpl_ease_sine_out(xpl_ease_float p)
{
	return sin(p * M_PI_2);
}

// Modeled after half sine wave
xpl_ease_float xpl_ease_sine_in_out(xpl_ease_float p)
{
	return 0.5 * (1 - cos(p * M_PI));
}

// Modeled after shifted quadrant IV of unit circle
xpl_ease_float xpl_ease_circular_in(xpl_ease_float p)
{
	return 1 - sqrt(1 - (p * p));
}

// Modeled after shifted quadrant II of unit circle
xpl_ease_float xpl_ease_circular_out(xpl_ease_float p)
{
	return sqrt((2 - p) * p);
}

// Modeled after the piecewise circular function
// y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)
// y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
xpl_ease_float xpl_ease_circular_in_out(xpl_ease_float p)
{
	if(p < 0.5)
	{
		return 0.5 * (1 - sqrt(1 - 4 * (p * p)));
	}
	else
	{
		return 0.5 * (sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
	}
}

// Modeled after the exponential function y = 2^(10(x - 1))
xpl_ease_float xpl_ease_exponential_in(xpl_ease_float p)
{
	return (p == 0.0) ? p : pow(2, 10 * (p - 1));
}

// Modeled after the exponential function y = -2^(-10x) + 1
xpl_ease_float xpl_ease_exponential_out(xpl_ease_float p)
{
	return (p == 1.0) ? p : 1 - pow(2, -10 * p);
}

// Modeled after the piecewise exponential
// y = (1/2)2^(10(2x - 1))         ; [0,0.5)
// y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
xpl_ease_float xpl_ease_exponential_in_out(xpl_ease_float p)
{
	if(p == 0.0 || p == 1.0) return p;
	
	if(p < 0.5)
	{
		return 0.5 * pow(2, (20 * p) - 10);
	}
	else
	{
		return -0.5 * pow(2, (-20 * p) + 10) + 1;
	}
}

// Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
xpl_ease_float xpl_ease_elastic_in(xpl_ease_float p)
{
	return sin(13 * M_PI_2 * p) * pow(2, 10 * (p - 1));
}

// Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1
xpl_ease_float xpl_ease_elastic_out(xpl_ease_float p)
{
	return sin(-13 * M_PI_2 * (p + 1)) * pow(2, -10 * p) + 1;
}

// Modeled after the piecewise exponentially-damped sine wave:
// y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)
// y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
xpl_ease_float xpl_ease_elastic_in_out(xpl_ease_float p)
{
	if(p < 0.5)
	{
		return 0.5 * sin(13 * M_PI_2 * (2 * p)) * pow(2, 10 * ((2 * p) - 1));
	}
	else
	{
		return 0.5 * (sin(-13 * M_PI_2 * ((2 * p - 1) + 1)) * pow(2, -10 * (2 * p - 1)) + 2);
	}
}

// Modeled after the overshooting cubic y = x^3-x*sin(x*pi)
xpl_ease_float xpl_ease_back_in(xpl_ease_float p)
{
	return p * p * p - p * sin(p * M_PI);
}

// Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
xpl_ease_float xpl_ease_back_out(xpl_ease_float p)
{
	xpl_ease_float f = (1 - p);
	return 1 - (f * f * f - f * sin(f * M_PI));
}

// Modeled after the piecewise overshooting cubic function:
// y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)
// y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
xpl_ease_float xpl_ease_back_in_out(xpl_ease_float p)
{
	if(p < 0.5)
	{
		xpl_ease_float f = 2 * p;
		return 0.5 * (f * f * f - f * sin(f * M_PI));
	}
	else
	{
		xpl_ease_float f = (1 - (2*p - 1));
		return 0.5 * (1 - (f * f * f - f * sin(f * M_PI))) + 0.5;
	}
}

xpl_ease_float xpl_ease_bounce_in(xpl_ease_float p)
{
	return 1 - xpl_ease_bounce_out(1 - p);
}

xpl_ease_float xpl_ease_bounce_out(xpl_ease_float p)
{
	if(p < 4/11.0)
	{
		return (121 * p * p)/16.0;
	}
	else if(p < 8/11.0)
	{
		return (363/40.0 * p * p) - (99/10.0 * p) + 17/5.0;
	}
	else if(p < 9/10.0)
	{
		return (4356/361.0 * p * p) - (35442/1805.0 * p) + 16061/1805.0;
	}
	else
	{
		return (54/5.0 * p * p) - (513/25.0 * p) + 268/25.0;
	}
}

xpl_ease_float xpl_ease_bounce_in_out(xpl_ease_float p)
{
	if(p < 0.5)
	{
		return 0.5 * xpl_ease_bounce_in(p*2);
	}
	else
	{
		return 0.5 * xpl_ease_bounce_out(p * 2 - 1) + 0.5;
	}
}
