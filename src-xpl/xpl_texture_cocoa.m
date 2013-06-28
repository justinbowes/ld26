#include "xpl_texture_cocoa.h"
#include "SOIL.h"

GLuint xpl_texture_load_cgimage(xpl_texture_t *self, CGImageRef cg_image) {
	const size_t bpp = 8;
	self->channels = 4;
	self->size = xivec2_set((int)CGImageGetWidth(cg_image), (int)CGImageGetHeight(cg_image));
	size_t bytes = self->size.width * self->size.height * self->channels * ceill(bpp / 8);
	void *texture_data = xpl_alloc(bytes);
	CGContextRef texture_context = CGBitmapContextCreate(texture_data,
														 self->size.width, self->size.height,
														 bpp, self->size.width * self->channels,
														 CGImageGetColorSpace(cg_image),
														 kCGImageAlphaPremultipliedLast);
	CGContextDrawImage(texture_context, CGRectMake(0.f, 0.f, self->size.width, self->size.height), cg_image);
	CGContextRelease(texture_context);
	
	GLint original_unpack_alignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_unpack_alignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	self->texture_id = SOIL_create_OGL_texture(texture_data,
											   self->size.x, self->size.y,
											   self->channels,
											   self->texture_id ? self->texture_id : SOIL_CREATE_NEW_ID,
											   SOIL_FLAG_INVERT_Y);
	glPixelStorei(GL_UNPACK_ALIGNMENT, original_unpack_alignment);
    GL_DEBUG();
	
	return self->texture_id;
}

#if defined(XPL_PLATFORM_IOS)
GLuint xpl_texture_load_uiimage(xpl_texture_t *self, UIImage *image) {
	CGImageRef cg_image = image.CGImage;
	return xpl_texture_load_cgimage(cg_image);
}
#endif