//
//  xpl_sphere.h
//  p1
//
//  Created by Justin Bowes on 2013-02-27.
//  Copyright (c) 2013 Informi Software Inc. All rights reserved.
//

#ifndef p1_xpl_sphere_h
#define p1_xpl_sphere_h

#include <stdbool.h>
#include <float.h>

#include "xpl_vec3.h"

typedef union _xsph {
    
    struct {
        xvec3 center;
        float radius;
    };
    float data[4];
    
} xpl_sphere;

XPLINLINE xpl_sphere xpl_sphere_set(xvec3 origin, float radius) {
    xpl_sphere r;
    r.center = origin;
    r.radius = radius;
    return r;
}

XPLINLINE xpl_sphere xpl_sphere_origin(float radius) {
    return xpl_sphere_set(xvec3_all(0.0f), radius);
}

XPLINLINE xpl_sphere xpl_sphere_origin_unit() {
    return xpl_sphere_origin(1.0f);
}

XPLINLINE bool xpl_sphere_intersect_sphere(const xpl_sphere s1, const xpl_sphere s2) {
    xvec3 diff = xvec3_sub(s1.center, s2.center);
    float d_sq = xvec3_length_sq(diff);

    float r_sum_sq = s1.radius + s2.radius;
    r_sum_sq *= r_sum_sq;
    
    return (d_sq <= r_sum_sq);
}

XPLINLINE bool xpl_sphere_point_inside(const xpl_sphere s, const xvec3 p, float *penetration_depth) {
    float d_sq = xvec3_length_sq(xvec3_sub(s.center, p));
    float p_depth_sq = ((s.radius * s.radius) - d_sq);
    if (p_depth_sq > 0.0f) {
        if (*penetration_depth) *penetration_depth = sqrtf(p_depth_sq);
        return true;
    }
    return false;
}

XPLINLINE bool xpl_sphere_intersect_line(const xpl_sphere s,
										 const xvec3 point_0, const xvec3 point_1,
                                         int *intersection_count_out,
										 float *intersection_1_out, float *intersection_2_out) {
    xvec3 v10 = xvec3_sub(point_1, point_0);
    float a = xvec3_dot(v10, v10);
    
    xvec3 v0s = xvec3_sub(point_0, s.center);
    float b = 2.0f * xvec3_dot(v10, v0s);
    
    float c = xvec3_dot(s.center, s.center) + xvec3_dot(point_0, point_0);
    c -= 2.0f * xvec3_dot(s.center, point_0);
    c -= (s.radius * s.radius);
    
    // It's a quadratic!
    float i = (b * b) - 4 * a * c;
    
    if (i < 0.0f) return false;
    
    if (i == 0.0f) {
        if (intersection_count_out) *intersection_count_out = 1;
        if (intersection_1_out)     *intersection_1_out     = -b / (2.0f * a);
    } else {
        if (intersection_count_out) *intersection_count_out = 2;
        if (intersection_1_out)     *intersection_1_out     = (-b + sqrtf(i)) / (2.0f * a);
        if (intersection_2_out)     *intersection_2_out     = (-b - sqrtf(i)) / (2.0f * a);
    }
    
    return true;
}


