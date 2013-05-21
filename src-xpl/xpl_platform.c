//
//  xpl-platform.c
//  xpl-osx
//
//  Created by Justin Bowes on 2012-09-08.
//  Copyright (c) 2012 Informi Software. All rights reserved.
//

#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <float.h>

#include "xpl.h"
#include "xpl_memory.h"
#include "xpl_rc.h"

#define PATH_SEP '/'

// ----------~ sleeeeep ~-----------------------
#if defined(XPL_PLATFORM_WINDOWS)

// winmm.dll function pointer typedefs
#       ifndef WIN32_LEAN_AND_MEAN
#               define WIN32_LEAN_AND_MEAN
#       endif
#       include <windows.h>
#       include <mmsystem.h>
typedef MMRESULT(WINAPI * JOYGETPOSEX_T) (UINT, LPJOYINFOEX);
typedef DWORD(WINAPI * TIMEGETTIME_T) (void);

static HMODULE lib_winmm = NULL;
static TIMEGETTIME_T s_time_get_time = NULL;

static void winmm_init() {
    if (lib_winmm == NULL) {
        lib_winmm = LoadLibrary("winmm.dll");
        s_time_get_time = (TIMEGETTIME_T)GetProcAddress(lib_winmm, "timeGetTime");
    }
}

static struct timer_info {
    int using_performance_counter;
    double resolution;

    union {
        __int64 start_time_64;
        __int32 start_time_32;
    };
} *s_timer_info = NULL;

void xpl_init_timer(void) {
    winmm_init();
    assert(s_timer_info == NULL);

    s_timer_info = xpl_alloc_type(struct timer_info);

    // Borrowed from GLFW for cases where we don't want to use it
    __int64 freq;
    if (QueryPerformanceFrequency((LARGE_INTEGER *) & freq)) {
        // Performance counter is available
        s_timer_info->using_performance_counter = TRUE;
        s_timer_info->resolution = 1.0 / (double) freq;
        QueryPerformanceCounter((LARGE_INTEGER *) & s_timer_info->start_time_64);
    } else {
        // Performance counter is unavailable
        s_timer_info->using_performance_counter = FALSE;
        s_timer_info->resolution = 0.001;
        s_timer_info->start_time_32 = s_time_get_time();
    }

}
double xpl_get_time(void) {
    double t;
    __int64 t_64;
    
    if (s_timer_info->using_performance_counter) {
        QueryPerformanceCounter((LARGE_INTEGER *) &t_64);
        t = (double)(t_64 - s_timer_info->start_time_64);
    } else {
        t = (double)(s_time_get_time() - s_timer_info->start_time_32);
    }

    return t;
}

#define SLEEP_MAX 2147483647.0
void xpl_sleep_seconds(double seconds) {
    DWORD t;
    
    if (seconds == 0.0) {
        t = 0;
    } else if (seconds < 0.001) {
        t = 1;
    } else if (seconds > SLEEP_MAX) {
        t = SLEEP_MAX;
    } else {
        t = (DWORD)(seconds * 1000.0 + 0.5); // round
    }
    Sleep(t);
}

#elif defined(XPL_PLATFORM_OSX) || defined(XPL_PLATFORM_IOS)
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <pthread.h>
#include <mach/mach_time.h>
#include <sys/time.h>

void xpl_sleep_seconds(double seconds) {
    if (seconds == 0.0) {
        sched_yield();
        return;
    }

    struct timeval current_time;
    struct timespec wait;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    long dt_sec, dt_usec;

    // Do a timed wait for a condition we will never signal.
    // usleep sleeps the process, not the thread.

    gettimeofday(&current_time, NULL);
    dt_sec = (long) seconds;
    dt_usec = (long) ((seconds - (double) dt_sec) * 1000000.0);
    wait.tv_nsec = (current_time.tv_usec + dt_usec) * 1000L;
    while (wait.tv_nsec > 1000000000L) {
        wait.tv_nsec -= 1000000000L;
        ++dt_sec;
    }
    wait.tv_sec = current_time.tv_sec + dt_sec;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);

    pthread_mutex_lock(&mutex);
    pthread_cond_timedwait(&condition, &mutex, &wait);
    pthread_mutex_unlock(&mutex);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);
}

XPLINLINE uint64_t get_absolute_time(void) {
    return mach_absolute_time();
}

static struct timer_info {
    double resolution;
    uint64_t start_timer;
} *s_timer_info = NULL;

