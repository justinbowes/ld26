//
//  log.h
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-02.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#ifndef xpl_osx_log_h
#define xpl_osx_log_h

#include <stdio.h>

#include "xpl_platform.h"

// ----------- logging ----------------------
#define XPL_LOG_LEVEL_TRACE             0
#define XPL_LOG_LEVEL_DEBUG             1
#define XPL_LOG_LEVEL_INFO              2
#define XPL_LOG_LEVEL_WARN              3
#define XPL_LOG_LEVEL_ERROR             4

#define XPL_LOG_LEVEL_OFF               999
#define XPL_LOG_LEVEL_ALL               XPL_LOG_LEVEL_TRACE

#ifdef DEBUG
#define XPL_LOG_LEVEL					XPL_LOG_LEVEL_DEBUG
#endif

#ifndef XPL_LOG_LEVEL
#define XPL_LOG_LEVEL                   XPL_LOG_LEVEL_INFO
#endif

#define LOG_ANSI

#if defined(XPL_PLATFORM_WINDOWS) || defined(XPL_PLATFORM_OSX)
#undef LOG_ANSI
#else
#define XPL_STDERR stderr
#define XPL_STDOUT stdout
#endif

#if !defined(LOG_ANSI)
#define __func__ __FUNCTION__
#define XPL_COLOR_txtblk ""
#define XPL_COLOR_txtred ""
#define XPL_COLOR_txtgrn ""
#define XPL_COLOR_txtylw ""
#define XPL_COLOR_txtblu ""
#define XPL_COLOR_txtpur ""
#define XPL_COLOR_txtcyn ""
#define XPL_COLOR_txtwht ""
#define XPL_COLOR_bldblk ""
#define XPL_COLOR_bldred ""
#define XPL_COLOR_bldgrn ""
#define XPL_COLOR_bldylw ""
#define XPL_COLOR_bldblu ""
#define XPL_COLOR_bldpur ""
#define XPL_COLOR_bldcyn ""
#define XPL_COLOR_bldwht ""
#define XPL_COLOR_undblk ""
#define XPL_COLOR_undred ""
#define XPL_COLOR_undgrn ""
#define XPL_COLOR_undylw ""
#define XPL_COLOR_undblu ""
#define XPL_COLOR_undpur ""
#define XPL_COLOR_undcyn ""
#define XPL_COLOR_undwht ""
#define XPL_COLOR_bakblk ""
#define XPL_COLOR_bakred ""
#define XPL_COLOR_bakgrn ""
#define XPL_COLOR_bakylw ""
#define XPL_COLOR_bakblu ""
#define XPL_COLOR_bakpur ""
#define XPL_COLOR_bakcyn ""
#define XPL_COLOR_bakwht ""
#define XPL_COLOR_txtrst ""
char *basename(const char *name);

#else
#include <libgen.h>
#define XPL_COLOR_txtblk "\033[0;30m"
#define XPL_COLOR_txtred "\033[0;31m"
#define XPL_COLOR_txtgrn "\033[0;32m"
#define XPL_COLOR_txtylw "\033[0;33m"
#define XPL_COLOR_txtblu "\033[0;34m"
#define XPL_COLOR_txtpur "\033[0;35m"
#define XPL_COLOR_txtcyn "\033[0;36m"
#define XPL_COLOR_txtwht "\033[0;37m"
#define XPL_COLOR_bldblk "\033[1;30m"
#define XPL_COLOR_bldred "\033[1;31m"
#define XPL_COLOR_bldgrn "\033[1;32m"
#define XPL_COLOR_bldylw "\033[1;33m"
#define XPL_COLOR_bldblu "\033[1;34m"
#define XPL_COLOR_bldpur "\033[1;35m"
#define XPL_COLOR_bldcyn "\033[1;36m"
#define XPL_COLOR_bldwht "\033[1;37m"
#define XPL_COLOR_undblk "\033[4;30m"
#define XPL_COLOR_undred "\033[4;31m"
#define XPL_COLOR_undgrn "\033[4;32m"
#define XPL_COLOR_undylw "\033[4;33m"
#define XPL_COLOR_undblu "\033[4;34m"
#define XPL_COLOR_undpur "\033[4;35m"
#define XPL_COLOR_undcyn "\033[4;36m"
#define XPL_COLOR_undwht "\033[4;37m"
#define XPL_COLOR_bakblk "\033[40m"
#define XPL_COLOR_bakred "\033[41m"
#define XPL_COLOR_bakgrn "\033[42m"
#define XPL_COLOR_bakylw "\033[43m"
#define XPL_COLOR_bakblu "\033[44m"
#define XPL_COLOR_bakpur "\033[45m"
#define XPL_COLOR_bakcyn "\033[46m"
#define XPL_COLOR_bakwht "\033[47m"
#define XPL_COLOR_txtrst "\033[0m"
#endif

#if (XPL_LOG_LEVEL < XPL_LOG_LEVEL_OFF)
#define LOG_MAX 2048
static char __xpl_log_output[LOG_MAX] XPL_UNUSED;
#	ifdef _WIN32
/* MinGW points to _iob which the IDEs don't like */
#		define xpl_log(stream, color, level, file, func, line, ...) printf("[%s%s%s]\t %s %s:%d %s\n", color, level, XPL_COLOR_txtrst, basename(file), func, line, (snprintf(__xpl_log_output, LOG_MAX, __VA_ARGS__), __xpl_log_output)); fflush(stdout)
#	else
#		define xpl_log(stream, color, level, file, func, line, ...) /* LOG level */ fprintf(stream, "[%s%s%s]\t %s %s:%d %s\n", color, level, XPL_COLOR_txtrst, basename(file), func, line, (snprintf(__xpl_log_output, LOG_MAX, __VA_ARGS__), __xpl_log_output)); fflush(stream)
#	endif
#else
#define xpl_log(s, c, lv, f, fn, ln, ...)
#endif

#if (XPL_LOG_LEVEL <= XPL_LOG_LEVEL_TRACE)
#define LOG_TRACE(...)               xpl_log(XPL_STDOUT, XPL_COLOR_txtrst XPL_COLOR_txtwht, "trace", __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define LOG_TRACE(...)
#endif

#if (XPL_LOG_LEVEL <= XPL_LOG_LEVEL_DEBUG)
#define LOG_DEBUG(...)               xpl_log(XPL_STDOUT, XPL_COLOR_txtrst XPL_COLOR_txtwht, "debug", __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

#if (XPL_LOG_LEVEL <= XPL_LOG_LEVEL_INFO)
#define LOG_INFO(...)                xpl_log(XPL_STDOUT, XPL_COLOR_txtrst XPL_COLOR_txtgrn, "info", __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define LOG_INFO(...)
#endif

#if (XPL_LOG_LEVEL <= XPL_LOG_LEVEL_WARN)
#define LOG_WARN(...)                xpl_log(XPL_STDERR, XPL_COLOR_txtrst XPL_COLOR_bldylw, "warn", __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define LOG_WARN(...)
#endif

#if (XPL_LOG_LEVEL <= XPL_LOG_LEVEL_ERROR)
#define LOG_ERROR(...)               xpl_log(XPL_STDERR, XPL_COLOR_txtrst XPL_COLOR_bakred XPL_COLOR_bldylw, "error", __FILE__, __func__, __LINE__, __VA_ARGS__)
#else
#define LOG_ERROR(...)
#endif

#endif
