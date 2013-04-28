/*
 * xpl_context.c
 *
 *  Created on: 2012-12-14
 *      Author: Justin
 */

#include <assert.h>

#include "xpl_mat4.h"
#include "xpl_context.h"
#include "xpl_engine_info.h"

xpl_context_t *xpl_context_new(xpl_app_t *app, const xpl_context_def_t *context_def) {
	xpl_context_t *context = xpl_calloc_type(xpl_context_t);
    
	context->app = app;
	xmat4_identity(&context->parent_matrix);
	context->size = app->execution_info->screen_size;

    context->functions = *context_def;

	return context;
}

void xpl_context_destroy(xpl_context_t **ppcontext) {
	assert(ppcontext);
	xpl_context_t *context = *ppcontext;
	assert(context);

	xpl_zero_struct(context);
	xpl_free(context);
	*ppcontext = NULL;
}