void xpl_init_timer(void) {
    assert(!s_timer_info);

    mach_timebase_info_data_t info;
    mach_timebase_info(&info);

    s_timer_info = xpl_alloc_type(struct timer_info);
    s_timer_info->resolution = (double) info.numer / (info.denom * 1.0e9);
    s_timer_info->start_timer = get_absolute_time();
}

double xpl_get_time(void) {
    return (double) (get_absolute_time() - s_timer_info->start_timer) * s_timer_info->resolution;
}

#else

#include <sys/time.h>
#include <sched.h>
#include <pthread.h>

void xpl_sleep_seconds(double seconds) {
    if (seconds == 0.0) {
        sched_yield();
        return;
    }

    struct timeval current_time;
    struct timespec wait;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    long dt_sec, dt_usec;

    // Do a timed wait for a condition we will never signal.
    // usleep sleeps the process, not the thread.

    gettimeofday(&current_time, NULL);
    dt_sec = (long) seconds;
    dt_usec = (long) ((seconds - (double) dt_sec) * 1000000.0);
    wait.tv_nsec = (current_time.tv_usec + dt_usec) * 1000L;
    while (wait.tv_nsec > 1000000000L) {
        wait.tv_nsec -= 1000000000L;
        ++dt_sec;
    }
    wait.tv_sec = current_time.tv_sec + dt_sec;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);

    pthread_mutex_lock(&mutex);
    pthread_cond_timedwait(&condition, &mutex, &wait);
    pthread_mutex_unlock(&mutex);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);
}

static struct timer_info {
    double resolution;
    uint64_t start_timer;
    int monotonic;
} *s_timer_info = NULL;

XPLINLINE uint64_t get_absolute_time(void) {
    if (s_timer_info->monotonic) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (uint64_t) ts.tv_sec * (uint64_t) 1000000000 + (uint64_t) ts.tv_nsec;
    } else {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (uint64_t) tv.tv_sec * (uint64_t) 1000000 + (uint64_t) tv.tv_usec;
    }
}

void xpl_init_timer(void) {
    assert(!s_timer_info);

    s_timer_info = xpl_alloc_type(struct timer_info);

    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        s_timer_info->monotonic = TRUE;
        s_timer_info->resolution = 1e-9;
    } else {
        s_timer_info->monotonic = FALSE;
        s_timer_info->resolution = 1e-6;
    }
    s_timer_info->start_timer = get_absolute_time();
}

double xpl_get_time(void) {
    return (double) (get_absolute_time() - s_timer_info->start_timer) * s_timer_info->resolution;
}

#endif

// ------------ Thread support -----------------
#if defined(XPL_PLATFORM_OSX) || defined(XPL_PLATFORM_IOS)
// Mostly in xpl_thread, but this shim allows the OSX impl to be the same
// as the posix one
#include <pthread.h>
#include <errno.h>
struct ptj_args {
    int joined;
    pthread_t td;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    void **res;
};
static void *waiter(void *ap) {
    struct ptj_args *args = ap;
    pthread_join(args->td, args->res);
    pthread_mutex_lock(&args->mtx);
    pthread_mutex_unlock(&args->mtx);
    args->joined = 1;
    pthread_cond_signal(&args->cond);
    return 0;
}
int pthread_timedjoin_np(pthread_t td, void **res, struct timespec *ts) {
    pthread_t tmp;
    int ret;
    struct ptj_args args = { .td = td, .res = res };
    
    pthread_mutex_init(&args.mtx, 0);
    pthread_cond_init(&args.cond, 0);
    pthread_mutex_lock(&args.mtx);
    
    ret = pthread_create(&tmp, 0, waiter, &args);
    if (!ret) {
        do {
            ret = pthread_cond_timedwait(&args.cond, &args.mtx, ts);
        } while (! args.joined && ret != ETIMEDOUT);
        
        pthread_cancel(tmp);
        pthread_join(tmp, 0);
        
    }
    pthread_cond_destroy(&args.cond);
    pthread_mutex_destroy(&args.mtx);
    
    return args.joined ? 0 : ret;
}
#endif

// ------------ Resource handling --------------

#if defined(XPL_PLATFORM_WINDOWS)

#include <windows.h>
#include <stdlib.h>
#include <math.h>

// #define snprintf _snprintf

static char resource_path_format[PATH_MAX] = {0};

