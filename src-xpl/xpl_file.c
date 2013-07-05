//
//  xpl_file.c
//  ld26
//
//  Created by Justin Bowes on 2013-04-25.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>


#include "xpl_file.h"
#include "xpl_platform.h"

const char *xpl_file_extension(const char *filename) {
	char *e = strrchr(filename, '.');
	if (e == NULL) e = "";
	return ++e;
}

bool xpl_file_has_extension(const char *filename, const char *extension) {
	const char *file_extension = xpl_file_extension(filename);
	return strcmp(file_extension, extension) == 0;
}

void xpl_file_get_contents(const char *filename, xpl_dynamic_buffer_t *buffer) {
	FILE *file = fopen(filename, "rb");
	if (! file) {
		LOG_ERROR("Couldn't open file %s", filename);
		return;
	}
	fseek(file, 0, SEEK_END);
	size_t len = ftell(file);
	fseek(file, 0, SEEK_SET);
	xpl_dynamic_buffer_alloc(buffer, len, true);
	fread(buffer->content, len, 1, file);
	fclose(file);
}

// Clean-room reimpl of GNU-like basename for Windows
#define INCLUDES_DRIVE(path)	((((path)[0] >= 'a' && (path)[0] <= 'z') || \
								  ((path)[0] >= 'A' && (path)[0] <= 'Z')) && (path)[1] == ':')
#define DRIVE_LENGTH(path)		(INCLUDES_DRIVE(path) ? 2 : 0)
#define IS_SEPARATOR(chr)		((chr) == '/' || (chr) == '\\')

char *xpl_basename(const char *name) {
	const char *after_last = name + DRIVE_LENGTH(name);
	int is_all_slashes = TRUE;
	const char *cp;
	
	for (cp = name; *cp != '\0'; cp++) {
		if (IS_SEPARATOR(*cp)) {
			after_last = cp + 1;
		} else {
			is_all_slashes = 0;
		}
	}
	
	if (*after_last == '\0' && IS_SEPARATOR(*name) && is_all_slashes) {
		// What did you do?!
		--after_last; // Return final slash.
	}
	
	return (char *)after_last;
}

/**
 * Warning: this is thorough, but not not bad.
 */
