//
//  xpl_platform.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-08.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_xpl_platform_h
#define xpl_osx_xpl_platform_h

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>
#include <math.h>

// ---------- Defines for platform ------------
// XPL_PLATFORM will be set to one of the _D targets below
// XPL_PLATFORM_(type) will be defined
// XPL_PLATFORM_(othertypes) will be undefined

#define XPL_PLATFORM_WINDOWS_D	1
#define XPL_PLATFORM_OSX_D		2
#define XPL_PLATFORM_IOS_D		4
#define XPL_PLATFORM_UNIX_D		8

#ifdef _WIN32
#undef __STRICT_ANSI__
#endif

#if defined(_WIN32)
#	define WINVER 0x0501
#	define _WIN32_WINNT 0x0501
#	if ! defined(WIN32_LEAN_AND_MEAN)
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#	define XPL_TOOLKIT_GLFW
#	define XPL_PLATFORM XPL_PLATFORM_WINDOWS_D
#	define XPL_PLATFORM_WINDOWS XPL_PLATFORM
#	define XPL_PLATFORM_STRING "Windows"
#elif defined(__APPLE__)
#	include <TargetConditionals.h>
#	if (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
#		define XPL_TOOLKIT_IOS
#		define XPL_PLATFORM XPL_PLATFORM_IOS_D
#		define XPL_PLATFORM_IOS XPL_PLATFORM
#		define XPL_PLATFORM_STRING "IOS"
#	else
#		define XPL_TOOLKIT_GLFW
#		define XPL_PLATFORM XPL_PLATFORM_OSX_D
#		define XPL_PLATFORM_OSX XPL_PLATFORM
#		define XPL_PLATFORM_STRING "OSX"
#	endif
#else
#	define XPL_TOOLKIT_GLFW
#	define XPL_PLATFORM XPL_PLATFORM_UNIX_D
#	define XPL_PLATFORM_UNIX XPL_PLATFORM
#	define XPL_PLATFORM_STRING "Unix"
#endif


// ---------- calling convention ------------
#ifdef XPL_PLATFORM_WINDOWS
#define XPLCALL __stdcall
#else
#define XPLCALL
#endif

#ifdef XPL_PLATFORM_WINDOWS
#define XPLINLINE static inline
#else
#define XPLINLINE static inline
// My latest reading is that these can't work, despite the fact that they do.
// #define XPLINLINE_DECL extern inline
// #define XPLINLINE_IMPL inline
#endif


// ---------- string stuff -------------------
#ifdef XPL_PLATFORM_WINDOWS
float fminf(float f1, float f2);
float fmaxf(float f1, float f2);
float roundf(float f);
#else
#	define snwprintf swprintf
#	if defined(XPL_PLATFORM_OSX) || defined(XPL_PLATFORM_IOS)
#		include <sys/syslimits.h>
#	else
#		include <linux/limits.h>
#	endif
#endif

// ------------- thread stuff -------------------
#if defined(XPL_PLATFORM_OSX) || defined(XPL_PLATFORM_IOS)
#include <pthread.h>
int pthread_timedjoin_np(pthread_t td, void **res, struct timespec *ts);
#endif

// ----------- standard integer types -------------
#include <stdint.h>

// ------------ type sizes ------------------
#ifdef __WORDSIZE
#define XPL_WORDSIZE __WORDSIZE
#elif defined(__INTPTR_WIDTH__)
#define XPL_WORDSIZE __INTPTR_WIDTH__
#elif defined(UINTPTR_MAX) && UINTPTR_MAX > UINT32_MAX
#define XPL_WORDSIZE    64
#elif defined(UINTPTR_MAX) && UINTPTR_MAX == UINT32_MAX
#define XPL_WORDSIZE 32
#else
#error "Couldn't determine platform pointer size"
#endif

#if XPL_WORDSIZE == 64
#define xpl_uiword      uint64_t
#define xpl_iword       int64_t
#elif XPL_WORDSIZE == 32
#define xpl_uiword      uint64_t
#define xpl_iword       int64_t
#else
#error "Word size is weird"
#endif

// ------------- macro magic -----------------------
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)

// ------------- unused variables -------------------
#ifdef XPL_UNUSED
#elif defined (__llvm__)
#       define XPL_UNUSED __unused
#elif defined (__GNUC__)
#       define XPL_UNUSED __attribute__((unused))
#endif

void xpl_sleep_seconds(double seconds);
void xpl_init_timer(void);
double xpl_get_time(void);

const char *xpl_resource_extension(const char *filename);

typedef union xpl_resolve_resource_opts {
	const char resource_paths[4][PATH_MAX];
	struct {
		char resource_path[PATH_MAX];
		char library_resource_path[PATH_MAX];
		char dev_resource_path[PATH_MAX];
		char data_resource_path[PATH_MAX];
	};
} xpl_resolve_resource_opts_t;

void xpl_create_generic_path(char *path_out, const char *path_in, size_t length);

void xpl_resource_path(char *path_out, const char *path_in, size_t length);
void xpl_library_resource_path(char *path_out, const char *path_in, size_t length);
void xpl_data_resource_path(char *path_out, const char *path_in, size_t length);
int xpl_resolve_resource(char *path_out, const char *path_in, size_t length);
int xpl_resource_exists(const char *resource_path);
void xpl_resource_resolve_opts(xpl_resolve_resource_opts_t *out, const char *path_in);
size_t xpl_mbs_to_wcs(const char *mbs, wchar_t *wcs, size_t wcs_size);
size_t xpl_wcs_to_mbs(const wchar_t *wcs, char *mbs, size_t mbs_size);

#include "xpl.h"

#ifndef APPLICATION_NAME_PATH
#   error APPLICATION_NAME_PATH must be defined
#endif
#define APP_NAME_PATH_STR EXPAND_AND_STRINGIFY(APPLICATION_NAME_PATH)

#endif
