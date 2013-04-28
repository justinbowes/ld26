//
//  xpl_effect.h
//  p1
//
//  Created by Justin Bowes on 2013-03-04.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_effect_h
#define p1_xpl_effect_h

#include <stdbool.h>

#include "xpl_effect_buffer.h"

struct xpl_effect_buffer;

/**
 * A callback made available to effects that draws a quad. Untransformed, this
 * will cover clip space at z=0.
 */
typedef void(*xpl_effect_render_callback)(struct xpl_effect_buffer *effect_buffer, xpl_shader_t *shader);

/**
 * A function called immediately when the effect is added to the effect chain.
 * If this function returns true, the effect chain should flip buffers prior to calling this
 * input -- it copies instead of overdrawing.
 */
typedef bool(*xpl_effect_init_func)(struct xpl_effect_buffer *effect_buffer, void *data);

/**
 * A function called immediately when the effect is removed from the effect chain,
 * or when the effect buffer is being destroyed.
 */
typedef void(*xpl_effect_destroy_func)(struct xpl_effect_buffer *effect_buffer, void *data);

/**
 * The effect function to call. The GL state for this effect assumes pixel
 * processing if init() returned true. See xpl_effect_buffer_render_effects.
 */
typedef bool(*xpl_effect_render_func)(struct xpl_effect_buffer *effect_buffer, double time, xpl_effect_render_callback render_callback, void *data);


typedef struct xpl_effect_def {
    xpl_effect_init_func init;
    xpl_effect_render_func effect;
    xpl_effect_destroy_func destroy;
} xpl_effect_def_t;

#endif
