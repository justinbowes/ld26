//
//  xpl_l10n.c
//  p1
//
//  Created by Justin Bowes on 2013-03-15.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "minIni.h"
#include "uthash.h"

#include "xpl.h"
#include "xpl_l10n.h"

#define LOCALE_MAX 10

#define L10N_KEYLEN 128

typedef struct l10n_entry {
    char key[L10N_KEYLEN];
    char *mbs_value;
    
    UT_hash_handle hh;
} l10n_entry_t;

#ifdef DEBUG
static bool use_cached_results = true; // yeah, but we have to edit inside the bundle.
#else
static bool use_cached_results = true;
#endif

static char locale[LOCALE_MAX] = { 0 };
static char fallback_locale[LOCALE_MAX] = { 0 };
static l10n_entry_t *l10n_table = NULL;

static void clear_l10n_table(void) {
    l10n_entry_t *el, *tmp;
    HASH_ITER(hh, l10n_table, el, tmp) {
        HASH_DEL(l10n_table, el);
        xpl_free(el->mbs_value);
        xpl_free(el);
    }
}

void xpl_l10n_set_locale(const char *set_locale) {
    strncpy(locale, set_locale, LOCALE_MAX);
    clear_l10n_table();
    
    const char *localization_pref_resource = "l10n_prefs.ini";
    char localization_file[PATH_MAX];
    xpl_data_resource_path(localization_file, localization_pref_resource, PATH_MAX);
    ini_puts("l10n", "l10n", set_locale, localization_file);
}

void xpl_l10n_load_saved_locale() {
    const char *localization_pref_resource = "l10n_prefs.ini";
    char localization_file[PATH_MAX];
    if (! xpl_resolve_resource(localization_file, localization_pref_resource, PATH_MAX)) return;
    
    char saved_localization[LOCALE_MAX];
    ini_gets("l10n", "l10n", NULL, saved_localization, LOCALE_MAX, localization_file);
    if (strlen(saved_localization)) xpl_l10n_set_locale(saved_localization);
}

void xpl_l10n_set_fallback_locale(const char *set_locale) {
    strncpy(fallback_locale, set_locale, LOCALE_MAX);
    clear_l10n_table();
}

static char * unescape(const char *input) {
	char *output = xpl_calloc(strlen(input) + 1);
	bool is_escape = false;
	int j = 0, value = 0, chars_consumed = 0;
	const char *i = input;
	while (*i) {
		if (! is_escape) {
			if (*i == '\\') {
				is_escape = true;
			} else {
				output[j++] = *i;
			}
			i++;
		} else {
			// parse embedded sequences
			switch (*i) {
				case '0':
				{
					sscanf(i, "%3o%n", &value, &chars_consumed);
					output[j++] = value;
					i += chars_consumed;
					break;
				}
				case 'x':
				case 'X':
				{
					sscanf(i, "%x%n", &value, &chars_consumed);
					output[j++] = value;
					i += chars_consumed;
					break;
				}
				case 'u':
				case 'U':
				{
					// unsupported
					break;
				}
					
				case '\'':	output[j++] = '\'';	i++; break;
				case '"':	output[j++] = '"';	i++; break;
				case '?':	output[j++] = '?';	i++; break;
				case '\\':	output[j++] = '\\';	i++; break;
				case 'a':	output[j++] = '\a';	i++; break;
				case 'b':	output[j++] = '\b';	i++; break;
				case 'f':	output[j++] = '\f';	i++; break;
				case 'n':	output[j++] = '\n';	i++; break;
				case 'r':	output[j++] = '\r';	i++; break;
					
				default:
					break;
			}
			is_escape = false;
		}
	}
	return output;
}

static bool l10n_lookup(const char *loc, const char *key, char **l10n_out) {
    *l10n_out = NULL;
    
    char locale_key[L10N_KEYLEN];
    snprintf(locale_key, L10N_KEYLEN, "%s:%s", loc, key);
    
    l10n_entry_t *result;
    HASH_FIND_STR(l10n_table, locale_key, result);
    if (result && use_cached_results) {
        *l10n_out = result->mbs_value;
        return (*l10n_out != NULL);
    }
    
	int file_index = 0;
    LOG_DEBUG("Lookup IO for %s:%s", loc, key);
	
	char *text = NULL;
	char *unescaped = NULL;
	int chars_copied;
	while (true) {
		chars_copied = -1;
		char resource_name[PATH_MAX];
		if (loc && strlen(loc)) {
			snprintf(resource_name, PATH_MAX, "l10n_%s_%d.ini", loc, file_index++);
		} else {
			snprintf(resource_name, PATH_MAX, "l10n_%d.ini", file_index++);			
		}
		char filename[PATH_MAX];
		bool found = xpl_resolve_resource(filename, resource_name, PATH_MAX);
		if (found) {
			size_t buffer_size = 64;
			while (chars_copied == -1) {
				text = xpl_realloc(text, buffer_size * sizeof(char));
				text[0] = 0;
				chars_copied = ini_gets("l10n", key, "", text, (int)buffer_size, filename);
				if (chars_copied == buffer_size - 1) {
					buffer_size *= 1.5;
					chars_copied = -1;
				}
			}
			if (chars_copied == 0) {
				xpl_free(text);
				text = NULL;
			}
		}
		if (chars_copied > 0) {
			unescaped = unescape(text);
			xpl_free(text);
			break;
		}
		if (! found) break;
	}
	
    l10n_entry_t *entry = xpl_calloc_type(l10n_entry_t);
    strncpy(entry->key, locale_key, L10N_KEYLEN);
    entry->mbs_value = unescaped;
    HASH_ADD_STR(l10n_table, key, entry);
    
    *l10n_out = entry->mbs_value;
    
    return chars_copied > 0;
}

const char * xpl_l10n_get(const char *key) {
    assert(fallback_locale[0]);
    char *value = NULL;
    if (! l10n_lookup(locale, key, &value)) {
        if (! l10n_lookup(fallback_locale, key, &value)) {
            return key;
        }
    }
    return value;
}
