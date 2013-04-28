/* 
 * File:   xpl_math.h
 * Author: Justin
 *
 * Created on December 13, 2012, 9:34 AM
 */

#ifndef XPL_MATH_H
#define	XPL_MATH_H

#include "xpl.h"

#ifndef M_E
#define M_E		2.7182818284590452354
#endif
    
#ifndef M_LOG2E
#define M_LOG2E		1.4426950408889634074
#endif
    
#ifndef M_LOG10E
#define M_LOG10E	0.43429448190325182765
#endif
    
#ifndef M_LN2
#define M_LN2		0.69314718055994530942
#endif

#ifndef M_LN10
#define M_LN10		2.30258509299404568402
#endif

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#ifndef M_2PI
#define M_2PI		6.28318530717959
#endif

#ifndef M_3PI
#define M_3PI       9.42477796076938
#endif

#ifndef M_PI_2
#define M_PI_2		1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4		0.78539816339744830962
#endif
    
#ifndef M_1_PI
#define M_1_PI		0.31830988618379067154
#endif

#ifndef M_2_PI
#define M_2_PI		0.63661977236758134308
#endif
    
#ifndef M_2_SQRTPI
#define M_2_SQRTPI	1.12837916709551257390
#endif
    
#ifndef M_SQRT2
#define M_SQRT2		1.41421356237309504880
#endif
    
#ifndef M_SQRT1_2
#define M_SQRT1_2	0.70710678118654752440
#endif
    
XPLINLINE float xmix(float a, float b, float r) {
    return (1.0f - r) * a + r * b;
}

XPLINLINE float xpl_angle_difference_degreesf(float a, float b) {
    float d = a - b;
    while (d >= 360.0f) d -= 360.0f;
    d += 540.0f;
    while (d >= 360.0f) d -= 360.0f;
    d -= 180.0f;
    return d;
}

XPLINLINE float xpl_angle_difference_radiansf(float a, float b) {
    float d = a - b;
    while (d >= (float)M_2PI) d -= (float)M_2PI;
    d += M_3PI;
    while (d >= (float)M_2PI) d -= (float)M_2PI;
    d -= M_PI;
    return d;
}

XPLINLINE float xpl_angle_in_radians(float degrees) {
    return degrees * ((float)M_PI) / 180.0f;
}

XPLINLINE float xpl_angle_in_degrees(float radians) {
    return radians * 180.0f / ((float)M_PI);
}

XPLINLINE unsigned int xpl_next_power_of_2(unsigned int val) {
	unsigned count = 0;
	
	/* First n in the below condition is for the case where n is 0*/
	if (val & !(val & (val - 1)))
		return val;
	
	while(val != 0)	{
		val  >>= 1;
		count += 1;
	}
	
	return 1 << count;
}

#define xpl_signum(x) ((x > 0) - (x < 0))


#endif	/* XPL_MATH_H */