// This and other collision algorithms cribbed from
// http://www.flipcode.com/archives/Moving_Sphere_VS_Triangle_Collision.shtml
XPLINLINE bool xpl_sphere_swept_intersect_triangle(const xpl_sphere s,
                                                   const xvec3 *tri_points, const xvec3 tri_normal,
                                                   const xvec3 sphere_vel,
                                                   float *dist_travel_out, xvec3 *reaction_out) {
    
    int i;
    float v_sq = xvec3_length_sq(sphere_vel);
    xvec3 normalized_velocity = xvec3_normalize(sphere_vel);
    float normalized_velocity_dot_tri_normal = xvec3_dot(tri_normal, normalized_velocity);
    if (v_sq > 0.f) {
        if (normalized_velocity_dot_tri_normal > - 0.0001f) return false;
    }
    
    int colliding = -1;
    
    *dist_travel_out = FLT_MAX;
    
    {
        xpl_plane plane = xpl_plane_normalized_point_normal(tri_points[0], tri_normal);
        
        // pass 1: sphere versus plane
        float h = xpl_plane_dist(plane, s.center);
        
        if (h < -s.radius) return false;
        
        if (h > s.radius) {
            // Minkowski sum
            h -= s.radius;
            if (normalized_velocity_dot_tri_normal != 0.0f) {
                float t = -h / normalized_velocity_dot_tri_normal;
                xvec3 on_plane = xvec3_add(s.center, xvec3_scale(normalized_velocity, t));
                if (xvec3_point_inside_triangle(tri_points, on_plane)) {
                    *dist_travel_out = t;
                    if (reaction_out) *reaction_out = tri_normal;
                    colliding = 0;
                }
            }
        }
    }
    
    // pass 2: sphere versus triangle vertices
    for (i = 0; i < 3; ++i) {
        xvec3 segment_point0 = tri_points[i];
        xvec3 segment_point1 = xvec3_sub(segment_point0, normalized_velocity);
        xvec3 v = xvec3_sub(segment_point1, segment_point0);
        
        float intersection_1 = FLT_MAX;
        float intersection_2 = FLT_MAX;
        int intersection_count;
        bool intersect_line = xpl_sphere_intersect_line(s, segment_point0, segment_point1, &intersection_count, &intersection_1, &intersection_2);
        if (! intersect_line) continue;
        
        float t = xmin(intersection_1, intersection_2);
        if (t < 0) continue;
        
        if (t < *dist_travel_out) {
            *dist_travel_out = t;
            xvec3 on_sphere = xvec3_add(segment_point0, xvec3_scale(v, t));
            if (reaction_out) *reaction_out = xvec3_sub(s.center, on_sphere);
            
            colliding = 1;
        }
    }
    
    // pass 3: sphere versus triangle edges
    for (i = 0; i < 3; ++i) {
        xvec3 edge0 = tri_points[i];
        xvec3 edge1 = tri_points[(i + 1) % 3];
        
        xpl_plane edge_plane = xpl_plane_normalized_points(edge0, edge1, xvec3_sub(edge1, normalized_velocity));
        float d = xpl_plane_dist(edge_plane, s.center);
        if (d > s.radius || d < -s.radius) continue;
        
        float srsq = s.radius * s.radius;
        float r = sqrtf(srsq - d * d);
        
        xvec3 point0 = xpl_plane_project(edge_plane, s.center);
        
        xvec3 on_line;
        /* float h = */ xvec3_distance_to_line(point0, edge0, edge1, &on_line);
        xvec3 v = xvec3_sub(on_line, point0);
        v = xvec3_normalize(v);
        
        xvec3 point1 = xvec3_add(xvec3_scale(v, r), point0);
        
        float p1_x = fabsf(edge_plane.A);
        float p1_y = fabsf(edge_plane.B);
        float p1_z = fabsf(edge_plane.C);
        
        int a0 = 0, a1 = 1;
        if (p1_x > p1_y && p1_x > p1_z) {
            a0 = 1; a1 = 2;
        } else if (p1_y > p1_z) {
            a0 = 0; a1 = 2;
        }
        
        xvec3 vv = xvec3_add(point1, normalized_velocity);
        
        float t;
        bool line_line = xvec2_intersect_line_line(xvec2_set(point1.data[a0], point1.data[a1]),
                                                   xvec2_set(vv.data[a0], vv.data[a1]),
                                                   xvec2_set(edge0.data[a0], edge0.data[a1]),
                                                   xvec2_set(edge1.data[a0], edge1.data[a1]),
                                                   &t);
        if ((! line_line) || (t < 0)) continue;
        
        xvec3 intersection = xvec3_add(point1, xvec3_scale(normalized_velocity, t));
        
        xvec3 r1 = xvec3_sub(edge0, intersection);
        xvec3 r2 = xvec3_sub(edge1, intersection);
        if (xvec3_dot(r1, r2) > 0) continue;
        
        if (t > *dist_travel_out) continue;
        
        *dist_travel_out = t;
        if (reaction_out) *reaction_out = xvec3_sub(s.center, point1);
        colliding = 2;
    }
    
    if (reaction_out && (colliding != -1)) {
        *reaction_out = xvec3_normalize(*reaction_out);
    }
    
    return (colliding != -1);
}

#endif
