//
//  xpl_mat4.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_mat4_h
#define p1_xpl_mat4_h

typedef union _xm4 {
	float data[16];
	float grid[4][4];
} xmat4;

#include "xpl_vec.h"

XPLINLINE xmat4 *xmat4_create(xmat4 *mat) {
	xmat4 *dest = xpl_alloc_type(xmat4);
	if (mat) {
		dest->data[0] = mat->data[0];
		dest->data[1] = mat->data[1];
		dest->data[2] = mat->data[2];
		dest->data[3] = mat->data[3];
		dest->data[4] = mat->data[4];
		dest->data[5] = mat->data[5];
		dest->data[6] = mat->data[6];
		dest->data[7] = mat->data[7];
		dest->data[8] = mat->data[8];
		dest->data[9] = mat->data[9];
		dest->data[10] = mat->data[10];
		dest->data[11] = mat->data[11];
		dest->data[12] = mat->data[12];
		dest->data[13] = mat->data[13];
		dest->data[14] = mat->data[14];
		dest->data[15] = mat->data[15];
	} else {
		dest->data[0] = 0;
		dest->data[1] = 0;
		dest->data[2] = 0;
		dest->data[3] = 0;
		dest->data[4] = 0;
		dest->data[5] = 0;
		dest->data[6] = 0;
		dest->data[7] = 0;
		dest->data[8] = 0;
		dest->data[9] = 0;
		dest->data[10] = 0;
		dest->data[11] = 0;
		dest->data[12] = 0;
		dest->data[13] = 0;
		dest->data[14] = 0;
		dest->data[15] = 0;
	}
    
	return dest;
}

XPLINLINE xmat4 *xmat4_set(const xmat4 *src, xmat4 *dest) {
	dest->data[0] = src->data[0];
	dest->data[1] = src->data[1];
	dest->data[2] = src->data[2];
	dest->data[3] = src->data[3];
	dest->data[4] = src->data[4];
	dest->data[5] = src->data[5];
	dest->data[6] = src->data[6];
	dest->data[7] = src->data[7];
	dest->data[8] = src->data[8];
	dest->data[9] = src->data[9];
	dest->data[10] = src->data[10];
	dest->data[11] = src->data[11];
	dest->data[12] = src->data[12];
	dest->data[13] = src->data[13];
	dest->data[14] = src->data[14];
	dest->data[15] = src->data[15];
	return dest;
}

XPLINLINE xmat4 *xmat4_identity(xmat4 *dest) {
	dest->data[0] = 1;
	dest->data[1] = 0;
	dest->data[2] = 0;
	dest->data[3] = 0;
	dest->data[4] = 0;
	dest->data[5] = 1;
	dest->data[6] = 0;
	dest->data[7] = 0;
	dest->data[8] = 0;
	dest->data[9] = 0;
	dest->data[10] = 1;
	dest->data[11] = 0;
	dest->data[12] = 0;
	dest->data[13] = 0;
	dest->data[14] = 0;
	dest->data[15] = 1;
	return dest;
}

XPLINLINE xmat4 *xmat4_transpose(xmat4 *src, xmat4 *dest) {
	// If we are transposing ourselves we can skip a few steps but have to cache some values
	if (!dest || src == dest) {
		float a01 = src->data[1],
        a02 = src->data[2],
        a03 = src->data[3],
        a12 = src->data[6],
        a13 = src->data[7],
        a23 = src->data[11];
        
		src->data[1] = src->data[4];
		src->data[2] = src->data[8];
		src->data[3] = src->data[12];
		src->data[4] = a01;
		src->data[6] = src->data[9];
		src->data[7] = src->data[13];
		src->data[8] = a02;
		src->data[9] = a12;
		src->data[11] = src->data[14];
		src->data[12] = a03;
		src->data[13] = a13;
		src->data[14] = a23;
		return src;
	}
    
	dest->data[0] = src->data[0];
	dest->data[1] = src->data[4];
	dest->data[2] = src->data[8];
	dest->data[3] = src->data[12];
	dest->data[4] = src->data[1];
	dest->data[5] = src->data[5];
	dest->data[6] = src->data[9];
	dest->data[7] = src->data[13];
	dest->data[8] = src->data[2];
	dest->data[9] = src->data[6];
	dest->data[10] = src->data[10];
	dest->data[11] = src->data[14];
	dest->data[12] = src->data[3];
	dest->data[13] = src->data[7];
	dest->data[14] = src->data[11];
	dest->data[15] = src->data[15];
	return dest;
}

