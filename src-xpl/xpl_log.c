#include "xpl.h"
#include "xpl_log.h"

#ifdef _WIN32

// Clean-room reimpl of GNU-like basename for Windows
#define INCLUDES_DRIVE(path)	((((path)[0] >= 'a' && (path)[1] <= 'z') || ((path)[0] >= 'A' && (path)[0] <= 'Z')) && (path)[1] == ':')
#define DRIVE_LENGTH(path)		(INCLUDES_DRIVE(path) ? 2 : 0)
#define IS_SEPARATOR(chr)		((chr) == '/' || (chr) == '\\')

char *basename(const char *name) {
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

#endif