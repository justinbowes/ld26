/*
 * xpl_context.h
 *
 *  Created on: 2012-12-14
 *      Author: Justin
 */

#ifndef XPL_CONTEXT_H_
#define XPL_CONTEXT_H_

#include "xpl.h"
#include "xpl_app.h"

struct xpl_context;

typedef void *(*xpl_context_init_func)(struct xpl_context *context);
typedef void (*xpl_context_engine_func)(struct xpl_context *context, double time, void *data);
typedef void (*xpl_context_render_func)(struct xpl_context *context, double time, void *data);
typedef void (*xpl_context_destroy_func)(struct xpl_context *context, void *data);
typedef struct xpl_context *(*xpl_context_handoff_func)(struct xpl_context *context, void *data);

typedef struct xpl_context_def {
	xpl_context_init_func init;
	xpl_context_engine_func engine;
	xpl_context_render_func render;
	xpl_context_destroy_func destroy;
	xpl_context_handoff_func handoff;
} xpl_context_def_t;

typedef struct xpl_context {
	xpl_app_t *app;

	xivec2 size;
	xmat4 parent_matrix;
    
    xpl_context_def_t functions;
    void *data;

} xpl_context_t;

xpl_context_t *xpl_context_new(xpl_app_t *app, const xpl_context_def_t *context_def);
void xpl_context_destroy(xpl_context_t **ppcontext);

#endif /* XPL_CONTEXT_H_ */