XPLINLINE float xmat4_determinant(const xmat4 *mat) {
    
	float a00 = mat->data[0], a01 = mat->data[1], a02 = mat->data[2], a03 = mat->data[3],
    a10 = mat->data[4], a11 = mat->data[5], a12 = mat->data[6], a13 = mat->data[7],
    a20 = mat->data[8], a21 = mat->data[9], a22 = mat->data[10], a23 = mat->data[11],
    a30 = mat->data[12], a31 = mat->data[13], a32 = mat->data[14], a33 = mat->data[15];
    
	return (a30 * a21 * a12 * a03 - a20 * a31 * a12 * a03 - a30 * a11 * a22 * a03 + a10 * a31 * a22 * a03 +
			a20 * a11 * a32 * a03 - a10 * a21 * a32 * a03 - a30 * a21 * a02 * a13 + a20 * a31 * a02 * a13 +
			a30 * a01 * a22 * a13 - a00 * a31 * a22 * a13 - a20 * a01 * a32 * a13 + a00 * a21 * a32 * a13 +
			a30 * a11 * a02 * a23 - a10 * a31 * a02 * a23 - a30 * a01 * a12 * a23 + a00 * a31 * a12 * a23 +
			a10 * a01 * a32 * a23 - a00 * a11 * a32 * a23 - a20 * a11 * a02 * a33 + a10 * a21 * a02 * a33 +
			a20 * a01 * a12 * a33 - a00 * a21 * a12 * a33 - a10 * a01 * a22 * a33 + a00 * a11 * a22 * a33);
}

XPLINLINE xmat4 * xmat4_assign(xmat4 *mat, float a00, float a01, float a02, float a03, float a10, float a11, float a12, float a13, float a20, float a21, float a22, float a23, float a30, float a31, float a32, float a33) {
	if (mat == NULL) {
		mat = xmat4_create(NULL);
	}
	mat->data[0] = a00;
	mat->data[1] = a01;
	mat->data[2] = a02;
	mat->data[3] = a03;
	mat->data[4] = a10;
	mat->data[5] = a11;
	mat->data[6] = a12;
	mat->data[7] = a13;
	mat->data[8] = a20;
	mat->data[9] = a21;
	mat->data[10] = a22;
	mat->data[11] = a23;
	mat->data[12] = a30;
	mat->data[13] = a31;
	mat->data[14] = a32;
	mat->data[15] = a33;
	return mat;
}

XPLINLINE xmat4 *xmat4_inverse(xmat4 *src, xmat4 *dest) {
    float *m = src->data;
    float inv[16], det;
    int i;
    inv[0] = (m[5]  * m[10] * m[15] -
              m[5]  * m[11] * m[14] -
              m[9]  * m[6]  * m[15] +
              m[9]  * m[7]  * m[14] +
              m[13] * m[6]  * m[11] -
              m[13] * m[7]  * m[10]);
    
    inv[4] = (-m[4] * m[10] * m[15] +
              m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] -
              m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] +
              m[12] * m[7]  * m[10]);
    
    inv[8] = (m[4]  * m[9]  * m[15] -
              m[4]  * m[11] * m[13] -
              m[8]  * m[5]  * m[15] +
              m[8]  * m[7]  * m[13] +
              m[12] * m[5]  * m[11] -
              m[12] * m[7]  * m[9]);
    
    inv[12] = (-m[4]  * m[9]  * m[14] +
               m[4]   * m[10] * m[13] +
               m[8]   * m[5]  * m[14] -
               m[8]   * m[6]  * m[13] -
               m[12]  * m[5]  * m[10] +
               m[12]  * m[6]  * m[9]);
    
    inv[1] = (-m[1]  * m[10] * m[15] +
              m[1]  * m[11] * m[14] +
              m[9]  * m[2]  * m[15] -
              m[9]  * m[3]  * m[14] -
              m[13] * m[2]  * m[11] +
              m[13] * m[3]  * m[10]);
    
    inv[5] = (m[0]  * m[10] * m[15] -
              m[0]  * m[11] * m[14] -
              m[8]  * m[2]  * m[15] +
              m[8]  * m[3]  * m[14] +
              m[12] * m[2]  * m[11] -
              m[12] * m[3]  * m[10]);
    
    inv[9] = (-m[0]  * m[9]  * m[15] +
              m[0]   * m[11] * m[13] +
              m[8]   * m[1]  * m[15] -
              m[8]   * m[3]  * m[13] -
              m[12]  * m[1]  * m[11] +
              m[12]  * m[3]  * m[9]);
    
    inv[13] = (m[0]  * m[9]  * m[14] -
               m[0]  * m[10] * m[13] -
               m[8]  * m[1]  * m[14] +
               m[8]  * m[2]  * m[13] +
               m[12] * m[1]  * m[10] -
               m[12] * m[2]  * m[9]);
    
    inv[2] = (m[1]  * m[6] * m[15] -
              m[1]  * m[7] * m[14] -
              m[5]  * m[2] * m[15] +
              m[5]  * m[3] * m[14] +
              m[13] * m[2] * m[7] -
              m[13] * m[3] * m[6]);
    
    inv[6] = (-m[0] * m[6] * m[15] +
              m[0]  * m[7] * m[14] +
              m[4]  * m[2] * m[15] -
              m[4]  * m[3] * m[14] -
              m[12] * m[2] * m[7] +
              m[12] * m[3] * m[6]);
    
    inv[10] = (m[0]  * m[5] * m[15] -
               m[0]  * m[7] * m[13] -
               m[4]  * m[1] * m[15] +
               m[4]  * m[3] * m[13] +
               m[12] * m[1] * m[7] -
               m[12] * m[3] * m[5]);
    
    inv[14] = (-m[0]  * m[5] * m[14] +
               m[0]   * m[6] * m[13] +
               m[4]   * m[1] * m[14] -
               m[4]   * m[2] * m[13] -
               m[12]  * m[1] * m[6] +
               m[12]  * m[2] * m[5]);
    
    inv[3] = (-m[1] * m[6] * m[11] +
              m[1]  * m[7] * m[10] +
              m[5]  * m[2] * m[11] -
              m[5]  * m[3] * m[10] -
              m[9]  * m[2] * m[7] +
              m[9]  * m[3] * m[6]);
    
    inv[7] = (m[0] * m[6] * m[11] -
              m[0] * m[7] * m[10] -
              m[4] * m[2] * m[11] +
              m[4] * m[3] * m[10] +
              m[8] * m[2] * m[7] -
              m[8] * m[3] * m[6]);
    
    inv[11] = (-m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] +
               m[8] * m[3] * m[5]);
    
    inv[15] = (m[0] * m[5] * m[10] -
               m[0] * m[6] * m[9] -
               m[4] * m[1] * m[10] +
               m[4] * m[2] * m[9] +
               m[8] * m[1] * m[6] -
               m[8] * m[2] * m[5]);
    
    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    
    assert(det != 0.0f);
    
    det = 1.0 / det;
    
    for (i = 0; i < 16; i++)
        dest->data[i] = inv[i] * det;
    
	return dest;
}

