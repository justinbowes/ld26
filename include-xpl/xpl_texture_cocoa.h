//
//  xpl_texture_uiimage.h
//  app
//
//  Created by Justin Bowes on 2013-06-26.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef app_xpl_texture_uiimage_h
#define app_xpl_texture_uiimage_h

#include "xpl_platform.h"
#include "xpl_gl.h"
#include "xpl_texture.h"

#if defined(XPL_PLATFORM_IOS)
#import <UIKit/UIKit.h>
GLuint xpl_texture_load_uiimage(xpl_texture_t *self, UIImage *image);
#else
#error NO
#endif


#endif