static const char *application_root_format() {
    if (resource_path_format[0] != 0) {
        return resource_path_format;
    }

    char executable_path[PATH_MAX] = {0};
    GetModuleFileNameA(NULL, &executable_path[0], PATH_MAX);
    LOG_INFO("Application path is %s", executable_path);

    char drive[_MAX_DRIVE] = {0};
    char dir[_MAX_DIR] = {0};

    _splitpath(executable_path, drive, dir, NULL, NULL);

    snprintf(resource_path_format, PATH_MAX, "%s%s%%s", drive, dir);
    LOG_INFO("Resource path format is %s", resource_path_format);
    return resource_path_format;
}

#define OS_SEP '\\'
#define PLATFORM_RESOURCE_ROOT_FORMAT       application_root_format()
#define PLATFORM_APP_RESOURCE_FORMAT        "resources\\%s"
#define PLATFORM_LIBRARY_RESOURCE_FORMAT    "xpl\\%s"
#define PLATFORM_WRITABLE_DATA_RESOURCE_FORMAT "data\\%s"
#define PLATFORM_DEV_RESOURCE_FORMAT		"..\\resources\\%s"

float fminf(float f1, float f2) {
    return f1 <= f2 ? f1 : f2;
}

float fmaxf(float f1, float f2) {
    return f1 >= f2 ? f1 : f2;
}

float roundf(float f) {
    return floorf(f + 0.5f);
}

#elif defined(XPL_PLATFORM_OSX)

#include <CoreFoundation/CoreFoundation.h>
#include <sched.h>
#include <pthread.h>

static char resource_path_format[PATH_MAX] = {0};

static const char *bundle_root_format() {
    if (resource_path_format[0] != 0) {
        return resource_path_format;
    }

    char resource_path[PATH_MAX] = {0};
    CFBundleRef main_bundle = CFBundleGetMainBundle();
    CFURLRef resources_url = CFBundleCopyResourcesDirectoryURL(main_bundle);
    if (!CFURLGetFileSystemRepresentation(resources_url, TRUE, (UInt8 *) resource_path, PATH_MAX)) {
        LOG_ERROR("Could not get file system representation for resources URL");
        exit(XPL_RC_RESOURCE_PATH_NOT_FOUND);
    }
    CFRelease(resources_url);

    LOG_INFO("Resource bundle path is %s", resource_path);

    snprintf(resource_path_format, PATH_MAX, "%s/%%s", resource_path);
    return resource_path_format;
}

#define OS_SEP '/'
#define PLATFORM_RESOURCE_ROOT_FORMAT       bundle_root_format()
#define PLATFORM_APP_RESOURCE_FORMAT        "resources/%s"
#define PLATFORM_LIBRARY_RESOURCE_FORMAT    "xpl/%s"
#define PLATFORM_DEV_RESOURCE_FORMAT		"../resources/%s"

#else

#define OS_SEP '/'
#define PLATFORM_RESOURCE_ROOT_FORMAT       "%s"
#define PLATFORM_APP_RESOURCE_FORMAT        "resources/%s"
#define PLATFORM_LIBRARY_RESOURCE_FORMAT    "xpl/%s"
#define PLATFORM_WRITABLE_DATA_RESOURCE_FORMAT "data/%s"
#define PLATFORM_DEV_RESOURCE_FORMAT		"../resources/%s"

#endif

static void str_replace(char *s, char find, char replace) {
    for (char *p = s; *p; p++) {
        if (*p == find) *p = replace;
    }
}

const char *xpl_resource_extension(const char *filename) {
	const char *dot = strrchr(filename, '.');
	if (!dot || dot == filename) return "";
	return dot + 1;
}

void xpl_create_generic_path(char *path_out, const char *path_in, size_t length) {
    assert(length > 0);

    strncpy(path_out, path_in, length);
    if (OS_SEP != PATH_SEP) {
        str_replace(path_out, OS_SEP, PATH_SEP);
    }
    path_out[length - 1] = 0;
}

static void format_resource_path(char *path_out, const char *path_in, const char *subformat, size_t length) {
	assert(path_in != path_out); // Fails, don't feel like figuring out why now.
    assert(length > 0);
    // Assumes PATH_SEP length == OS_SEP length on all platforms.  If PATH_SEP is ::, this explodes.
    char *path_temp1 = (char *) xpl_alloc(sizeof (char) * length);
    char *path_temp2 = (char *) xpl_alloc(sizeof (char) * length);

    // We go back and forth in case the separators were mixed
    xpl_create_generic_path(path_temp1, path_in, length);
    if (OS_SEP != PATH_SEP) {
        str_replace(path_temp1, PATH_SEP, OS_SEP);
    }

    snprintf(path_temp2, length, subformat, path_temp1);
    snprintf(path_out, length, PLATFORM_RESOURCE_ROOT_FORMAT, path_temp2);

    xpl_free(path_temp1);
    xpl_free(path_temp2);

    path_out[length - 1] = 0;
}