XPLINLINE xmat4 *xmat4_to_rotation_xmat4(const xmat4 *src, xmat4 *dest) {
	if (!dest) {
		dest = xmat4_create(NULL);
	}
    
	dest->data[0] = src->data[0];
	dest->data[1] = src->data[1];
	dest->data[2] = src->data[2];
	dest->data[3] = src->data[3];
	dest->data[4] = src->data[4];
	dest->data[5] = src->data[5];
	dest->data[6] = src->data[6];
	dest->data[7] = src->data[7];
	dest->data[8] = src->data[8];
	dest->data[9] = src->data[9];
	dest->data[10] = src->data[10];
	dest->data[11] = src->data[11];
	dest->data[12] = 0;
	dest->data[13] = 0;
	dest->data[14] = 0;
	dest->data[15] = 1;
    
	return dest;
}

XPLINLINE xmat3 *xmat4_to_xmat3(const xmat4 *src, xmat3 *dest) {
	if (!dest) {
		dest = xmat3_create(NULL);
	}
    
	dest->data[0] = src->data[0];
	dest->data[1] = src->data[1];
	dest->data[2] = src->data[2];
	dest->data[3] = src->data[4];
	dest->data[4] = src->data[5];
	dest->data[5] = src->data[6];
	dest->data[6] = src->data[8];
	dest->data[7] = src->data[9];
	dest->data[8] = src->data[10];
    
	return dest;
}

XPLINLINE xmat3 *xmat4_to_inverse_xmat3(const xmat4 *src, xmat3 *dest) {
	// Cache the matrix values (makes for huge speed increases!)
	float a00 = src->data[0], a01 = src->data[1], a02 = src->data[2],
    a10 = src->data[4], a11 = src->data[5], a12 = src->data[6],
    a20 = src->data[8], a21 = src->data[9], a22 = src->data[10],
    
    b01 = a22 * a11 - a12 * a21,
    b11 = -a22 * a10 + a12 * a20,
    b21 = a21 * a10 - a11 * a20,
    
    d = a00 * b01 + a01 * b11 + a02 * b21,
    id;
    
	if (!d) {
		return NULL;
	}
	id = 1 / d;
    
	if (!dest) {
		dest = xmat3_create(NULL);
	}
    
	dest->data[0] = b01 * id;
	dest->data[1] = (-a22 * a01 + a02 * a21) * id;
	dest->data[2] = (a12 * a01 - a02 * a11) * id;
	dest->data[3] = b11 * id;
	dest->data[4] = (a22 * a00 - a02 * a20) * id;
	dest->data[5] = (-a12 * a00 + a02 * a10) * id;
	dest->data[6] = b21 * id;
	dest->data[7] = (-a21 * a00 + a01 * a20) * id;
	dest->data[8] = (a11 * a00 - a01 * a10) * id;
    
	return dest;
}

