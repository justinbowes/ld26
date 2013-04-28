//
//  xpl.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-01.
//  Copyright (c) 2012 Justin Bowes. All rights reserved.
//

#ifndef xpl_osx_xpl_h
#define xpl_osx_xpl_h

#include "app_settings.h"

// ------------- sanity ---------------------
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
 #ifdef __cplusplus
  #define NULL 0
 #else
  #define NULL ((void *)0)
 #endif
#endif /* NULL */

#include "xpl_platform.h"
#include "xpl_memory.h"
#include "xpl_log.h"
#include "xpl_l10n.h"


// ---------- eine kleine math --------------
#define xmin(x, y) 			(x < y ? x : y)
#define xmax(x, y) 			(x > y ? x : y)
#define xclamp(x, min, max) (x < min ? min : (x > max ? max : x))

#endif
