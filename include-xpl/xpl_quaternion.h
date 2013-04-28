//
//  xpl_quaternion.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_quaternion_h
#define p1_xpl_quaternion_h

typedef union _xq {
	float data[4];
	struct {
		float x;
		float y;
		float z;
		float w;
	};
} xquat;

#include "xpl_vec.h"

XPLINLINE xquat *xquat_create(xquat *quat) {
	xquat *dest = xpl_alloc_type(xquat);
    
	if (quat) {
		dest->data[0] = quat->data[0];
		dest->data[1] = quat->data[1];
		dest->data[2] = quat->data[2];
		dest->data[3] = quat->data[3];
	} else {
		dest->data[0] = 0;
		dest->data[1] = 0;
		dest->data[2] = 0;
		dest->data[3] = 0;
	}
    
	return dest;
}

XPLINLINE xquat *xquat_assign(xquat *quat, float x, float y, float z, float w) {
	if (quat == NULL) {
		quat = xquat_create(NULL);
	}
	quat->x = x;
	quat->y = y;
	quat->z = z;
	quat->w = w;
	return quat;
}

XPLINLINE xquat *xquat_set(xquat *quat, xquat *dest) {
	dest->data[0] = quat->data[0];
	dest->data[1] = quat->data[1];
	dest->data[2] = quat->data[2];
	dest->data[3] = quat->data[3];
    
	return dest;
}

XPLINLINE xquat *xquat_from_axis_angle_rad(const xvec3 axis, float angle, xquat *dest) {
	if (dest == NULL) {
		dest = xquat_create(NULL);
	}
    
	xvec3 normalized_axis = xvec3_normalize(axis);
	float sinHalfTheta = sinf(angle * 0.5f);
    
	dest->x = normalized_axis.x * sinHalfTheta;
	dest->y = normalized_axis.y * sinHalfTheta;
	dest->z = normalized_axis.z * sinHalfTheta;
	dest->w = cosf(angle * 0.5f);
    
	return dest;
}

XPLINLINE xquat *xquat_calculate_w(xquat *quat, xquat *dest) {
	float x = quat->data[0], y = quat->data[1], z = quat->data[2];
    
	if (!dest || quat == dest) {
		quat->data[3] = -sqrtf(fabs(1.0 - x * x - y * y - z * z));
		return quat;
	}
	dest->data[0] = x;
	dest->data[1] = y;
	dest->data[2] = z;
	dest->data[3] = -sqrtf(fabs(1.0 - x * x - y * y - z * z));
	return dest;
}

XPLINLINE float xquat_dot(xquat *quat, xquat *quat2) {
	return quat->data[0] * quat2->data[0] +
    quat->data[1] * quat2->data[1] +
    quat->data[2] * quat2->data[2] +
    quat->data[3] * quat2->data[3];
}

XPLINLINE xquat *xquat_inverse(xquat *quat, xquat *dest) {
	float dot = xquat_dot(quat, quat),
    invDot = 1.0 / dot;
	if (!dest || quat == dest) {
		quat->data[0] *= -invDot;
		quat->data[1] *= -invDot;
		quat->data[2] *= -invDot;
		quat->data[3] *= invDot;
		return quat;
	}
	dest->data[0] = -quat->data[0] * invDot;
	dest->data[1] = -quat->data[1] * invDot;
	dest->data[2] = -quat->data[2] * invDot;
	dest->data[3] = quat->data[3] * invDot;
	return dest;
}

XPLINLINE xquat *xquat_conjugate(xquat *quat, xquat *dest) {
	if (!dest || quat == dest) {
		quat->data[0] *= -1;
		quat->data[1] *= -1;
		quat->data[2] *= -1;
		return quat;
	}
	dest->data[0] = -quat->data[0];
	dest->data[1] = -quat->data[1];
	dest->data[2] = -quat->data[2];
	dest->data[3] = quat->data[3];
	return dest;
}