XPLINLINE xmat4 *xmat4_multiply(xmat4 *mat1, const xmat4 *mat2, xmat4 *dest) {
	if (!dest) {
		dest = mat1;
	}
    
	// Cache the matrix values
    // Required in case either mat1 or mat2 == dest
	float a00 = mat1->data[0], a01 = mat1->data[1], a02 = mat1->data[2], a03 = mat1->data[3],
    a10 = mat1->data[4], a11 = mat1->data[5], a12 = mat1->data[6], a13 = mat1->data[7],
    a20 = mat1->data[8], a21 = mat1->data[9], a22 = mat1->data[10], a23 = mat1->data[11],
    a30 = mat1->data[12], a31 = mat1->data[13], a32 = mat1->data[14], a33 = mat1->data[15];
    
	float b00 = mat2->data[0], b01 = mat2->data[1], b02 = mat2->data[2], b03 = mat2->data[3],
    b10 = mat2->data[4], b11 = mat2->data[5], b12 = mat2->data[6], b13 = mat2->data[7],
    b20 = mat2->data[8], b21 = mat2->data[9], b22 = mat2->data[10], b23 = mat2->data[11],
    b30 = mat2->data[12], b31 = mat2->data[13], b32 = mat2->data[14], b33 = mat2->data[15];
    
	dest->data[0] = b00 * a00 + b01 * a10 + b02 * a20 + b03 * a30;
	dest->data[1] = b00 * a01 + b01 * a11 + b02 * a21 + b03 * a31;
	dest->data[2] = b00 * a02 + b01 * a12 + b02 * a22 + b03 * a32;
	dest->data[3] = b00 * a03 + b01 * a13 + b02 * a23 + b03 * a33;
	dest->data[4] = b10 * a00 + b11 * a10 + b12 * a20 + b13 * a30;
	dest->data[5] = b10 * a01 + b11 * a11 + b12 * a21 + b13 * a31;
	dest->data[6] = b10 * a02 + b11 * a12 + b12 * a22 + b13 * a32;
	dest->data[7] = b10 * a03 + b11 * a13 + b12 * a23 + b13 * a33;
	dest->data[8] = b20 * a00 + b21 * a10 + b22 * a20 + b23 * a30;
	dest->data[9] = b20 * a01 + b21 * a11 + b22 * a21 + b23 * a31;
	dest->data[10] = b20 * a02 + b21 * a12 + b22 * a22 + b23 * a32;
	dest->data[11] = b20 * a03 + b21 * a13 + b22 * a23 + b23 * a33;
	dest->data[12] = b30 * a00 + b31 * a10 + b32 * a20 + b33 * a30;
	dest->data[13] = b30 * a01 + b31 * a11 + b32 * a21 + b33 * a31;
	dest->data[14] = b30 * a02 + b31 * a12 + b32 * a22 + b33 * a32;
	dest->data[15] = b30 * a03 + b31 * a13 + b32 * a23 + b33 * a33;
    
	return dest;
}

XPLINLINE xvec3 *xmat4_multiply_xvec3(const xmat4 *mat, xvec3 *vec, xvec3 *dest) {
	float *tgt;
	if (!dest) {
		tgt = &vec->data[0];
	} else {
		tgt = &dest->data[0];
	}
    
	float x = vec->x, y = vec->y, z = vec->z;
    
	tgt[0] = mat->data[0] * x + mat->data[4] * y + mat->data[8] * z + mat->data[12];
	tgt[1] = mat->data[1] * x + mat->data[5] * y + mat->data[9] * z + mat->data[13];
	tgt[2] = mat->data[2] * x + mat->data[6] * y + mat->data[10] * z + mat->data[14];
    
	return dest;
}

XPLINLINE xvec4 *xmat4_multiply_xvec4(const xmat4 *mat, xvec4 *vec, xvec4 *dest) {
	float *tgt;
	if (!dest) {
		tgt = &vec->data[0];
	} else {
		tgt = &dest->data[0];
	}
    
	float x = vec->data[0], y = vec->data[1], z = vec->data[2], w = vec->data[3];
    
	tgt[0] = mat->data[0] * x + mat->data[4] * y + mat->data[8] * z + mat->data[12] * w;
	tgt[1] = mat->data[1] * x + mat->data[5] * y + mat->data[9] * z + mat->data[13] * w;
	tgt[2] = mat->data[2] * x + mat->data[6] * y + mat->data[10] * z + mat->data[14] * w;
	tgt[3] = mat->data[3] * x + mat->data[7] * y + mat->data[11] * z + mat->data[15] * w;
    
	return dest;
}


