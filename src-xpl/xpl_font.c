//
//  xpl_font.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-23.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include "xpl_gl.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <wchar.h>

#include "uthash.h"

#include "xpl.h"
#include "xpl_vec.h"
#include "xpl_memory.h"
#include "xpl_font.h"
#include "xpl_platform.h"
#include "xpl_hash.h" // for windows max
#define LOG_FT_ERROR(error) LOG_ERROR("FT_Error (0x%02x)", error)

#define HRES 64
#define DPI 72

typedef struct xpl_kerning {
	wchar_t			charcode;
	float			kerning;
    
	UT_hash_handle	hh;
    
} xpl_kerning_t;

static int resource_name_for_font_name(char *resource_name, const char *font_name, const char *extension, size_t length) {
	char *font_path = (char *)xpl_alloc(length * sizeof (char));
	sprintf(font_path, "fonts/%s.%s", font_name, extension);
    
	int result = xpl_resolve_resource(resource_name, font_path, length);
	xpl_free(font_path);
	return result;
}

static int font_load_face(FT_Library *library, const char *filename, const float size,
                          FT_Face *face) {
	assert(library);
	assert(filename);
	assert(size);
    
	FT_Matrix matrix = { (int) ((1.0 / HRES) * 0x10000L),
        (int) (0.0 * 0x10000L),
        (int) (0.0 * 0x10000L),
        (int) (1.0 * 0x10000L) };
    
	FT_Error error;
	error = FT_Init_FreeType(library);
	if (error) {
		LOG_FT_ERROR(error);
		return FALSE;
	}
    
	error = FT_New_Face(*library, filename, 0, face);
	if (error) {
		LOG_FT_ERROR(error);
		FT_Done_FreeType(*library);
		return FALSE;
	}
    
	error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
	if (error) {
		FT_Done_Face(*face);
		FT_Done_FreeType(*library);
		return FALSE;
	}
    
	error = FT_Set_Char_Size(*face, (int) (HRES * size), 0, DPI * HRES, DPI);
	if (error) {
		FT_Done_Face(*face);
		FT_Done_FreeType(*library);
		return FALSE;
	};
    
	FT_Set_Transform(*face, &matrix, NULL );
    
	return TRUE;
}

static xpl_kerning_t *kerning_new(wchar_t wc, float kerning) {
	xpl_kerning_t *self = xpl_alloc_type(xpl_kerning_t);
	self->charcode = wc;
	self->kerning = kerning;
	return self;
}

static void kerning_destroy(xpl_kerning_t **ppkerning) {
	assert(ppkerning);
	xpl_kerning_t *kerning = *ppkerning;
	assert(kerning);
    
	xpl_free(kerning);
    
	*ppkerning = NULL;
}

static xpl_glyph_t *glyph_new(wchar_t charcode) {
    
	xpl_glyph_t *self = xpl_alloc_type(xpl_glyph_t);
    
	self->charcode = charcode;
	self->width = 0.0f;
	self->height = 0.0f;
	self->offset_x = 0;
	self->offset_y = 0;
	self->advance_x = 0.0f;
	self->advance_y = 0.0f;
	self->tex_s0 = 0.0f;
	self->tex_t0 = 0.0f;
	self->tex_s1 = 0.0f;
	self->tex_t1 = 0.0f;
	self->kerning_table = NULL;
    
	return self;
}

static void glyph_destroy_kerning(xpl_glyph_t *glyph) {
    
	xpl_kerning_t *elem, *tmp;
    
	HASH_ITER(hh, glyph->kerning_table, elem, tmp) {
        
		HASH_DEL(glyph->kerning_table, elem);
		kerning_destroy(&elem);
	}
}

static void glyph_destroy(xpl_glyph_t **ppglyph) {
    
	xpl_glyph_t *glyph = *ppglyph;
	assert(glyph);
    
	glyph_destroy_kerning(glyph);
    
	xpl_free(glyph);
	glyph = NULL;
}