XPLINLINE float xquat_length(xquat *quat) {
	float x = quat->data[0], y = quat->data[1], z = quat->data[2], w = quat->data[3];
	return sqrtf(x * x + y * y + z * z + w * w);
}

XPLINLINE xquat *xquat_normalize(xquat *quat, xquat *dest) {
	if (!dest) {
		dest = quat;
	}
    
	float x = quat->data[0], y = quat->data[1], z = quat->data[2], w = quat->data[3],
    len = sqrtf(x * x + y * y + z * z + w * w);
	if (len == 0) {
		dest->data[0] = 0;
		dest->data[1] = 0;
		dest->data[2] = 0;
		dest->data[3] = 0;
		return dest;
	}
	len = 1 / len;
	dest->data[0] = x * len;
	dest->data[1] = y * len;
	dest->data[2] = z * len;
	dest->data[3] = w * len;
    
	return dest;
}

XPLINLINE xquat *xquat_multiply(xquat *quat, xquat *quat2, xquat *dest) {
	if (!dest) {
		dest = quat;
	}
    
	float qax = quat->data[0], qay = quat->data[1], qaz = quat->data[2], qaw = quat->data[3],
    qbx = quat2->data[0], qby = quat2->data[1], qbz = quat2->data[2], qbw = quat2->data[3];
    
	dest->data[0] = qax * qbw + qaw * qbx + qay * qbz - qaz * qby;
	dest->data[1] = qay * qbw + qaw * qby + qaz * qbx - qax * qbz;
	dest->data[2] = qaz * qbw + qaw * qbz + qax * qby - qay * qbx;
	dest->data[3] = qaw * qbw - qax * qbx - qay * qby - qaz * qbz;
    
	return dest;
}

XPLINLINE xvec3 *xquat_multiply_vec3(xquat *quat, xvec3 *vec, xvec3 *dest) {
	if (!dest) {
		dest = vec;
	}
    
	float x = vec->data[0], y = vec->data[1], z = vec->data[2],
    qx = quat->data[0], qy = quat->data[1], qz = quat->data[2], qw = quat->data[3],
    
    // calculate quat * vec
    ix = qw * x + qy * z - qz * y,
    iy = qw * y + qz * x - qx * z,
    iz = qw * z + qx * y - qy * x,
    iw = -qx * x - qy * y - qz * z;
    
	// calculate result * inverse quat
	dest->data[0] = ix * qw + iw * -qx + iy * -qz - iz * -qy;
	dest->data[1] = iy * qw + iw * -qy + iz * -qx - ix * -qz;
	dest->data[2] = iz * qw + iw * -qz + ix * -qy - iy * -qx;
    
	return dest;
}

XPLINLINE xquat *xquat_slerp_longpath(xquat *quat, xquat *quat2, float slerp, xquat *dest) {
	float cosHalfTheta = quat->data[0] * quat2->data[0] +
						 quat->data[1] * quat2->data[1] +
						 quat->data[2] * quat2->data[2] +
						 quat->data[3] * quat2->data[3];
	float ratioA, ratioB, adj;

	if (cosHalfTheta > -0.95f && cosHalfTheta < 0.95f) {
		float angle = acosf(cosHalfTheta);
	    ratioA = sinf(angle * (1.0f - slerp));
	    ratioB = sinf(angle * slerp);
	    adj = 1.0f / sinf(angle);
	} else {
		// lerp
		ratioA = (1.0f - slerp);
		ratioB = slerp;
		adj = 1.0f;
	}

	dest->data[0] = (quat->data[0] * ratioA + quat2->data[0] * ratioB) * adj;
	dest->data[1] = (quat->data[1] * ratioA + quat2->data[1] * ratioB) * adj;
	dest->data[2] = (quat->data[2] * ratioA + quat2->data[2] * ratioB) * adj;
	dest->data[3] = (quat->data[3] * ratioA + quat2->data[3] * ratioB) * adj;

	return dest;
}