XPLINLINE xmat4 *xmat4_translate(xmat4 *mat, xvec3 *vec, xmat4 *dest) {
	float x = vec->data[0], y = vec->data[1], z = vec->data[2],
    a00, a01, a02, a03,
    a10, a11, a12, a13,
    a20, a21, a22, a23;
    
	if (!dest || mat == dest) {
		mat->data[12] = mat->data[0] * x + mat->data[4] * y + mat->data[8] * z + mat->data[12];
		mat->data[13] = mat->data[1] * x + mat->data[5] * y + mat->data[9] * z + mat->data[13];
		mat->data[14] = mat->data[2] * x + mat->data[6] * y + mat->data[10] * z + mat->data[14];
		mat->data[15] = mat->data[3] * x + mat->data[7] * y + mat->data[11] * z + mat->data[15];
		return mat;
	}
    
	a00 = mat->data[0];
	a01 = mat->data[1];
	a02 = mat->data[2];
	a03 = mat->data[3];
	a10 = mat->data[4];
	a11 = mat->data[5];
	a12 = mat->data[6];
	a13 = mat->data[7];
	a20 = mat->data[8];
	a21 = mat->data[9];
	a22 = mat->data[10];
	a23 = mat->data[11];
    
	dest->data[0] = a00;
	dest->data[1] = a01;
	dest->data[2] = a02;
	dest->data[3] = a03;
	dest->data[4] = a10;
	dest->data[5] = a11;
	dest->data[6] = a12;
	dest->data[7] = a13;
	dest->data[8] = a20;
	dest->data[9] = a21;
	dest->data[10] = a22;
	dest->data[11] = a23;
    
	dest->data[12] = a00 * x + a10 * y + a20 * z + mat->data[12];
	dest->data[13] = a01 * x + a11 * y + a21 * z + mat->data[13];
	dest->data[14] = a02 * x + a12 * y + a22 * z + mat->data[14];
	dest->data[15] = a03 * x + a13 * y + a23 * z + mat->data[15];
	return dest;
}

XPLINLINE xmat4 *xmat4_scale(xmat4 *mat, xvec3 *vec, xmat4 *dest) {
	float x = vec->data[0], y = vec->data[1], z = vec->data[2];
    
	if (!dest || mat == dest) {
		mat->data[0] *= x;
		mat->data[1] *= x;
		mat->data[2] *= x;
		mat->data[3] *= x;
		mat->data[4] *= y;
		mat->data[5] *= y;
		mat->data[6] *= y;
		mat->data[7] *= y;
		mat->data[8] *= z;
		mat->data[9] *= z;
		mat->data[10] *= z;
		mat->data[11] *= z;
		return mat;
	}
    
	dest->data[0] = mat->data[0] * x;
	dest->data[1] = mat->data[1] * x;
	dest->data[2] = mat->data[2] * x;
	dest->data[3] = mat->data[3] * x;
	dest->data[4] = mat->data[4] * y;
	dest->data[5] = mat->data[5] * y;
	dest->data[6] = mat->data[6] * y;
	dest->data[7] = mat->data[7] * y;
	dest->data[8] = mat->data[8] * z;
	dest->data[9] = mat->data[9] * z;
	dest->data[10] = mat->data[10] * z;
	dest->data[11] = mat->data[11] * z;
	dest->data[12] = mat->data[12];
	dest->data[13] = mat->data[13];
	dest->data[14] = mat->data[14];
	dest->data[15] = mat->data[15];
	return dest;
}

XPLINLINE xmat4 *xmat4_rotate(xmat4 *mat, float angle, const xvec3 *axis, xmat4 *dest) {
	float x = axis->data[0], y = axis->data[1], z = axis->data[2],
    len = sqrtf(x * x + y * y + z * z),
    s, c, t,
    a00, a01, a02, a03,
    a10, a11, a12, a13,
    a20, a21, a22, a23,
    b00, b01, b02,
    b10, b11, b12,
    b20, b21, b22;
    
	if (!len) {
		return NULL;
	}
	if (len != 1) {
		len = 1 / len;
		x *= len;
		y *= len;
		z *= len;
	}
    
	s = sinf(angle);
	c = cosf(angle);
	t = 1 - c;
    
	a00 = mat->data[0];
	a01 = mat->data[1];
	a02 = mat->data[2];
	a03 = mat->data[3];
	a10 = mat->data[4];
	a11 = mat->data[5];
	a12 = mat->data[6];
	a13 = mat->data[7];
	a20 = mat->data[8];
	a21 = mat->data[9];
	a22 = mat->data[10];
	a23 = mat->data[11];
    
	// Construct the elements of the rotation matrix
	b00 = x * x * t + c;
	b01 = y * x * t + z * s;
	b02 = z * x * t - y * s;
	b10 = x * y * t - z * s;
	b11 = y * y * t + c;
	b12 = z * y * t + x * s;
	b20 = x * z * t + y * s;
	b21 = y * z * t - x * s;
	b22 = z * z * t + c;
    
	if (!dest) {
		dest = mat;
	} else if (mat != dest) { // If the source and destination differ, copy the unchanged last row
		dest->data[12] = mat->data[12];
		dest->data[13] = mat->data[13];
		dest->data[14] = mat->data[14];
		dest->data[15] = mat->data[15];
	}
    
	// Perform rotation-specific matrix multiplication
	dest->data[0] = a00 * b00 + a10 * b01 + a20 * b02;
	dest->data[1] = a01 * b00 + a11 * b01 + a21 * b02;
	dest->data[2] = a02 * b00 + a12 * b01 + a22 * b02;
	dest->data[3] = a03 * b00 + a13 * b01 + a23 * b02;
    
	dest->data[4] = a00 * b10 + a10 * b11 + a20 * b12;
	dest->data[5] = a01 * b10 + a11 * b11 + a21 * b12;
	dest->data[6] = a02 * b10 + a12 * b11 + a22 * b12;
	dest->data[7] = a03 * b10 + a13 * b11 + a23 * b12;
    
	dest->data[8] = a00 * b20 + a10 * b21 + a20 * b22;
	dest->data[9] = a01 * b20 + a11 * b21 + a21 * b22;
	dest->data[10] = a02 * b20 + a12 * b21 + a22 * b22;
	dest->data[11] = a03 * b20 + a13 * b21 + a23 * b22;
	return dest;
}

