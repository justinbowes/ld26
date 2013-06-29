#include <math.h>
#include <wchar.h>
#include <string.h>

#include "uthash.h"

#include "xpl.h"
#include "xpl_gl.h"
#include "xpl_memory.h"
#include "xpl_text_cache.h"
#include "xpl_vao.h"
#include "xpl_bo.h"
#include "xpl_hash.h"
#include "xpl_command_render.h"
#include "xpl_imui.h"
#include "xpl_color.h"

#undef DISABLE_CACHES

static const float PI = 3.14159264;
static const int HASH_RAWPOLY = 1;
static const int HASH_RECT = 2;
static const int HASH_ROUNDED_RECT = 3;
static const int HASH_LINE = 4;
// static const int HASH_FONT = 5;
static const int HASH_TRI = 6;

static struct xpl_text_cache *g_text_cache;

// ---------------------------------------------------------------------

typedef struct _geometry_table_entry {
	int key;
	xpl_vao_t *vao;
	xpl_bo_t *vertex_bo;
	xpl_bo_t *color_bo;
	size_t vertices;
    
	UT_hash_handle hh;
    
} _geometry_table_entry_t;

static _geometry_table_entry_t *geometry_table_entry_new(int key) {
    
	_geometry_table_entry_t *entry = xpl_alloc_type(_geometry_table_entry_t);
#    ifndef DISABLE_CACHES
	LOG_DEBUG("New geometry table entry: %d", key);
#    endif
	entry->key = key;
	entry->vao = xpl_vao_new();
	entry->vertex_bo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	entry->color_bo = xpl_bo_new(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    
	return entry;
}

static void geometry_table_entry_destroy(_geometry_table_entry_t **ppbuffer) {
	assert(ppbuffer);
    
	_geometry_table_entry_t *entry = *ppbuffer;
	assert(entry);
    
	if (entry->vao)
		xpl_vao_destroy(&entry->vao);
	if (entry->vertex_bo)
		xpl_bo_destroy(&entry->vertex_bo);
	if (entry->color_bo)
		xpl_bo_destroy(&entry->color_bo);
    
	xpl_free(entry);
    
	*ppbuffer = NULL;
}

// ---------------------------------------------------------------------

typedef struct _geometry_table {
	_geometry_table_entry_t *entries;	// uthash
    
} _geometry_table_t;

_geometry_table_t * geometry_table_new() {
	_geometry_table_t *table = xpl_alloc_type(_geometry_table_t);
	table->entries = NULL;
    
	return table;
}

static void geometry_table_destroy(_geometry_table_t **pptable) {
	assert(pptable);
    
	_geometry_table_t *table = *pptable;
	assert(table);
    
	_geometry_table_entry_t *buf, *tmp;
    
	HASH_ITER(hh, table->entries, buf, tmp)
	{
		HASH_DEL(table->entries, buf);
		geometry_table_entry_destroy(&buf);
	}
    
	xpl_free(table);
    
	*pptable = NULL;
    
}
// ---------------------------------------------------------------------

typedef struct _geometry_cache {
	_geometry_table_t *last_frame;	// uthash
	_geometry_table_t *this_frame;	// uthash
    
} _geometry_cache_t;

static _geometry_cache_t *geometry_cache_new() {
	_geometry_cache_t *cache = xpl_alloc_type(_geometry_cache_t);
	cache->last_frame = geometry_table_new();
	cache->this_frame = geometry_table_new();
	return cache;
}

static void geometry_cache_destroy(_geometry_cache_t **ppcache) {
	assert(ppcache);
    
	_geometry_cache_t *cache = *ppcache;
	assert(cache);
    
	if (cache->last_frame)
		geometry_table_destroy(&cache->last_frame);
	if (cache->this_frame)
		geometry_table_destroy(&cache->this_frame);
    
	xpl_free(cache);
    
	*ppcache = NULL;
}

// ---------------------------------------------------------------------
static _geometry_cache_t *g_geom_cache;

static _geometry_table_entry_t * geometry_cache_get(int hashkey) {
#    ifdef DISABLE_CACHES
	return NULL;
#    endif
	_geometry_table_entry_t *entry;
	HASH_FIND_INT(g_geom_cache->this_frame->entries, &hashkey, entry);
	if (entry != NULL ) {
		LOG_WARN("Duplicate fetch for %d this frame", hashkey);
		return entry;
	}
    
	HASH_FIND_INT(g_geom_cache->last_frame->entries, &hashkey, entry);
	if (entry == NULL )
		return NULL ;
    
	// transfer to this frame table
	HASH_DEL(g_geom_cache->last_frame->entries, entry);
	HASH_ADD_INT(g_geom_cache->this_frame->entries, key, entry);
    
	return entry;
}

static void geometry_cache(_geometry_table_entry_t *geom) {
	assert(geom);
	HASH_ADD_INT(g_geom_cache->this_frame->entries, key, geom);
}

static void geometry_cache_advance_frame(void) {
	_geometry_table_entry_t *entry, *tmp;
    
	HASH_ITER(hh, g_geom_cache->last_frame->entries, entry, tmp)
	{
		HASH_DEL(g_geom_cache->last_frame->entries, entry);
		geometry_table_entry_destroy(&entry);
	}
	_geometry_table_t *swap = g_geom_cache->last_frame;
	g_geom_cache->last_frame = g_geom_cache->this_frame;
	g_geom_cache->this_frame = swap;
}

// ---------------------------------------------------------------------
static xpl_shader_t *g_ui_shader = NULL;
static int g_ui_state_set = 0;

XPLINLINE void imui_render_state_set(void) {
	if (g_ui_state_set)
		return;
    
	// LOG_DEBUG("Setting UI render state");
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glUseProgram(g_ui_shader->id);
    
    GL_DEBUG();
    
	g_ui_state_set = TRUE;
}

XPLINLINE void imui_render_state_clear(void) {
	// LOG_DEBUG("Clearing UI render state");
	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    
	if (!g_ui_state_set)
		return;
    
	glUseProgram(0);
    
    GL_DEBUG();
    
	g_ui_state_set = FALSE;
}

XPLINLINE void draw_geom(xmat4 *mvp, _geometry_table_entry_t *geom) {
	imui_render_state_set();
	glUniformMatrix4fv(xpl_shader_get_uniform(g_ui_shader, "mvp"), 1, GL_FALSE, &mvp->data[0]);
	xpl_vao_program_draw_arrays(geom->vao, g_ui_shader, GL_TRIANGLES, 0,
                                (GLsizei) geom->vertices);
}

// ---------------------------------------------------------------------

#define TEMP_COORD_COUNT 1000
static xvec2 s_temp_normals[TEMP_COORD_COUNT];
static xvec2 s_temp_coords[TEMP_COORD_COUNT];

inline uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (r) | (g << 8) | (b << 16) | (a << 24);
}

