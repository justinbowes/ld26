#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "xpl_memory.h"
#include "xpl_log.h"
#include "xpl_hash.h"

// Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
// rights reserved.

// License to copy and use this software is granted provided that it
// is identified as the "RSA Data Security, Inc. MD5 Message-Digest
// Algorithm" in all material mentioning or referencing this software
// or this function.
//
// License is also granted to make and use derivative works provided
// that such works are identified as "derived from the RSA Data
// Security, Inc. MD5 Message-Digest Algorithm" in all material
// mentioning or referencing the derived work.
//
// RSA Data Security, Inc. makes no representations concerning either
// the merchantability of this software or the suitability of this
// software for any particular purpose. It is provided "as is"
// without express or implied warranty of any kind.
//
// These notices must be retained in any copies of any part of this
// documentation and/or software.

// Constants for MD5Transform routine.
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static uint8_t PADDING[64] = {
							  0x80, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0
};

// F, G, H and I are basic MD5 functions.
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits.
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + (uint32_t)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) { \
  (a) += G ((b), (c), (d)) + (x) + (uint32_t)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + (uint32_t)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) { \
  (a) += I ((b), (c), (d)) + (x) + (uint32_t)(ac); \
  (a) = ROTATE_LEFT ((a), (s)); \
  (a) += (b); \
  }

typedef uint8_t BYTE;

// POINTER defines a generic pointer type
typedef uint8_t *POINTER;



// Decodes input (uint8_t) into output (uint32_t). Assumes len is
// a multiple of 4.

static void md5_decode(uint32_t *output, uint8_t *input, size_t len) {
	assert((len % 4) == 0);
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[i] = (
					((uint32_t)input[j]) |
					(((uint32_t)input[j + 1]) << 8) |
					(((uint32_t)input[j + 2]) << 16) |
					(((uint32_t)input[j + 3]) << 24)
					);
	}
}

// The core of the MD5 algorithm is here.
// MD5 basic transformation. Transforms state based on block.

static void md5_transform(uint32_t state[4], uint8_t block[64]) {
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	md5_decode(x, block, 64);

	/* Round 1 */
	FF(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG(d, a, b, c, x[10], S22, 0x2441453); /* 22 */
	GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH(b, c, d, a, x[ 6], S34, 0x4881d05); /* 44 */
	HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	// Zeroize sensitive information.
	memset((POINTER)x, 0, sizeof (x));
}

// Encodes input (uint32_t) into output (uint8_t). Assumes len is
// a multiple of 4.

static void md5_encode(uint8_t *output, uint32_t *input, size_t len) {
	assert((len % 4) == 0);
	unsigned int i, j;

	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = (uint8_t)(input[i] & 0xff);
		output[j + 1] = (uint8_t)((input[i] >> 8) & 0xff);
		output[j + 2] = (uint8_t)((input[i] >> 16) & 0xff);
		output[j + 3] = (uint8_t)((input[i] >> 24) & 0xff);
	}
}

/// Buffer must be 32+1 (nul) = 33 chars long at least

static void md5_characterize(xpl_md5_context_t *self) {
	int pos;

	for (pos = 0; pos < 16; pos++) {
		sprintf(self->digest_chars + (pos * 2), "%02x", self->digest_raw[pos]);
	}
}




// MD5 initialization. Begins an MD5 operation, writing a new context.

xpl_md5_context_t *xpl_md5_new() {
	xpl_md5_context_t *context = xpl_alloc_type(xpl_md5_context_t);

	context->finished = 0;

	context->count[0] = 0;
	context->count[1] = 0;

	// Load magic initialization constants.
	context->state[0] = 0x67452301;
	context->state[1] = 0xefcdab89;
	context->state[2] = 0x98badcfe;
	context->state[3] = 0x10325476;

	memset(& context->digest_raw, 0, sizeof (context->digest_raw));
	memset(& context->digest_chars, 0, sizeof (context->digest_chars));

	return context;
}

void xpl_md5_destroy(xpl_md5_context_t **ppcontext) {
	xpl_md5_context_t *context = *ppcontext;

	// Take a little extra care with this
	memset(context, 0, sizeof (*context));

	xpl_free(context);
	*ppcontext = NULL;
}

// MD5 block update operation. Continues an MD5 message-digest
// operation, processing another message block, and updating the
// context.

void xpl_md5_update(xpl_md5_context_t *self,
					uint8_t *input, // input block
					size_t input_length) // length of input block
{
	if (self->finished) {
		LOG_ERROR("Cannot update a finished digest! Aborting.");
		return;
	}

	unsigned int i, index, part_length;

	// Compute number of bytes mod 64
	index = (unsigned int)((self->count[0] >> 3) & 0x3F);

	// Update number of bits
	if ((self->count[0] += ((uint32_t)input_length << 3)) < ((uint32_t)input_length << 3)) {
		self->count[1]++;
	}
	self->count[1] += ((uint32_t)input_length >> 29);

	part_length = 64 - index;

	// Transform as many times as possible.
	if (input_length >= part_length) {
		memcpy((POINTER) & self->buffer[index], (POINTER)input, part_length);
		md5_transform(self->state, self->buffer);

		for (i = part_length; i + 63 < input_length; i += 64) {
			md5_transform(self->state, &input[i]);
		}

		index = 0;
	} else {
		i = 0;
	}

	/* Buffer remaining input */
	memcpy((POINTER) & self->buffer[index], (POINTER) & input[i], input_length - i);
}

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
// Writes to digestRaw

void xpl_md5_finish(xpl_md5_context_t *self) {
	uint8_t bits[8];
	size_t index, padding_length;

	// Save number of bits
	md5_encode(bits, self->count, 8);

	// Pad out to 56 mod 64.
	index = (size_t)((self->count[0] >> 3) & 0x3f);
	padding_length = (index < 56) ? (56 - index) : (120 - index);
	xpl_md5_update(self, PADDING, padding_length);

	// Append length (before padding)
	xpl_md5_update(self, bits, 8);

	// Store state in digest
	md5_encode(self->digest_raw, self->state, 16);

	// Zeroize sensitive information.
	self->finished = 1;
	memset(&self->state[0], 0, sizeof (self->state));
	memset(&self->count[0], 0, sizeof (self->count));
	memset(&self->buffer[0], 0, sizeof (self->buffer));

	md5_characterize(self);
}


/// Load a file from disk and digest it
// Digests a file and returns the result.

void xpl_md5_digest_file(xpl_md5_context_t *self, const char *filename) {
	FILE *file;
	size_t len;
	uint8_t buffer[1024];

	if ((file = fopen(filename, "rb")) == NULL)
		printf("%s can't be opened\n", filename);
	else {
		while ((len = fread(buffer, 1, 1024, file)) > 0) {
			xpl_md5_update(self, buffer, len);
		}
		xpl_md5_finish(self);

		fclose(file);
	}
}

/// Digests a byte-array already in memory

void xpl_md5_digest_bytes(xpl_md5_context_t *self, uint8_t *bytes, size_t length) {
	xpl_md5_update(self, bytes, length);
}

// Digests a string and prints the result.

void xpl_md5_digest_string(xpl_md5_context_t *self, const char *string) {
	xpl_md5_update(self, (uint8_t *)string, strlen(string));
}