XPLINLINE xmat4 *xmat4_rotate_x(xmat4 *mat, float angle, xmat4 *dest) {
	float s = sinf(angle),
    c = cosf(angle),
    a10 = mat->data[4],
    a11 = mat->data[5],
    a12 = mat->data[6],
    a13 = mat->data[7],
    a20 = mat->data[8],
    a21 = mat->data[9],
    a22 = mat->data[10],
    a23 = mat->data[11];
    
	if (!dest) {
		dest = mat;
	} else if (mat != dest) { // If the source and destination differ, copy the unchanged rows
		dest->data[0] = mat->data[0];
		dest->data[1] = mat->data[1];
		dest->data[2] = mat->data[2];
		dest->data[3] = mat->data[3];
        
		dest->data[12] = mat->data[12];
		dest->data[13] = mat->data[13];
		dest->data[14] = mat->data[14];
		dest->data[15] = mat->data[15];
	}
    
	// Perform axis-specific matrix multiplication
	dest->data[4] = a10 * c + a20 * s;
	dest->data[5] = a11 * c + a21 * s;
	dest->data[6] = a12 * c + a22 * s;
	dest->data[7] = a13 * c + a23 * s;
    
	dest->data[8] = a10 * -s + a20 * c;
	dest->data[9] = a11 * -s + a21 * c;
	dest->data[10] = a12 * -s + a22 * c;
	dest->data[11] = a13 * -s + a23 * c;
	return dest;
}

XPLINLINE xmat4 *xmat4_rotate_y(xmat4 *mat, float angle, xmat4 *dest) {
	float s = sinf(angle),
    c = cosf(angle),
    a00 = mat->data[0],
    a01 = mat->data[1],
    a02 = mat->data[2],
    a03 = mat->data[3],
    a20 = mat->data[8],
    a21 = mat->data[9],
    a22 = mat->data[10],
    a23 = mat->data[11];
    
	if (!dest) {
		dest = mat;
	} else if (mat != dest) { // If the source and destination differ, copy the unchanged rows
		dest->data[4] = mat->data[4];
		dest->data[5] = mat->data[5];
		dest->data[6] = mat->data[6];
		dest->data[7] = mat->data[7];
        
		dest->data[12] = mat->data[12];
		dest->data[13] = mat->data[13];
		dest->data[14] = mat->data[14];
		dest->data[15] = mat->data[15];
	}
    
	// Perform axis-specific matrix multiplication
	dest->data[0] = a00 * c + a20 * -s;
	dest->data[1] = a01 * c + a21 * -s;
	dest->data[2] = a02 * c + a22 * -s;
	dest->data[3] = a03 * c + a23 * -s;
    
	dest->data[8] = a00 * s + a20 * c;
	dest->data[9] = a01 * s + a21 * c;
	dest->data[10] = a02 * s + a22 * c;
	dest->data[11] = a03 * s + a23 * c;
	return dest;
}

XPLINLINE xmat4 *xmat4_rotate_z(xmat4 *mat, float angle, xmat4 *dest) {
	float s = sinf(angle),
    c = cosf(angle),
    a00 = mat->data[0],
    a01 = mat->data[1],
    a02 = mat->data[2],
    a03 = mat->data[3],
    a10 = mat->data[4],
    a11 = mat->data[5],
    a12 = mat->data[6],
    a13 = mat->data[7];
    
	if (!dest) {
		dest = mat;
	} else if (mat != dest) { // If the source and destination differ, copy the unchanged last row
		dest->data[8] = mat->data[8];
		dest->data[9] = mat->data[9];
		dest->data[10] = mat->data[10];
		dest->data[11] = mat->data[11];
        
		dest->data[12] = mat->data[12];
		dest->data[13] = mat->data[13];
		dest->data[14] = mat->data[14];
		dest->data[15] = mat->data[15];
	}
    
	// Perform axis-specific matrix multiplication
	dest->data[0] = a00 * c + a10 * s;
	dest->data[1] = a01 * c + a11 * s;
	dest->data[2] = a02 * c + a12 * s;
	dest->data[3] = a03 * c + a13 * s;
    
	dest->data[4] = a00 * -s + a10 * c;
	dest->data[5] = a01 * -s + a11 * c;
	dest->data[6] = a02 * -s + a12 * c;
	dest->data[7] = a03 * -s + a13 * c;
    
	return dest;
}