static _geometry_table_entry_t *geometry_cache_polygon(int key, xvec2 *coords,
                                                       size_t num_coords, float blend_r, uint32_t color) {
	_geometry_table_entry_t *geom = geometry_table_entry_new(key);
    
	size_t last_coord = num_coords - 1;
    
	// j is a trailing iterator
	for (size_t i = 0, j = last_coord; i < num_coords; j = i++) {
		const xvec2 *v0 = &coords[j];
		const xvec2 *v1 = &coords[i];
		xvec2 diff = xvec2_set(v1->x - v0->x, v1->y - v0->y);
		float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
		if (dist > 0) {
			// normalize
			dist = 1.0f / dist;
			diff.x *= dist;
			diff.y *= dist;
		}
		s_temp_normals[j] = xvec2_set(diff.y, -diff.x);
	}
    
	// j trails
	for (size_t i = 0, j = last_coord; i < num_coords; j = i++) {
		const xvec2 *dlx0 = &s_temp_normals[j];
		const xvec2 *dlx1 = &s_temp_normals[i];
		xvec2 dm = xvec2_set(dlx0->x + dlx1->x * 0.5f,
                             dlx0->y + dlx1->y * 0.5f);
		float dmr2 = dm.x * dm.x + dm.y * dm.y;
		if (dmr2 > 0.000001f) {
			float scale = 1.0f / dmr2;
			if (scale > 10.0f)
				scale = 10.0f;
			dm = xvec2_set(dm.x * scale, dm.y * scale);
		}
		s_temp_coords[i] = xvec2_set(coords[i].x + dm.x * blend_r,
                                     coords[i].y + dm.y * blend_r);
	}
    
	uint32_t color_transparent = RGBA(color & 0xff,
                                      (color >> 8) & 0xff,
                                      (color >> 16) & 0xff,
                                      0);
    
	size_t vertices = num_coords * 6 + (num_coords - 2) * 3;
	xvec2 *v = xpl_alloc(vertices * sizeof (xvec2));
	uint32_t *c = xpl_alloc(vertices * sizeof (uint32_t));
	int k = 0;
    
	// edge
	for (size_t i = 0, j = last_coord; i < num_coords; j = i++) {
		v[k] = coords[i];
		c[k++] = color;
        
		v[k] = coords[j];
		c[k++] = color;
        
		v[k] = s_temp_coords[j];
		c[k++] = color_transparent;
        
		// t2
        
		v[k] = s_temp_coords[j];
		c[k++] = color_transparent;
        
		v[k] = s_temp_coords[i];
		c[k++] = color_transparent;
        
		v[k] = coords[i];
		c[k++] = color;
	}
    
	// interior
	for (size_t i = 2; i < num_coords; ++i) {
		v[k] = coords[0];
		c[k++] = color;
        
		v[k] = coords[i - 1];
		c[k++] = color;
        
		v[k] = coords[i];
		c[k++] = color;
	}
	assert(vertices == k);
	geom->vertices = k;
    
	// Create BOs from data
	xpl_bo_append(geom->vertex_bo, &v[0], k * sizeof(xvec2));
	xpl_bo_commit(geom->vertex_bo);
	xpl_bo_append(geom->color_bo, &c[0], k * sizeof(uint32_t));
	xpl_bo_commit(geom->color_bo);
    
	xpl_free(v);
	xpl_free(c);
    
	xpl_vao_define_vertex_attrib(geom->vao, "position", geom->vertex_bo, 2,
                                 GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	xpl_vao_define_vertex_attrib(geom->vao, "color", geom->color_bo, 4,
                                 GL_UNSIGNED_BYTE, GL_TRUE, sizeof(uint32_t), 0);
    
	geometry_cache(geom);
	return geom;
}

static void draw_polygon(xmat4 *mvp, xvec2 *coords, size_t num_coords,
                         float blend_r, uint32_t color) {
	// cache pass
	int hash = XPL_HASH_INIT;
	hash = xpl_hashi(HASH_RAWPOLY, hash);
	for (size_t i = 0; i < num_coords; ++i) {
		hash = xpl_hashf(coords[i].x, hash);
		hash = xpl_hashf(coords[i].y, hash);
	}
	hash = xpl_hashi((int)num_coords, hash);
	hash = xpl_hashf(blend_r, hash);
	hash = xpl_hashi(color, hash);
    
	_geometry_table_entry_t *geom = geometry_cache_get(hash);
	if (!geom) {
		geom = geometry_cache_polygon(hash, coords, num_coords, blend_r, color);
	}
	draw_geom(mvp, geom);
}

static void draw_rect(xmat4 *mvp, xrect rect, float blend_r, uint32_t color) {
	// cache pass
	int hash = XPL_HASH_INIT;
	hash = xpl_hashi(HASH_RECT, hash);
	hash = xpl_hashf(rect.origin.x, hash);
	hash = xpl_hashf(rect.origin.y, hash);
	hash = xpl_hashf(rect.size.width, hash);
	hash = xpl_hashf(rect.size.height, hash);
	hash = xpl_hashf(blend_r, hash);
	hash = xpl_hashi(color, hash);
    
	_geometry_table_entry_t *geom = geometry_cache_get(hash);
	if (!geom) {
		xvec2 verts[4];
		verts[0] = xvec2_set(rect.x + 0.5f, rect.y + 0.5f);
		verts[1] = xvec2_set(rect.x + rect.width - 0.5f, rect.y + 0.5f);
		verts[2] = xvec2_set(rect.x + rect.width - 0.5f,
                             rect.y + rect.height - 0.5f);
		verts[3] = xvec2_set(rect.x + 0.5f, rect.y + rect.height - 0.5f);
		geom = geometry_cache_polygon(hash, verts, 4, blend_r, color);
	}
	draw_geom(mvp, geom);
}

static const int CIRCLE_VERTS = 8 * 4;

static xvec2 *init_circle_verts() {
	xvec2 *verts = xpl_alloc(CIRCLE_VERTS * sizeof (xvec2));
	for (size_t i = 0; i < CIRCLE_VERTS; ++i) {
		float a = ((float) i) / ((float) CIRCLE_VERTS) * PI * 2;
		verts[i] = xvec2_set(cosf(a), sinf(a));
	}
	return verts;
    
}

static xvec2 *g_circle_verts = NULL;

static void draw_rounded_rect(xmat4 *mvp, xrect rect, float corner_r,
                              float blend_r, uint32_t color) {
	// cache
	int hash = XPL_HASH_INIT;
	hash = xpl_hashi(HASH_ROUNDED_RECT, hash);
	hash = xpl_hashf(rect.origin.x, hash);
	hash = xpl_hashf(rect.origin.y, hash);
	hash = xpl_hashf(rect.size.width, hash);
	hash = xpl_hashf(rect.size.height, hash);
	hash = xpl_hashf(corner_r, hash);
	hash = xpl_hashf(blend_r, hash);
	hash = xpl_hashi(color, hash);
    
	_geometry_table_entry_t *geom = geometry_cache_get(hash);
	if (!geom) {
		const unsigned n = CIRCLE_VERTS / 4; // quarter circle
		xvec2 verts[(n + 1) * 4];
		const xvec2 *cverts = g_circle_verts;
		int k = 0;
        
		// top right?
		for (size_t i = 0; i <= n; ++i) {
			verts[k] = xvec2_set(
                                 rect.origin.x + rect.size.width - corner_r + cverts[i].x * corner_r,
                                 rect.origin.y + rect.size.height - corner_r + cverts[i].y * corner_r);
			k++;
		}
        
		for (size_t i = n; i <= 2 * n; ++i) {
			verts[k] = xvec2_set(rect.origin.x + corner_r + cverts[i].x * corner_r,
                                 rect.origin.y + rect.size.height - corner_r + cverts[i].y * corner_r);
			k++;
		}
        
		for (size_t i = 2 * n; i <= 3 * n; ++i) {
			verts[k] = xvec2_set(rect.origin.x + corner_r + cverts[i].x * corner_r,
                                 rect.origin.y + corner_r + cverts[i].y * corner_r);
			k++;
		}
        
		for (size_t i = 3 * n; i < 4 * n; ++i) {
			verts[k] = xvec2_set(
                                 rect.origin.x + rect.size.width - corner_r + cverts[i].x * corner_r,
                                 rect.origin.y + corner_r + cverts[i].y * corner_r);
			k++;
		}
		verts[k++] = xvec2_set(
                               rect.origin.x + rect.size.width - corner_r + cverts[0].x * corner_r,
                               rect.origin.y + corner_r + cverts[0].y * corner_r);
		assert(k == (n + 1) * 4);
        
		geom = geometry_cache_polygon(hash, verts, k, blend_r, color);
	}
	draw_geom(mvp, geom);
}

static void draw_line(xmat4 *mvp, xvec4 line, float width, float blend_r,
                      uint32_t color) {
	// cache
	int hash = XPL_HASH_INIT;
	hash = xpl_hashi(HASH_LINE, hash);
	hash = xpl_hashf(line.origin.x, hash);
	hash = xpl_hashf(line.origin.y, hash);
	hash = xpl_hashf(line.dest.x, hash);
	hash = xpl_hashf(line.dest.y, hash);
	hash = xpl_hashf(width, hash);
	hash = xpl_hashf(blend_r, hash);
	hash = xpl_hashi(color, hash);
    
	_geometry_table_entry_t *geom = geometry_cache_get(hash);
	if (!geom) {
        
		xvec2 vec = xvec2_set(line.dest.x - line.origin.x, line.dest.y - line.origin.y);
		float d = sqrtf(vec.x * vec.x + vec.y * vec.y);
		if (d > 0.00001f) {
			d = 1.0f / d;
			vec.x *= d;
			vec.y *= d;
		}
        
		xvec2 normal = xvec2_set(-vec.y, vec.x);
		xvec2 verts[4];
        
		width -= blend_r;
		width *= 0.5f;
		if (width < 0.01f)
			width = 0.01f;
        
		vec.x *= width;
		vec.y *= width;
		normal.x *= width;
		normal.y *= width;
        
		verts[0] = xvec2_set(line.origin.x - vec.x - normal.x,
							 line.origin.y - vec.y - normal.y);
		verts[1] = xvec2_set(line.origin.x - vec.x + normal.x,
							 line.origin.y - vec.y + normal.y);
		verts[2] = xvec2_set(line.dest.x + vec.x + normal.x,
							 line.dest.y + vec.y + normal.y);
		verts[3] = xvec2_set(line.dest.x + vec.x - normal.x,
							 line.dest.y + vec.y - normal.y);
        
		geom = geometry_cache_polygon(hash, verts, 4, blend_r, color);
	}
	draw_geom(mvp, geom);
    
}

static void draw_triangle(xmat4 *mvp, xvec2 *verts, float blend_r,
                          uint32_t color) {
	int hash = XPL_HASH_INIT;
	hash = xpl_hashi(HASH_TRI, hash);
	for (size_t i = 0; i < 3; ++i) {
		hash = xpl_hashf(verts[i].x, hash);
		hash = xpl_hashf(verts[i].y, hash);
	}
	_geometry_table_entry_t *geom = geometry_cache_get(hash);
	if (!geom) {
		geom = geometry_cache_polygon(hash, verts, 3, blend_r, color);
	}
	draw_geom(mvp, geom);
}

static float text_get_length(xpl_font_t *font, const char *text,
                             size_t position) {
	assert(font);
    
	float len = 0;
	size_t charno = 0;
	if (position == -1)
		position = strlen(text);
	while (*text && (charno <= position)) {
		char c = (char) *text;
		if (c == '\t') {
			len += text_get_length(font, "    ", -1);
			continue;
		}
        
		xpl_glyph_t *glyph = xpl_font_get_glyph(font, c);
		len += glyph->advance_x;
        
		++text;
		++charno;
	}
	return len;
}

xpl_font_t *get_font_for_ref_markup(xpl_markup_t *ref_markup,
                                    const char *text) {
	xpl_markup_t *markup = xpl_markup_new();
	xpl_markup_set(markup, ref_markup->family, ref_markup->size,
                   ref_markup->bold, ref_markup->italic, ref_markup->foreground_color,
                   ref_markup->background_color);
    
	xpl_cached_text_t *entry = xpl_text_cache_get(g_text_cache, markup, text); // Likely cached.
    
	xpl_font_t *result = xpl_font_manager_get_from_markup(
                                                          entry->buffer->font_manager, markup);
	xpl_free(markup);
	return result;
}

xpl_font_position_t xpl_imui_text_get_position(xpl_markup_t *ref_markup,
                                               const char *text, const float offset) {
	xpl_font_position_t result;
	result.left_offset = 0.0f;
	result.right_offset = 0.0f;
	result.char_offset = 0;
    
	// Markup isn't trustworthy; fill it out
	xpl_font_t *font = get_font_for_ref_markup(ref_markup, text);
    
	const size_t text_length = strlen(text);
	for (size_t i = 0; i <= text_length; ++i) {
		float right_edge = 0.0f;
		if (i == text_length) {
			right_edge += text_get_length(font, " ", -1);
		} else {
			right_edge = text_get_length(font, text, i);
		}
		result.left_offset = result.right_offset;
		result.right_offset = right_edge;
		result.char_offset = (int)i;
		if (right_edge >= offset) {
			// As of this character, we've reached the supplied offset.
			break;
		}
	}
    
	return result;
}

xpl_font_position_t xpl_imui_text_offsets_for_position(xpl_markup_t *ref_markup,
                                                       const char *text, const int pos) {
	xpl_font_position_t result;
	result.left_offset = 0.0f;
	result.right_offset = 0.0f;
	result.char_offset = 0;
    
	// Markup isn't trustworthy; fill it out
	xpl_font_t *font = get_font_for_ref_markup(ref_markup, text);
    
	const size_t text_length = strlen(text);
	for (size_t i = 0; i <= xmin(pos, text_length); ++i) {
		float right_edge = text_get_length(font, text, i);
		result.left_offset = result.right_offset;
		result.right_offset = right_edge;
		result.char_offset = (int)i;
	}
    
	return result;
}

static void draw_text(xmat4 *mvp, xvec2 position, xpl_markup_t *markup,
                      const char *text, int align) {
	assert(markup);
	if (!text)
		return;
    
	imui_render_state_clear();
    
    xpl_cached_text_t *text_entry = xpl_text_cache_get(g_text_cache, markup, text);
    
	xpl_font_t *font = text_entry->managed_font;
    
	switch (align) {
        case XPL_IMUI_ALIGN_LEFT:
            break;
        case XPL_IMUI_ALIGN_CENTER:
        case XPL_IMUI_ALIGN_RIGHT: {
            position.x -= text_get_length(font, text, -1) * (align == XPL_IMUI_ALIGN_RIGHT ? 1.0f : 0.5f);
            break;
        }
	}
	xivec2 draw_position = xivec2_set(position.x, position.y);
	draw_position.y += markup->size;
    
	xvec3 translate = xvec3_extendi(draw_position, 0.0f);
	xmat4 transform;
	xmat4_translate(mvp, &translate, &transform);
    
	xpl_text_buffer_commit(text_entry->buffer);
	xpl_text_buffer_render(text_entry->buffer, transform.data);
    
}

static void imui_render_advance_frame() {
	xpl_text_cache_advance_frame(g_text_cache);
	geometry_cache_advance_frame();
}

void xpl_imui_render_draw(xivec2 *screen, xpl_render_cmd_t *commands,
                          size_t command_length, const float scale, const float blend_amount) {
	glDisable(GL_SCISSOR_TEST);
    
	xmat4 mat_vp, *vp = &mat_vp;
	xmat4_ortho(0, (float) screen->x, 0, (float) screen->y, -1.0f, 1.0f, vp);
    
	for (size_t i = 0; i < command_length; ++i) {
        
		const xpl_render_cmd_t *cmd = &commands[i];
		xmat4 mat_mvp, *mvp = &mat_mvp;
		xmat4_multiply(vp, &cmd->matrix, mvp);
        
		switch (cmd->type) {
                
            case XPL_RENDER_CMD_RECT:
                if (cmd->shape.radius == 0.0f) {
                    draw_rect(mvp, xrect_scale(cmd->shape.area, scale),
                              blend_amount, cmd->shape.color);
                } else {
                    draw_rounded_rect(mvp, xrect_scale(cmd->shape.area, scale),
                                      cmd->shape.radius * scale, blend_amount,
                                      cmd->shape.color);
                }
                break;
                
            case XPL_RENDER_CMD_LINE:
                draw_line(mvp, xvec4_scale(cmd->shape.line, scale),
                          cmd->shape.radius * scale, 1.0f, cmd->shape.color);
                break;
                
            case XPL_RENDER_CMD_TRIANGLE: {
                xvec2 verts[3];
                if (cmd->flags == 1) {
                    verts[0] = xvec2_set(cmd->shape.area.x * scale + 0.5f,
                                         cmd->shape.area.y * scale + 0.5f);
                    
                    verts[1] = xvec2_set(cmd->shape.area.x * scale + 0.5f +
                                         cmd->shape.area.width * scale - 1.0f,
                                         cmd->shape.area.y * scale + 0.5f +
                                         cmd->shape.area.height * scale * 0.5f - 0.5f);
                    
                    verts[2] = xvec2_set(cmd->shape.area.x * scale + 0.5f,
                                         cmd->shape.area.y * scale + 0.5f +
                                         cmd->shape.area.height * scale - 1.0f);
                } else if (cmd->flags == 2) {
                    verts[0] = xvec2_set(cmd->shape.area.x * scale + 0.5f,
                                         cmd->shape.area.y * scale + 0.5f +
                                         cmd->shape.area.height * scale - 1.0f);
                    
                    verts[1] = xvec2_set(cmd->shape.area.x * scale + 0.5f +
                                         cmd->shape.area.width * scale * 0.5f - 0.5f,
                                         cmd->shape.area.y * scale + 0.5f);
                    
                    verts[2] = xvec2_set(cmd->shape.area.x * scale + 0.5f +
                                         cmd->shape.area.width * scale - 1.0f,
                                         cmd->shape.area.y * scale + 0.5f +
                                         cmd->shape.area.height * scale - 1.0f);
                }
                draw_triangle(mvp, verts, 1.0f, cmd->shape.color);
                break;
            }
            case XPL_RENDER_CMD_POLYGON: {
                for (size_t j = 0; j < cmd->polygon.points_len; ++j) {
                    cmd->polygon.points[j].x *= scale;
                    cmd->polygon.points[j].y *= scale;
                }
                draw_polygon(mvp, cmd->polygon.points, cmd->polygon.points_len,
                             blend_amount, cmd->polygon.color);
                break;
            }
            case XPL_RENDER_CMD_TEXT: {
                xpl_markup_t *markup = cmd->text.markup;
                draw_text(mvp, xvec2_scale(cmd->text.position, scale), markup,
                          cmd->text.text, cmd->text.align);
                break;
            }
            case XPL_RENDER_CMD_SCISSOR:
                if (cmd->flags) {
                    glEnable(GL_SCISSOR_TEST);
                    glScissor(scale * cmd->shape.area.x,
                              scale * cmd->shape.area.y,
                              scale * cmd->shape.area.width,
                              scale * cmd->shape.area.height);
                } else {
                    glDisable(GL_SCISSOR_TEST);
                }
                break;
            default:
                LOG_ERROR("Unimplemented command: %d", cmd->type);
                break;
		}
	}
	imui_render_state_clear();
	imui_render_advance_frame();
}

static int g_initialized = FALSE;

void xpl_imui_render_init() {
	if (g_initialized)
		return;
    
	g_circle_verts = init_circle_verts();
    
	g_text_cache = xpl_text_cache_new(256);
    
	g_circle_verts = init_circle_verts();
	g_geom_cache = geometry_cache_new();
    
	g_ui_shader = xpl_shader_get("IMUI");
	if (!g_ui_shader->linked) {
		xpl_shader_add(g_ui_shader, GL_VERTEX_SHADER, "IMUI.Vertex");
		xpl_shader_add(g_ui_shader, GL_FRAGMENT_SHADER, "IMUI.Fragment");
		xpl_shader_link(g_ui_shader);
	}
	g_initialized = TRUE;
    
}

void xpl_imui_render_destroy() {
	if (!g_initialized)
		return;
    
    xpl_free(g_circle_verts);
    
	xpl_text_cache_destroy(&g_text_cache);
	geometry_cache_destroy(&g_geom_cache);
    
	xpl_shader_release(&g_ui_shader);
    
	g_initialized = FALSE;
    
}

void xpl_render_cmd_content_reset(xpl_render_cmd_t *cmd) {
	switch (cmd->type) {
        case XPL_RENDER_CMD_TEXT:
            xpl_free(cmd->text.markup);
            free(cmd->text.text); // strdup allocated
            break;
        case XPL_RENDER_CMD_POLYGON:
            xpl_free(cmd->polygon.points);
            // xpl_allocated
            cmd->polygon.points_len = 0;
            break;
	}
	xmat4_identity(&cmd->matrix);
	cmd->type = XPL_RENDER_CMD_INVALID;
}
