/*
 * File:   xpl_hash.h
 * Author: justin
 *
 * Created on November 4, 2012, 12:09 PM
 */

#ifndef XPL_HASH_H
#define	XPL_HASH_H

#include <stdint.h>
#include <stddef.h>
#include <wchar.h>

#include "uthash.h"

#include "xpl_platform.h"

#define XPL_HASH_INIT           17

typedef struct hash_u {
	union {
		float pf;
		int pi;
	};
	int i;
} hash_u_t;

XPLINLINE int xpl_hashi(int param, int seed) {
	// Hash the param into MAX_INT buckets.
	int result = 0, hashv = 0;
    hash_u_t s_fi;
	s_fi.pi = param;
	s_fi.i = seed;
	struct hash_u *key = &s_fi;
	HASH_JEN(key, sizeof (s_fi), INT_MAX, hashv, result);
	return result;
}

XPLINLINE int xpl_hashf(float param, int seed) {
	// Hash the param into MAX_INT buckets.
	int result = 0, hashv = 0;
    hash_u_t s_fi;
	s_fi.pf = param;
	s_fi.i = seed;
	struct hash_u *key = &s_fi;
	HASH_JEN(key, sizeof (s_fi), INT_MAX, hashv, result);
	return result;
}

XPLINLINE int xpl_hashp(const void *param, int seed) {
#    if XPL_WORDSIZE == 64
	// 64-bit version
	uint64_t addr = (uint64_t)param;
	int hash = xpl_hashi(addr & UINT32_MAX, seed);
	hash = xpl_hashi((addr >> 32) & UINT32_MAX, hash);
	return hash;
#    else
	// 32-bit version
	return xpl_hashi((int)param, seed);
#    endif
}

XPLINLINE int xpl_hashs(const char *s, int seed)  {
    int result = 0, hashv = 0;
    HASH_JEN(s, strlen(s), INT_MAX, hashv, result);
	return xpl_hashi(result, seed);
}

XPLINLINE int xpl_hashwcs(const wchar_t *s, int seed) {
    int result = 0, hashv = 0;
    HASH_JEN(s, wcslen(s), INT_MAX, hashv, result);
	return xpl_hashi(result, seed);
}

typedef struct xpl_md5_context {

	uint8_t finished;
	uint32_t state[4]; /* state (ABCD) */
	uint32_t count[2]; /* number of bits, modulo 2^64 (lsb first) */
	uint8_t buffer[64]; /* input buffer */
	uint8_t digest_raw[16];
	char digest_chars[33];
} xpl_md5_context_t;

xpl_md5_context_t *xpl_md5_new(void);
void xpl_md5_destroy(xpl_md5_context_t **ppcontext);

void xpl_md5_update(xpl_md5_context_t *self, uint8_t *input, size_t input_len);
void xpl_md5_finish(xpl_md5_context_t *self);

void xpl_md5_digest_file(xpl_md5_context_t *self, const char *filename);
void xpl_md5_digest_bytes(xpl_md5_context_t *self, uint8_t *bytes, size_t length);
void xpl_md5_digest_string(xpl_md5_context_t *self, const char *string);

#endif	/* XPL_HASH_H */