XPLINLINE xmat4 *xmat4_frustum(float left, float right, float bottom, float top, float fnear, float ffar, xmat4 *dest) {
	if (!dest) {
		dest = xmat4_create(NULL);
	}
	float rl = (right - left),
    tb = (top - bottom),
    fn = (ffar - fnear);
	dest->data[0] = (fnear * 2) / rl;
	dest->data[1] = 0;
	dest->data[2] = 0;
	dest->data[3] = 0;
	dest->data[4] = 0;
	dest->data[5] = (fnear * 2) / tb;
	dest->data[6] = 0;
	dest->data[7] = 0;
	dest->data[8] = (right + left) / rl;
	dest->data[9] = (top + bottom) / tb;
	dest->data[10] = -(ffar + fnear) / fn;
	dest->data[11] = -1;
	dest->data[12] = 0;
	dest->data[13] = 0;
	dest->data[14] = -(ffar * fnear * 2) / fn;
	dest->data[15] = 0;
	return dest;
}

XPLINLINE xmat4 *xmat4_perspective(float fovy, float aspect, float fnear, float ffar, xmat4 *dest) {
	float top = fnear * tanf(fovy * M_PI / 360.0),
    right = top * aspect;
	return xmat4_frustum(-right, right, -top, top, fnear, ffar, dest);
}

XPLINLINE xmat4 *xmat4_ortho(float left, float right, float bottom, float top, float fnear, float ffar, xmat4 *dest) {
	if (!dest) {
		dest = xmat4_create(NULL);
	}
	float rl = (right - left),
    tb = (top - bottom),
    fn = (ffar - fnear);
	dest->data[0] = 2 / rl;
	dest->data[1] = 0;
	dest->data[2] = 0;
	dest->data[3] = 0;
	dest->data[4] = 0;
	dest->data[5] = 2 / tb;
	dest->data[6] = 0;
	dest->data[7] = 0;
	dest->data[8] = 0;
	dest->data[9] = 0;
	dest->data[10] = -2 / fn;
	dest->data[11] = 0;
	dest->data[12] = -(left + right) / rl;
	dest->data[13] = -(top + bottom) / tb;
	dest->data[14] = -(ffar + fnear) / fn;
	dest->data[15] = 1;
	return dest;
}

XPLINLINE xmat4 *xmat4_look_at(const xvec3 *eye, const xvec3 *center, const xvec3 *up, xmat4 *dest) {
	if (!dest) {
		dest = xmat4_create(NULL);
	}
    
	float x0, x1, x2, y0, y1, y2, z0, z1, z2, len,
    eyex = eye->data[0],
    eyey = eye->data[1],
    eyez = eye->data[2],
    upx = up->data[0],
    upy = up->data[1],
    upz = up->data[2],
    centerx = center->data[0],
    centery = center->data[1],
    centerz = center->data[2];
    
	if (eyex == centerx && eyey == centery && eyez == centerz) {
		return xmat4_identity(dest);
	}
    
	//vec3.direction(eye, center, z);
	z0 = eyex - centerx;
	z1 = eyey - centery;
	z2 = eyez - centerz;
    
	// normalize (no check needed for 0 because of early return)
	len = 1 / sqrtf(z0 * z0 + z1 * z1 + z2 * z2);
	z0 *= len;
	z1 *= len;
	z2 *= len;
    
	//vec3.normalize(vec3.cross(up, z, x));
	x0 = upy * z2 - upz * z1;
	x1 = upz * z0 - upx * z2;
	x2 = upx * z1 - upy * z0;
	len = sqrtf(x0 * x0 + x1 * x1 + x2 * x2);
	if (!len) {
		x0 = 0;
		x1 = 0;
		x2 = 0;
	} else {
		len = 1 / len;
		x0 *= len;
		x1 *= len;
		x2 *= len;
	}
    
	//vec3.normalize(vec3.cross(z, x, y));
	y0 = z1 * x2 - z2 * x1;
	y1 = z2 * x0 - z0 * x2;
	y2 = z0 * x1 - z1 * x0;
    
	len = sqrtf(y0 * y0 + y1 * y1 + y2 * y2);
	if (!len) {
		y0 = 0;
		y1 = 0;
		y2 = 0;
	} else {
		len = 1 / len;
		y0 *= len;
		y1 *= len;
		y2 *= len;
	}
    
	dest->data[0] = x0;
	dest->data[1] = y0;
	dest->data[2] = z0;
	dest->data[3] = 0;
	dest->data[4] = x1;
	dest->data[5] = y1;
	dest->data[6] = z1;
	dest->data[7] = 0;
	dest->data[8] = x2;
	dest->data[9] = y2;
	dest->data[10] = z2;
	dest->data[11] = 0;
	dest->data[12] = -(x0 * eyex + x1 * eyey + x2 * eyez);
	dest->data[13] = -(y0 * eyex + y1 * eyey + y2 * eyez);
	dest->data[14] = -(z0 * eyex + z1 * eyey + z2 * eyez);
	dest->data[15] = 1;
    
	return dest;
}

