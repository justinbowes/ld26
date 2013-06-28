//
//  xpl_platform.m
//  p1
//
//  Created by Justin Bowes on 2013-03-15.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <assert.h>

#include "xpl_platform.h"
#include "xpl_log.h"

#if defined(XPL_PLATFORM_OSX) || defined(XPL_PLATFORM_IOS)
#	include <Foundation/Foundation.h>
#	include <Foundation/NSFileManager.h>

void xpl_resource_path(char *path_out, const char *path_in, size_t length) {
	NSString *resource_bundle = [[NSBundle mainBundle] resourcePath];
	assert(resource_bundle);
	const char *resource_bundle_cc = [resource_bundle UTF8String];
	snprintf(path_out, PATH_MAX, "%s/%s", resource_bundle_cc, path_in);
}

void xpl_data_resource_path(char *path_out, const char *path_in, size_t length) {
    NSError *error;
    NSURL *appSupportDir = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory
                                                                  inDomain:NSUserDomainMask
                                                         appropriateForURL:nil
                                                                    create:YES
                                                                     error:&error];
    assert(appSupportDir);
    NSString *appPath = [NSString stringWithFormat:@"%s/%s", "Informi", APP_NAME_PATH_STR];
    NSURL *appSupportWritable = [NSURL URLWithString:appPath relativeToURL:appSupportDir];
    LOG_TRACE("Ensuring writable path exists at %s", [[appSupportWritable path] UTF8String]);
    BOOL result = [[NSFileManager defaultManager] createDirectoryAtURL:appSupportWritable withIntermediateDirectories:YES attributes:nil error:&error];
    assert(result);
    
    const char *url_prefix = [[appSupportWritable path] UTF8String];
    snprintf(path_out, PATH_MAX, "%s/%s", url_prefix, path_in);
}
#endif
