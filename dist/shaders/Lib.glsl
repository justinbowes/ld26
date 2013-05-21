// ------------------------------------------------------------------------
// -- Constants
#define M_E         2.7182818284590452354
#define M_LOG2E     1.4426950408889634074
#define M_LOG10E	0.43429448190325182765
#define M_LN2		0.69314718055994530942
#define M_LN10		2.30258509299404568402
#define M_PI		3.14159265358979323846
#define M_2PI		6.28318530717959/*todo*/
#define M_3PI       9.42477796076938/*todo*/
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.78539816339744830962
#define M_1_PI		0.31830988618379067154
#define M_2_PI		0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2		1.41421356237309504880
#define M_SQRT1_2	0.70710678118654752440

// ------------------------------------------------------------------------
// -- Noise

#include "Lib.Noise.Simplex"
#include "Lib.Noise.Perlin"

// ------------------------------------------------------------------------
// -- Noise.Simplex

#include "LibNoiseSimplex.Lib"

// ------------------------------------------------------------------------
// -- Noise.Perlin

#include "LibNoisePerlin.Lib"

// ------------------------------------------------------------------------
// -- EncodeNormal

/**
 * Discontinuities at 0, 0, -1
 */
vec2 encode_normal_lambert(vec3 normal) {
    float f = sqrt(normal.z * 8.0 + 8.0);
    return normal.xy / f + 0.5;
}

vec3 decode_normal_lambert(vec2 encoded) {
    vec2 mapped = encoded * 4.0 - 2.0;
    float f = dot(mapped, mapped);
    float g = sqrt(1 - 0.25 * f);
    return normalize(vec3(mapped * g, 1.0 - f * -0.5));
}

/**
 * Ugly rippling along y = 0
 */
vec2 encode_normal_cry(vec3 normal) {
    vec2 enc = normalize(normal.xy) * (sqrt(-normal.z * 0.5 + 0.5));
    enc = enc * 0.5 + 0.5;
    return enc;
}

vec3 decode_normal_cry(vec2 encoded) {
    vec4 nn = vec4(encoded, 0.0, 0.0) * vec4(2.0, 2.0, 0.0, 0.0) + vec4(-1.0, -1.0, 1.0, -1.0);
    float l = dot(nn.xyz, -nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(l);
    return normalize(nn.xyz * 2.0 + vec3(0.0, 0.0, -1.0));
}

// ------------------------------------------------------------------------
// --- Depth

float linearize_depth(float near, float far, float exp_depth) {
    return (2.0 * near) / (far + near - exp_depth * (far - near));
}

// not tested
float depth_to_z_position(float near, float far, float depth) {
	return near / (far - depth * (far - near)) * far;
}

vec3 world_from_depth_z_over_w(mat4 inverse_vp, vec2 uv, float depth) {
    vec4 vec4_h = vec4(vec3(uv, depth) * 2.0 - 1.0, 1.0);
    vec4 vec4_d = inverse_vp * vec4_h;
    return vec4_d.xyz / vec4_d.w;
}

float fog_from_raw_depth(float raw_depth, float strength) {
    float one_plus_str_recip = 1.0 + (1.0 / strength);
    float fog = one_plus_str_recip + (1.0 / (strength * (raw_depth - one_plus_str_recip)));
    return clamp(0.0, 1.0, fog);
}

// ------------------------------------------------------------------------
// -- Spherical

vec3 to_spherical(vec3 cart, float r) {
    return vec3(r, acos(cart.z / r), atan(cart.y, cart.x));
}

vec3 to_spherical(vec3 cart) {
    return to_spherical(cart, length(cart));
}

vec3 to_cartesian(vec3 rtp) {
    return vec3(rtp.x * sin(rtp.y) * cos(rtp.z),
                rtp.x * sin(rtp.y) * sin(rtp.z),
                rtp.x * cos(rtp.y));
}

vec3 cartesian_error(vec3 vec) {
    vec3 spherical = to_spherical(vec);
    vec3 cartesian = to_cartesian(spherical);
    return abs(vec - cartesian);
}

// ------------------------------------------------------------------------
// -- Debug

vec4 bogus() {
    return (mod(gl_FragCoord.x + gl_FragCoord.y, 8.0) < 4.0 ?
            vec4(0.0, 0.0, gl_FragCoord.y / gl_FragCoord.x, 1.0): vec4(1.0, gl_FragCoord.x / gl_FragCoord.y, 0.0, 1.0));
}

vec4 box(vec2 uv, float width) {
    bool border = uv.x < width || uv.y < width || uv.x > 1.0 - width || uv.y > 1.0 - width;
    return border ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0);
}


// ------------------------------------------------------------------------
// -- Color

vec3 hue(float h)
{
    float r = abs(h * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(h * 6.0 - 2.0);
    float b = 2.0 - abs(h * 6.0 - 4.0);
    return clamp(vec3(r, g, b), 0.0, 1.0);
}

vec3 hsv_to_rgb(vec3 hsv)
{
    return ((hue(hsv.x) - 1) * hsv.y + 1) * hsv.z;
}


// ------------------------------------------------------------------------
// -- Effects

#include "Lib.Constants"

// Assumes [0, 1] coordinates
float vignette(vec2 uv, float strength) {
	float distance = length(uv * 2.0 - 1.0) / M_SQRT2;
    distance = clamp(distance, 0.0, 1.0);
	float amount = mix(0.0, 1.0, distance);
	float result = strength * amount;
    result = result * result * result;
	return 1.0 - result;
}

float scanline(float strength) {
	const float w = 1.0;
	float scanline = w - mod(gl_FragCoord.y + 0.5, 2.0 * w);
	scanline /= w;
	scanline *= scanline;
	scanline *= strength;
	return scanline;
}

float alias_toon(float value, int steps) {
    float alias = fwidth(value);
    float result;
    switch (steps) {
        case 1:
            result = step(0.5, value);
            break;
            
        default:
            result = round(value * steps) / steps;
            break;
    }

    if (abs(result - value) < alias) {
        return mix(result, value, smoothstep(value - alias, value + alias, result));
    }
    return result;
}

// ------------------------------------------------------------------------
// -- Blackbody
#include "LibBlackbody.Lib"