#if !defined(XPL_PLATFORM_IOS) && !defined(XPL_PLATFORM_OSX)
void xpl_resource_path(char *path_out, const char *path_in, size_t length) {
    format_resource_path(path_out, path_in, PLATFORM_APP_RESOURCE_FORMAT, length);
}
#endif

void xpl_library_resource_path(char *path_out, const char *path_in, size_t length) {
    format_resource_path(path_out, path_in, PLATFORM_LIBRARY_RESOURCE_FORMAT, length);
}

#if defined(XPL_PLATFORM_WINDOWS)
#include <shlobj.h>
void xpl_data_resource_path(char *path_out, const char *path_in, size_t length) {
	TCHAR szPath[MAX_PATH];
	// Not using SHGetKnownFolderPath because mingw prior to 2013-02 does not include knownfolders.h
    if (! SUCCEEDED(SHGetFolderPath(NULL,
    		  					    CSIDL_LOCAL_APPDATA,
    							    NULL,
    							    0,
    							    szPath))) {
    	LOG_ERROR("Couldn't retrieve Windows LOCAL_APPDATA path");
    	assert(0);
    }
    char local_appdata[PATH_MAX];
#	ifndef _UNICODE
    strcpy(local_appdata, szPath);
    char program_appdata[PATH_MAX];
    snprintf(program_appdata, PATH_MAX, "%s\\%s\\%s", local_appdata, "Informi", APP_NAME_PATH_STR);
    int create_dir_result = SHCreateDirectoryEx(NULL, program_appdata, NULL);
    if (create_dir_result != ERROR_SUCCESS &&
    		create_dir_result != ERROR_FILE_EXISTS &&
    		create_dir_result != ERROR_ALREADY_EXISTS) {
    	LOG_ERROR("Couldn't ensure directory %s exists", program_appdata);
    	assert(0);
    }
    snprintf(path_out, PATH_MAX, "%s\\%s", program_appdata, path_in);
#	else
    if (! WideCharToMultiByte(CP_UTF8, 0, szPath, -1, local_appdata, PATH_MAX, NULL, NULL)) {
    	LOG_ERROR("Couldn't convert wide tchar path to utf8 path");
    	// Not even sure if this is what should be done...not compiling for unicode atm.
    	assert(0);
    }
#	endif
}
#endif
#if defined(XPL_PLATFORM_OSX)
// obj-c implementation in xpl_platform.m
#endif
#if defined(XPL_PLATFORM_UNIX)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
void xpl_data_resource_path(char *path_out, const char *path_in, size_t length) {
	struct stat st = {0};
	snprintf(path_out, length, "~/.%s", path_in);
	if (stat(path_out, &st) == -1) {
		mkdir(path_out, 0700);
	}
}
#endif

void xpl_dev_resource_path(char *path_out, const char *path_in, size_t length) {
	format_resource_path(path_out, path_in, PLATFORM_DEV_RESOURCE_FORMAT, length);
}

int xpl_resource_exists(const char *resource_path) {
    FILE *file = fopen(resource_path, "r");
    if (file) {
        fclose(file);
        return TRUE;
    }
    return FALSE;
}

int xpl_resolve_resource(char *path_out, const char *path_in, size_t length) {
    xpl_resource_path(&path_out[0], path_in, length);
    if (xpl_resource_exists(&path_out[0])) return TRUE;

    xpl_library_resource_path(&path_out[0], path_in, length);
    if (xpl_resource_exists(&path_out[0])) return TRUE;

    xpl_dev_resource_path(&path_out[0], path_in, length);
    if (xpl_resource_exists(path_out)) return TRUE;

    xpl_data_resource_path(&path_out[0], path_in, length);
    if (xpl_resource_exists(&path_out[0])) return TRUE;

    // Don't clear the data path; maybe we want to write
    return FALSE;
}

void xpl_resource_resolve_opts(xpl_resolve_resource_opts_t *out, const char *path_in) {
	xpl_resource_path(&out->resource_path[0], path_in, PATH_MAX);
	xpl_library_resource_path(&out->library_resource_path[0], path_in, PATH_MAX);
	xpl_dev_resource_path(&out->dev_resource_path[0], path_in, PATH_MAX);
	xpl_data_resource_path(&out->data_resource_path[0], path_in, PATH_MAX);
}