XPLINLINE xquat *xquat_slerp(xquat *quat, xquat *quat2, float slerp, xquat *dest) {
	float cosHalfTheta = quat->data[0] * quat2->data[0] +
						 quat->data[1] * quat2->data[1] +
						 quat->data[2] * quat2->data[2] +
						 quat->data[3] * quat2->data[3];
	float ratioA, ratioB, adj;

	if (cosHalfTheta < 0.f) {
		cosHalfTheta = -cosHalfTheta;
		xquat invert = *quat2;
		invert.x = -invert.x;
		invert.y = -invert.y;
		invert.z = -invert.z;
		invert.w = -invert.w;
		return xquat_slerp(quat, &invert, slerp, dest);
	}

	if (cosHalfTheta < 0.95f) {
		float angle = acosf(cosHalfTheta);
	    ratioA = sinf(angle * (1.0f - slerp));
	    ratioB = sinf(angle * slerp);
	    adj = 1.0f / sinf(angle);
	} else {
		ratioA = 1.0f - slerp;
		ratioB = slerp;
		adj = 1.0f;
	}


	dest->data[0] = (quat->data[0] * ratioA + quat2->data[0] * ratioB) * adj;
	dest->data[1] = (quat->data[1] * ratioA + quat2->data[1] * ratioB) * adj;
	dest->data[2] = (quat->data[2] * ratioA + quat2->data[2] * ratioB) * adj;
	dest->data[3] = (quat->data[3] * ratioA + quat2->data[3] * ratioB) * adj;

	return dest;
}

XPLINLINE xquat *xquat_slerp_mgbaker(xquat *quat, xquat *quat2, float slerp, xquat *dest) {
	if (!dest) {
		dest = quat;
	}
    
	float cosHalfTheta = quat->data[0] * quat2->data[0] +
						 quat->data[1] * quat2->data[1] +
						 quat->data[2] * quat2->data[2] +
						 quat->data[3] * quat2->data[3],
		  halfTheta,
		  sinHalfTheta,
		  ratioA,
		  ratioB;
    
	if (fabs(cosHalfTheta) >= 1.0) {
		if (dest != quat) {
			dest->data[0] = quat->data[0];
			dest->data[1] = quat->data[1];
			dest->data[2] = quat->data[2];
			dest->data[3] = quat->data[3];
		}
		return dest;
	}
    
	halfTheta = acosf(cosHalfTheta);
	sinHalfTheta = sqrtf(1.0f - cosHalfTheta * cosHalfTheta);
    
	if (fabs(sinHalfTheta) < 0.001) {
		dest->data[0] = (quat->data[0] * 0.5 + quat2->data[0] * 0.5);
		dest->data[1] = (quat->data[1] * 0.5 + quat2->data[1] * 0.5);
		dest->data[2] = (quat->data[2] * 0.5 + quat2->data[2] * 0.5);
		dest->data[3] = (quat->data[3] * 0.5 + quat2->data[3] * 0.5);
		return dest;
	}
    
	ratioA = sinf((1.0f - slerp) * halfTheta) / sinHalfTheta;
	ratioB = sinf(slerp * halfTheta) / sinHalfTheta;
    
	dest->data[0] = (quat->data[0] * ratioA + quat2->data[0] * ratioB);
	dest->data[1] = (quat->data[1] * ratioA + quat2->data[1] * ratioB);
	dest->data[2] = (quat->data[2] * ratioA + quat2->data[2] * ratioB);
	dest->data[3] = (quat->data[3] * ratioA + quat2->data[3] * ratioB);
    
	return dest;
}

XPLINLINE char * xquat_str(xquat *quat, char *buffer, size_t buffer_size) {
	snprintf(buffer, buffer_size, "[%f, %f, %f, %f]", quat->data[0], quat->data[1], quat->data[2], quat->data[3]);
    return buffer;
}


#endif
