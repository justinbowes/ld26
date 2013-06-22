//
//  _version.h
//  protector
//
//  Created by Justin Bowes on 2013-06-22.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef protector__version_h
#define protector__version_h

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_REVISION 23
#define VERSION_BUILD 373
#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)
#define VERSION STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." STR(VERSION_REVISION) "." STR(VERSION_BUILD) " " __DATE__

#define LAST_BUILD_SUCCEEDED true

#endif