XPLINLINE xmat4 *xmat4_from_rotate_translate(const xquat *quat, const xvec3 *vec, xmat4 *dest) {
	if (!dest) {
		dest = xmat4_create(NULL);
	}
    
	// Quaternion math
	float x = quat->data[0], y = quat->data[1], z = quat->data[2], w = quat->data[3],
    x2 = x + x,
    y2 = y + y,
    z2 = z + z,
    
    xx = x * x2,
    xy = x * y2,
    xz = x * z2,
    yy = y * y2,
    yz = y * z2,
    zz = z * z2,
    wx = w * x2,
    wy = w * y2,
    wz = w * z2;
    
	dest->data[0] = 1 - (yy + zz);
	dest->data[1] = xy + wz;
	dest->data[2] = xz - wy;
	dest->data[3] = 0;
	dest->data[4] = xy - wz;
	dest->data[5] = 1 - (xx + zz);
	dest->data[6] = yz + wx;
	dest->data[7] = 0;
	dest->data[8] = xz + wy;
	dest->data[9] = yz - wx;
	dest->data[10] = 1 - (xx + yy);
	dest->data[11] = 0;
	dest->data[12] = vec->data[0];
	dest->data[13] = vec->data[1];
	dest->data[14] = vec->data[2];
	dest->data[15] = 1;
    
	return dest;
}


XPLINLINE xvec4 xvec4_transform(const xmat4 *m, xvec4 v) {
	xvec4 r;
	xmat4_multiply_xvec4(m, &v, &r);
	return r;
}


XPLINLINE xmat4 *xquat_to_xmat4(const xquat *quat, xmat4 *dest) {
	if (!dest) {
		dest = xmat4_create(NULL);
	}
    
	float x = quat->data[0], y = quat->data[1], z = quat->data[2], w = quat->data[3],
    x2 = x + x,
    y2 = y + y,
    z2 = z + z,
    
    xx = x * x2,
    xy = x * y2,
    xz = x * z2,
    yy = y * y2,
    yz = y * z2,
    zz = z * z2,
    wx = w * x2,
    wy = w * y2,
    wz = w * z2;
    
	dest->data[0] = 1 - (yy + zz);
	dest->data[1] = xy + wz;
	dest->data[2] = xz - wy;
	dest->data[3] = 0;
    
	dest->data[4] = xy - wz;
	dest->data[5] = 1 - (xx + zz);
	dest->data[6] = yz + wx;
	dest->data[7] = 0;
    
	dest->data[8] = xz + wy;
	dest->data[9] = yz - wx;
	dest->data[10] = 1 - (xx + yy);
	dest->data[11] = 0;
    
	dest->data[12] = 0;
	dest->data[13] = 0;
	dest->data[14] = 0;
	dest->data[15] = 1;
    
	return dest;
}

XPLINLINE xmat4 *xmat4_set_rotation_quat(xmat4 *mat, const xquat *quat, xmat4 *dest) {
    if (! dest) {
        dest = mat;
    }
    static xmat4 scratch;
    xquat_to_xmat4(quat, &scratch);
    xmat4_multiply(&scratch, mat, dest);
    return dest;
}

XPLINLINE xmat4 *xmat3_to_xmat4(xmat3 *mat, xmat4 *dest) {
	if (!dest) {
		dest = xmat4_create(NULL);
	}
    
	dest->data[15] = 1;
	dest->data[14] = 0;
	dest->data[13] = 0;
	dest->data[12] = 0;
    
	dest->data[11] = 0;
	dest->data[10] = mat->data[8];
	dest->data[9] = mat->data[7];
	dest->data[8] = mat->data[6];
    
	dest->data[7] = 0;
	dest->data[6] = mat->data[5];
	dest->data[5] = mat->data[4];
	dest->data[4] = mat->data[3];
    
	dest->data[3] = 0;
	dest->data[2] = mat->data[2];
	dest->data[1] = mat->data[1];
	dest->data[0] = mat->data[0];
    
	return dest;
}

XPLINLINE char * xmat4_str(xmat4 *mat, char *buffer, size_t buffer_size) {
	snprintf(buffer, buffer_size, "[%f, %f, %f, %f, \n %f, %f, %f, %f, \n %f, %f, %f, %f, \n %f, %f, %f, %f]",
             mat->data[0], mat->data[1], mat->data[2], mat->data[3],
             mat->data[4], mat->data[5], mat->data[6], mat->data[7],
             mat->data[8], mat->data[9], mat->data[10], mat->data[11],
             mat->data[12], mat->data[13], mat->data[14], mat->data[15]);
    return buffer;
}


#endif
