/*
 * xpl_color.h
 *
 *  Created on: 2013-03-06
 *      Author: Justin
 */

#ifndef XPL_COLOR_H_
#define XPL_COLOR_H_

#define RGBA(r,g,b,a)	(((r) & 0xff) | (((g) & 0xff) << 8) | (((b) & 0xff) << 16) | (((a) & 0xff) << 24))
#define RGBA_I(v)		(((int)((v).x * 255)) | ((int)((v).y * 255) << 8) | ((int)((v).z * 255) << 16) | ((int)((v).w * 255) << 24))
#define RGBA_V(r,g,b,a) {{ (r) / 255.0f, (g) / 255.0f, (b) / 255.0f, (a) / 255.0f }}
#define RGBA_F(x)		RGBA_V((x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, ((x) >> 24) & 0xff)

#endif /* XPL_COLOR_H_ */
