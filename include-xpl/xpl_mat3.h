//
//  xpl_mat3.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_mat3_h
#define p1_xpl_mat3_h

typedef union _xm3 {
	float data[9];
    float grid[3][3];
} xmat3;

#include "xpl_vec.h"

XPLINLINE xmat3 *xmat3_create(const xmat3 *copy) {
	xmat3 *dest = xpl_alloc_type(xmat3);
    
	if (copy) {
		dest->data[0] = copy->data[0];
		dest->data[1] = copy->data[1];
		dest->data[2] = copy->data[2];
		dest->data[3] = copy->data[3];
		dest->data[4] = copy->data[4];
		dest->data[5] = copy->data[5];
		dest->data[6] = copy->data[6];
		dest->data[7] = copy->data[7];
		dest->data[8] = copy->data[8];
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
	}
    
	return dest;
}

XPLINLINE xmat3 *xmat3_set(const xmat3 *src, xmat3 *dest) {
	dest->data[0] = src->data[0];
	dest->data[1] = src->data[1];
	dest->data[2] = src->data[2];
	dest->data[3] = src->data[3];
	dest->data[4] = src->data[4];
	dest->data[5] = src->data[5];
	dest->data[6] = src->data[6];
	dest->data[7] = src->data[7];
	dest->data[8] = src->data[8];
	return dest;
}

XPLINLINE xmat3 *xmat3_identity(xmat3 *dest) {
	if (!dest) {
		dest = xmat3_create(NULL);
	}
	dest->data[0] = 1;
	dest->data[1] = 0;
	dest->data[2] = 0;
	dest->data[3] = 0;
	dest->data[4] = 1;
	dest->data[5] = 0;
	dest->data[6] = 0;
	dest->data[7] = 0;
	dest->data[8] = 1;
	return dest;
}

XPLINLINE xmat3 *xmat3_transpose(xmat3 *mat, xmat3 *dest) {
	// If we are transposing ourselves we can skip a few steps but have to cache some values
	if (!dest || mat == dest) {
		float a01 = mat->data[1], a02 = mat->data[2],
        a12 = mat->data[5];
        
		mat->data[1] = mat->data[3];
		mat->data[2] = mat->data[6];
		mat->data[3] = a01;
		mat->data[5] = mat->data[7];
		mat->data[6] = a02;
		mat->data[7] = a12;
		return mat;
	}
    
	dest->data[0] = mat->data[0];
	dest->data[1] = mat->data[3];
	dest->data[2] = mat->data[6];
	dest->data[3] = mat->data[1];
	dest->data[4] = mat->data[4];
	dest->data[5] = mat->data[7];
	dest->data[6] = mat->data[2];
	dest->data[7] = mat->data[5];
	dest->data[8] = mat->data[8];
	return dest;
}


XPLINLINE xmat3 *xquat_to_xmat3(xquat *quat, xmat3 *dest) {
	if (!dest) {
		dest = xmat3_create(NULL);
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
    
	dest->data[3] = xy - wz;
	dest->data[4] = 1 - (xx + zz);
	dest->data[5] = yz + wx;
    
	dest->data[6] = xz + wy;
	dest->data[7] = yz - wx;
	dest->data[8] = 1 - (xx + yy);
    
	return dest;
}

XPLINLINE xvec3 *xmat3_multiply_xvec3(const xmat3 *mat, xvec3 *vec, xvec3 *dest) {
	float *tgt;
	if (! dest) {
		tgt = vec->data;
	} else {
		tgt = dest->data;
	}
	
	float x = vec->x, y = vec->y, z = vec->z;
	tgt[0] = mat->data[0] * x + mat->data[3] * y + mat->data[6] * z;
	tgt[1] = mat->data[1] * x + mat->data[4] * y + mat->data[7] * z;
	tgt[2] = mat->data[2] * x + mat->data[5] * y + mat->data[8] * z;
	
	return dest;
}

XPLINLINE xvec3 xmat3_transform(const xmat3 *m, xvec3 v) {
	xvec3 r;
	xmat3_multiply_xvec3(m, &v, &r);
	return r;
}

XPLINLINE char * xmat3_str(xmat3 *mat, char *buffer, size_t buffer_size) {
	snprintf(buffer, buffer_size, "[%f, %f, %f, \n %f, %f, %f, \n %f, %f, %f]", mat->data[0], mat->data[1], mat->data[2], mat->data[3], mat->data[4], mat->data[5], mat->data[6], mat->data[7], mat->data[8]);
    return buffer;
}


#endif