char *xpl_dirname(char *path)
{
	static char *retfail = NULL;
	wchar_t refcopy[2 * PATH_MAX + 1];
	wchar_t *refpath;
	size_t len = 0;
	
	/* to handle path names for files in multibyte character locales,
	 * we need to set up LC_CTYPE to match the host file system locale.  */
	char *locale = setlocale(LC_CTYPE, NULL);
	
	if (locale != NULL)
		locale = strdup (locale);
	
	setlocale (LC_CTYPE, "");
	
	if (path && *path) {
		/* allocate sufficient local storage space,
		 * in which to create a wide character reference copy of path.  */
		// THIS LINE DOES NOTHING
		// refcopy[1 + (len = xpl_mbs_to_wcs(NULL, path, 0))];
		/* create the wide character reference copy of path */
		refpath = refcopy;
		
		len = xpl_mbs_to_wcs(path, refpath, len);
		refcopy[len] = L'\0';
		/* SUSv3 identifies a special case, where path is exactly equal to "//";
		 * (we will also accept "\\" in the Win32 context, but not "/\" or "\/",
		 *  and neither will we consider paths with an initial drive designator).
		 * For this special case, SUSv3 allows the implementation to choose to
		 * return "/" or "//", (or "\" or "\\", since this is Win32); we will
		 * simply return the path unchanged, (i.e. "//" or "\\").  */
		if (len > 1 && (refpath[0] == L'/' || refpath[0] == L'\\')) {
			if (refpath[1] == refpath[0] && refpath[2] == L'\0') {
				setlocale(LC_CTYPE, locale);
				free(locale);
				return path;
            }
		} else if (len > 1 && refpath[1] == L':') {
			/* For all other cases ...
			 * step over the drive designator, if present ...  */
			/* FIXME: maybe should confirm *refpath is a valid drive designator.  */
			refpath += 2;
        }
		/* check again, just to ensure we still have a non-empty path name ... */
		if (*refpath) {
			/* reproduce the scanning logic of the "basename" function
			 * to locate the basename component of the current path string,
			 * (but also remember where the dirname component starts).  */
			wchar_t *refname, *w_basename;
			for (refname = w_basename = refpath; *refpath; ++refpath)
            {
				if (*refpath == L'/' || *refpath == L'\\')
                {
					/* we found a dir separator ...
					 * step over it, and any others which immediately follow it.  */
					while (*refpath == L'/' || *refpath == L'\\')
						++refpath;
					/* if we didn't reach the end of the path string ... */
					if (*refpath)
                    /* then we have a new candidate for the base name.  */
						w_basename = refpath;
					else
                    /* we struck an early termination of the path string,
                     * with trailing dir separators following the base name,
                     * so break out of the for loop, to avoid overrun.  */
						break;
                }
            }
			/* now check,
			 * to confirm that we have distinct dirname and basename components.  */
			if (w_basename > refname)
            {
				/* and, when we do ...
				 * backtrack over all trailing separators on the dirname component,
				 * (but preserve exactly two initial dirname separators, if identical),
				 * and add a NUL terminator in their place.  */
				do {
					--w_basename;
				} while (w_basename > refname && (*w_basename == L'/' || *w_basename == L'\\'));
				
				if (w_basename == refname &&
					(refname[0] == L'/' || refname[0] == L'\\')
					&& refname[1] == refname[0]
					&& refname[2] != L'/'
					&& refname[2] != L'\\') {
					
					++w_basename;
				}
				*++w_basename = L'\0';
				
				/* if the resultant dirname begins with EXACTLY two dir separators,
				 * AND both are identical, then we preserve them.  */
				refpath = refcopy;
				
				while ((*refpath == L'/' || *refpath == L'\\')) {
					++refpath;
				}
				if ((refpath - refcopy) > 2 || refcopy[1] != refcopy[0]) {
					refpath = refcopy;
				}
				/* and finally ...
				 * we remove any residual, redundantly duplicated separators from the dirname,
				 * reterminate, and return it.  */
				refname = refpath;
				while (*refpath) {
					if ((*refname++ = *refpath) == L'/' || *refpath++ == L'\\') {
						while (*refpath == L'/' || *refpath == L'\\') {
							++refpath;
						}
                    }
                }
				*refname = L'\0';
				/* finally ...
				 * transform the resolved dirname back into the multibyte char domain,
				 * restore the caller's locale, and return the resultant dirname.  */
				if ((len = wcstombs( path, refcopy, len )) != (size_t)(-1)) {
					path[len] = '\0';
				}
            }
			else
            {
				/* either there were no dirname separators in the path name,
				 * or there was nothing else ...  */
				if (*refname == L'/' || *refname == L'\\') {
					/* it was all separators, so return one.  */
					++refname;
                } else {
					/* there were no separators, so return '.'.  */
					*refname++ = L'.';
                }
				/* add a NUL terminator, in either case,
				 * then transform to the multibyte char domain,
				 * using our own buffer.  */
				*refname = L'\0';
				retfail = xpl_realloc(retfail, len = 1 + wcstombs(NULL, refcopy, 0));
				xpl_wcs_to_mbs(refcopy, path = retfail, len);
            }
			/* restore caller's locale, clean up, and return the resolved dirname.  */
			setlocale(LC_CTYPE, locale);
			free(locale);
			return path;
        }
    }
	/* path is NULL, or an empty string; default return value is "." ...
	 * return this in our own buffer, regenerated by wide char transform,
	 * in case the caller trashed it after a previous call.
	 */
	retfail = xpl_realloc(retfail, len = 1 + wcstombs (NULL, L".", 0));
	xpl_wcs_to_mbs(L".", retfail, len);
	
	/* restore caller's locale, clean up, and return the default dirname.  */
	setlocale (LC_CTYPE, locale);
	
	free (locale);
	
	return retfail;
}

