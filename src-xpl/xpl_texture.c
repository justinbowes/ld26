//
//  xpl_texture.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-10-07.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <assert.h>
#include <stdarg.h>

#include "xpl_gl.h"
#include "SOIL.h"

#include "xpl_log.h"
#include "xpl_memory.h"
#include "xpl_gl_debug.h"
#include "xpl_platform.h"

#include "xpl_texture.h"

xpl_texture_t *xpl_texture_new(void) {

	xpl_texture_t *texture = xpl_calloc_type(xpl_texture_t);
	return texture;
}

void xpl_texture_destroy(xpl_texture_t **pptexture) {
	xpl_texture_t *texture = *pptexture;

	if (texture->texture_id) {
		glDeleteTextures(1, &texture->texture_id);
	}

	*pptexture = NULL;
}

GLuint xpl_texture_load(xpl_texture_t *self, const char *resource_name, bool allow_compress) {
	char filename[PATH_MAX];
	if (!xpl_resolve_resource(filename, resource_name, PATH_MAX)) {
		LOG_ERROR("Couldn't load resource: %s", resource_name);
		return 0;
	}

	GLint original_unpack_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_unpack_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	unsigned char *data = SOIL_load_image(filename, &self->size.x,
			&self->size.y, &self->channels, FALSE);
	self->texture_id = SOIL_create_OGL_texture(data,
											   self->size.x, self->size.y,
											   self->channels,
											   self->texture_id ? self->texture_id : SOIL_CREATE_NEW_ID,
											   (allow_compress ? SOIL_FLAG_COMPRESS_TO_DXT : 0) | SOIL_FLAG_INVERT_Y);
	SOIL_free_image_data(data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, original_unpack_alignment);
    GL_DEBUG();

	return self->texture_id;
}

GLuint xpl_texture_load_array(xpl_texture_t *self, ...) {
	GLint original_unpack_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_unpack_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	char *resource_name[255] = { 0 };
	char *name;
	va_list args;
	int elements = 0;
	va_start(args, self);
	do {
		name = va_arg(args, char *);
		if (!name)
			break; // done;
		resource_name[elements++] = name;
	} while (resource_name[elements]);
	va_end(args);

	// Generate an OpenGL texture if required.
	if (self->texture_id == 0) {
		glGenTextures(1, &self->texture_id);
	}
	glBindTexture(GL_TEXTURE_2D_ARRAY, self->texture_id);

	// For each argument, construct a real filename, load the image, and append it to a buffer.
	char filename[PATH_MAX];
	int width, height, channels;
	for (int i = 0; i < elements; i++) {

		if (!xpl_resolve_resource(filename, resource_name[i], PATH_MAX)) {
			LOG_ERROR("Failed loading element %s", resource_name[i]);
			return 0;
		}

		unsigned char *data = SOIL_load_image(filename, &width, &height,
				&channels, SOIL_LOAD_AUTO);

		GLint format, internal_format;
		if (i == 0) {
			switch (channels) {
			case 1:
				format = GL_RED;
				internal_format = GL_R8;
				break;

			case 2:
				format = GL_RG;
				internal_format = GL_RG8;
				break;

			case 3:
				format = GL_RGB;
				internal_format = GL_RGB8;
				break;

			case 4:
				format = GL_RGBA8;
				internal_format = GL_RGBA8;
				break;

			default:
				LOG_ERROR("Unrecognized channel count %d", channels);
				return 0;
			}
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internal_format,
                         width, height, elements, 0, format,
                         GL_UNSIGNED_BYTE, NULL);
		}

		int x, y;
		for (y = 0; y * 2 < height; ++y) {
			int index1 = y * width * channels;
			int index2 = (height - 1 - y) * width * channels;
			for (x = width * channels; x > 0; --x) {
				unsigned char temp = data[index1];
				data[index1] = data[index2];
				data[index2] = temp;
				++index1;
				++index2;
			}
		}

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
                        width, height, 1, format,
                        GL_UNSIGNED_BYTE, data);

		free(data);

	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, original_unpack_alignment);

    glBindTexture(GL_TEXTURE_2D_ARRAY, GL_NONE);
    
    GL_DEBUG();
    
	return self->texture_id;
}

xpl_texture_region_t *xpl_texture_region_new(xpl_texture_t *texture,
		xirect *region) {
	assert(texture);
	xpl_texture_region_t *tr = xpl_alloc_type(xpl_texture_region_t);
	const float width = texture->size.x;
	const float height = texture->size.y;
	if (region == NULL ) {
		tr->region_i = xirect_set(0, 0, texture->size.x, texture->size.y);
		tr->region_f = xrect_set(0.f, 0.f, 1.f, 1.f);
	} else {
		tr->region_i = *region;
		tr->region_f = xrect_set(region->x / width,
								 region->y / height,
								 region->width / width,
								 region->height / height);
	}
	return tr;
}

void xpl_texture_region_destroy(xpl_texture_region_t **ppregion) {
	assert(ppregion);
	xpl_texture_region_t *region = *ppregion;
	assert(region);
	xpl_free(region);
	*ppregion = NULL;
}