/**
 * Generates any missing kerning pairs for all glyphs loaded for the current font.
 * Kerning does not account for outline.
 * @param self
 */
void font_generate_kerning(xpl_font_t *self) {
	assert(self);
    
	FT_Library library;
	FT_Face face;
	FT_Vector kerning;
    
	if (!font_load_face(&library, self->filename, self->size, &face)) {
        
		LOG_ERROR(
                  "Couldn't generate kerning for %s %f", self->filename, self->size);
		return;
	}
    
	// Check whether kerning is required for each glyph pair
	xpl_glyph_t *trailing_glyph, *t1;
	xpl_glyph_t *leading_glyph, *t2;
	FT_UInt trailing_index, leading_index;
    
	HASH_ITER(hh, self->glyph_ttable, trailing_glyph, t1) {
        
		if (trailing_glyph->charcode == -1)
			continue; // Skip special background glyph at with char code -1
        
		HASH_ITER(hh, self->glyph_ttable, leading_glyph, t2) {
			if (leading_glyph->charcode == -1)
				continue; // Skip special background glyph

			xpl_kerning_t *k;
			HASH_FIND_INT(trailing_glyph->kerning_table, &leading_glyph->charcode, k);
			if (k) continue;
            
			trailing_index = FT_Get_Char_Index(face, trailing_glyph->charcode);
			leading_index = FT_Get_Char_Index(face, leading_glyph->charcode);
			FT_Get_Kerning(face, leading_index, trailing_index, FT_KERNING_UNFITTED,
                           &kerning);
            
			// Add the kerning even if it's zero so we don't keep regenerating it
			// 26.6 encoding and transform matrix means kerning is in units of 64 * 64
			k = kerning_new(leading_glyph->charcode, kerning.x / (float) (HRES * HRES));
			HASH_ADD_INT(trailing_glyph->kerning_table, charcode, k);
		}
	}
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

#ifndef XPL_PLATFORM_IOS
static int lcd_filter_default[] = { 0x10, 0x40, 0x70, 0x40, 0x10 };
/*
 static int lcd_filter_light[]       = { 0x00, 0x55, 0x56, 0x55, 0x00 };
 */
#define FILTER lcd_filter_default
#endif
#define EXTRA_PRECISION 100.0f

xpl_font_t *xpl_font_new(xpl_texture_atlas_t *atlas, const char *name,
                         const float size) {
	assert(name);
	assert(size);
    
	char resource_name[PATH_MAX];
	if (! resource_name_for_font_name(&resource_name[0], name, "ttf", PATH_MAX)) {
		if (! resource_name_for_font_name(&resource_name[0], name, "otf", PATH_MAX)) {
			LOG_ERROR("Couldn't create requested font: %s %f", name, size);
			return NULL;
		}
	}
    
	xpl_font_t *self = xpl_alloc_type(xpl_font_t);
    
	self->glyph_ttable = NULL;
	self->manager_atlas = atlas;
	self->height = 0.0f;
	self->ascender = 0.0f;
	self->descender = 0.0f;
	self->name = strdup(name);
	self->filename = strdup(resource_name);
	self->size = size;
	self->outline_type = xfo_none;
	self->outline_thickness = 0.0f;
	self->hinting = TRUE;
	
#ifndef XPL_PLATFORM_IOS
	self->lcd_filtering = TRUE;
    
	self->lcd_weights[0] = FILTER[0];
	self->lcd_weights[1] = FILTER[1];
	self->lcd_weights[2] = FILTER[2];
	self->lcd_weights[3] = FILTER[3];
	self->lcd_weights[4] = FILTER[4];
#endif
    
	// Try to get high-res font metrics
	FT_Library library;
	FT_Face face;
    
	if (!font_load_face(&library, self->filename, self->size * EXTRA_PRECISION,
                        &face))
		return self;
    
	self->underscore_position = self->size * face->underline_position / (float) (HRES * HRES);
	self->underscore_position = fmaxf(roundf(self->underscore_position), -2.0f);
    
	self->underscore_thickness = self->size * face->underline_thickness / (float) (HRES * HRES);
	self->underscore_thickness = fmaxf(roundf(self->underscore_thickness), 1.0f);
    
	FT_Size_Metrics metrics = face->size->metrics;
    
	// 2^6 = 64
	self->ascender = (metrics.ascender >> 6) / EXTRA_PRECISION;
	self->descender = (metrics.descender >> 6) / EXTRA_PRECISION;
	self->height = (metrics.height >> 6) / EXTRA_PRECISION;
	self->linegap = self->height - self->ascender + self->descender;
    
	FT_Done_Face(face);
	FT_Done_FreeType(library);
    
	// -1 is a special glyph code that ends up as glyph 0 I guess.
	xpl_font_get_glyph(self, -1);
    
	return self;
}

void xpl_font_destroy(xpl_font_t **ppfont) {
    
	xpl_font_t *font = *ppfont;
	assert(font);
    
	free(font->filename); // Allocated using strdup
	free(font->name); // Allocated using strdup
    
	xpl_glyph_t *elem, *tmp;
    
	HASH_ITER(hh, font->glyph_ttable, elem, tmp) {
		HASH_DEL(font->glyph_ttable, elem);
		glyph_destroy(&elem);
	}
    
	// Atlas not owned by font. Who owns it? xpl_font_manager.
	// if (font->atlas) xpl_texture_atlas_destroy(&font->atlas);
    
	xpl_free(font);
	*ppfont = NULL;
}

static unsigned char initial_data[4 * 4 * 3] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

xpl_glyph_t *xpl_font_get_glyph(xpl_font_t *self, wchar_t charcode) {
	assert(self);
	assert(self->filename);
	assert(self->manager_atlas);
    
	int int_charcode = charcode;
    
	const wchar_t neg_1 = (wchar_t)(-1);
	xpl_glyph_t *glyph;
    
	// Has the charcode already been loaded?
	HASH_FIND_INT(self->glyph_ttable, &int_charcode, glyph);
	if (glyph) {
		return glyph;
	}
    
	// No charcode found. Allocate into glyph.
    
	// Charcode -1 is special: overline, underline, strikethrough, background.
	if (charcode == neg_1) {
		size_t width = self->manager_atlas->width;
		size_t height = self->manager_atlas->height;
        
		// Get a 5 x 5 pixel region
		xirect region = xpl_texture_atlas_get_region(self->manager_atlas, 5, 5);
		if (region.x < 0) {
			LOG_ERROR("Texture atlas is full");
			return NULL ;
		}
        
		glyph = glyph_new(neg_1);
        
		// Leave a border by shrinking the region
		region.width = 4;
		region.height = 4;
		xpl_texture_atlas_set_region(self->manager_atlas, region, initial_data, 0);
        
		// This area is arbitrary.
		glyph->tex_s0 = (region.x + 2.0f) / (float) width;
		glyph->tex_t0 = (region.y + 2.0f) / (float) height;
		glyph->tex_s1 = (region.x + 3.0f) / (float) width;
		glyph->tex_t1 = (region.y + 3.0f) / (float) height;
        
		HASH_ADD_INT(self->glyph_ttable, charcode, glyph);
		return glyph;
	}
    
	// Random nonspecial nonloaded glyph.
	wchar_t buffer[2] = { charcode, 0 };
	if (xpl_font_load_glyphs(self, buffer)) {
		// We only asked for one glyph, and the missed list was nonzero.
		return NULL;
	}
    
	// That worked. Look up and return the glyph.
	int int_cc = (int)charcode;
	HASH_FIND_INT(self->glyph_ttable, &int_cc, glyph);
	assert(glyph);
	return glyph;
}

size_t xpl_font_load_glyphs(xpl_font_t *self, const wchar_t *charcodes) {
	assert(self);
	assert(charcodes);
    
	size_t charcount = wcslen(charcodes);
	FT_Bitmap ft_bitmap;
	FT_Error error;
#ifndef XPL_PLATFORM_IOS
	size_t buffer_depth = self->manager_atlas->depth;
#else
	size_t buffer_depth = 1;
#endif
    
	FT_UInt glyph_index;
	xpl_glyph_t *glyph;
    
	size_t missed = 0;
    
	FT_Library library;
	FT_Face face;
	if (!font_load_face(&library, self->filename, self->size, &face)) {
		return charcount; // We missed all of them.
	}
    
	for (size_t i = 0; i < charcount; ++i) {
		// Do we already have this one?
		int int_cc = charcodes[i];
		HASH_FIND_INT(self->glyph_ttable, &int_cc, glyph);
		if (glyph != NULL) continue;
        
		glyph_index = FT_Get_Char_Index(face, int_cc);
        
		FT_Int32 flags = 0;
		flags |= (
                  self->outline_type > xfo_none ?
                  FT_LOAD_NO_BITMAP : FT_LOAD_RENDER);
		flags |= (
                  self->hinting ?
                  FT_LOAD_FORCE_AUTOHINT :
                  (FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT));
        
#ifndef XPL_PLATFORM_IOS
		if (buffer_depth == 3) {
			// Buffoonery: If the texture atlas depth is 3 bytes, we surmise that the
			// user wants subpixel rendering.
			FT_Library_SetLcdFilter(library, FT_LCD_FILTER_DEFAULT);
			flags |= FT_LOAD_TARGET_LCD;
			if (self->lcd_filtering)
				FT_Library_SetLcdFilterWeights(library, self->lcd_weights);
		}
#endif
        
		error = FT_Load_Glyph(face, glyph_index, flags);
		if (error) {
			LOG_FT_ERROR(error);
			FT_Done_Face(face);
			FT_Done_FreeType(library);
			return charcount - i;
		}
        
		xrect bmp_size = {
            { 0 }
		};
		FT_Glyph ft_glyph;
		if (self->outline_type == xfo_none) {
			FT_GlyphSlot slot = face->glyph;
			ft_bitmap = slot->bitmap;
			bmp_size.x = slot->bitmap_left;
			bmp_size.y = slot->bitmap_top;
			bmp_size.width = slot->bitmap.width;
			bmp_size.height = slot->bitmap.rows;
		} else {
			// Outline complexity.
			FT_Stroker stroker; // Yeah, that happened.
            
			error = FT_Stroker_New(library, &stroker);
			if (error) {
				LOG_FT_ERROR(error);
				FT_Done_FreeType(library);
				return charcount - i;
			}
            
			FT_Stroker_Set(stroker, (int) (self->outline_thickness * HRES),
                           FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
            
			error = FT_Get_Glyph(face->glyph, &ft_glyph);
			if (error) {
				LOG_FT_ERROR(error);
				FT_Done_Face(face);
				FT_Stroker_Done(stroker);
				FT_Done_FreeType(library);
				return charcount - i;
			}
            
			switch (self->outline_type) {
                case xfo_line:
                    error = FT_Glyph_Stroke(&ft_glyph, stroker, 1);
                    break;
                    
                case xfo_inner:
                    error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 1, 1);
                    break;
                    
                case xfo_outer:
                    error = FT_Glyph_StrokeBorder(&ft_glyph, stroker, 0, 1);
                    break;
                    
                default:
                    LOG_ERROR("Unrecognized outline type: %d", self->outline_type);
                    assert(0);
                    break;
			}
			if (error) {
				LOG_FT_ERROR(error);
				FT_Done_Face(face);
				FT_Stroker_Done(stroker);
				FT_Done_FreeType(library);
				return charcount - i;
			}
            
#ifndef XPL_PLATFORM_IOS
			int glyph_to_bitmap_flags = (buffer_depth == 3 ? FT_RENDER_MODE_LCD : FT_RENDER_MODE_NORMAL);
#else
			int glyph_to_bitmap_flags = FT_RENDER_MODE_NORMAL;
#endif
			error = FT_Glyph_To_Bitmap(&ft_glyph, glyph_to_bitmap_flags, NULL, TRUE);
			if (error) {
				LOG_FT_ERROR(error);
				FT_Done_Face(face);
				FT_Stroker_Done(stroker);
				FT_Done_FreeType(library);
				return charcount - i;
			}
            
			FT_BitmapGlyph ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
			bmp_size.x = ft_bitmap_glyph->left;
			bmp_size.y = ft_bitmap_glyph->top;
			ft_bitmap = ft_bitmap_glyph->bitmap;
			bmp_size.width = ft_bitmap.width;
			bmp_size.height = ft_bitmap.rows;
            
			FT_Stroker_Done(stroker);
		} // endif stroked
        
		const size_t pad_region = buffer_depth;
		// Separate each glyph by at least one black pixel.
		xirect region = xpl_texture_atlas_get_region(self->manager_atlas,
                                                     (bmp_size.width / buffer_depth) + pad_region,
                                                     bmp_size.height + pad_region);
        
		if (region.x < 0) {
			missed++;
			LOG_WARN("Texture atlas is full on glyph index %ud", (unsigned int)i);
			continue;
		}
        
		region.width -= pad_region;
		region.height -= pad_region;
		xpl_texture_atlas_set_region(self->manager_atlas, region, ft_bitmap.buffer,
                                     ft_bitmap.pitch);
        
        if (self->outline_type) {
			FT_Done_Glyph(ft_glyph);
        }
        
		glyph = glyph_new(charcodes[i]);
		glyph->width = (float) region.width;
		glyph->height = (float) region.height;
		glyph->offset_x = bmp_size.x;
		glyph->offset_y = bmp_size.y;
		glyph->tex_s0 = region.x / (float) self->manager_atlas->width;
		glyph->tex_t0 = region.y / (float) self->manager_atlas->height;
		glyph->tex_s1 = (region.x + glyph->width) / (float) self->manager_atlas->width;
		glyph->tex_t1 = (region.y + glyph->height) / (float) self->manager_atlas->height;
        
		// Reload the glyph with no hinting to get the advance
		error = FT_Load_Glyph(face, glyph_index,
                              FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
		if (error) {
			xpl_free(glyph);
			LOG_FT_ERROR(error);
			FT_Done_FreeType(library);
			return charcount - i;
		}
		FT_GlyphSlot slot = face->glyph;
		glyph->advance_x = slot->advance.x / 64.0f;
		glyph->advance_y = slot->advance.y / 64.0f;
        
		HASH_ADD_INT(self->glyph_ttable, charcode, glyph);
		int verify = (int)charcodes[i];
		xpl_glyph_t *verify_glyph;
		HASH_FIND_INT(self->glyph_ttable, &verify, verify_glyph);
		if (! glyph) {
			LOG_ERROR("Couldn't verify glyph %d using %d", verify_glyph->charcode, verify);
			assert(false);
		}
        
	} // endfor (each glyph to cache)
	FT_Done_Face(face);
	FT_Done_FreeType(library);
    
	xpl_texture_atlas_commit(self->manager_atlas);
	font_generate_kerning(self);
    
	// How many glyphs were missed due to invalid space in the atlas?
	return missed;
}

float xpl_font_glyph_get_kerning(const xpl_glyph_t *self,
                                 const wchar_t charcode) {
    
	assert(self);
	xpl_kerning_t *kerning;
	int int_charcode = charcode;
	HASH_FIND_INT(self->kerning_table, &int_charcode, kerning);
    
	// All kerning pairs are supposed to be pre-generated, to keep us from
	// doing repeated font load calls to freetype;
	return kerning ? kerning->kerning : 0.0;
}

void xpl_font_apply_markup(xpl_font_t *self, const struct xpl_markup *markup) {
	self->outline_type = markup->outline;
    self->outline_thickness = markup->outline_thickness;
}

