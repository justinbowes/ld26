/*
 Copyright © 2012 Informi Software Inc.
 All rights reserved.

 Text shaders
*/


//------------------- Vertex.GL32 -------------------------

in vec3 			position;
in vec4 			color;
in vec2 			uv;
in float 			shift;
in float 			gamma;

uniform mat4 		mvp;
uniform vec4		global_tint;

out float			vshift;
out float 			vgamma;
out vec4			vcolor;
out vec2			vuv;

void main()
{

    gl_Position = mvp * vec4(position, 1.0);
    vcolor = vec4(color.rgb * global_tint.rgb * global_tint.a, color.a * global_tint.a);
    vshift = shift;
    vgamma = gamma;
    vuv = uv;
}


//------------------- Fragment.GL32 -------------------------------

uniform sampler2D 	tex;
uniform vec3 		subpixel;

in float 			vgamma;
in float 			vshift;
in vec4				vcolor;
in vec2				vuv;

const vec2			neg_one_x 		= vec2(-1.0, 0.0);
const vec3			one 			= vec3(1.0, 1.0, 1.0);

out vec4			frag_color;

void main()
{

    // LCD Off
    if (subpixel.z == 1.0) {
        // No GL_ALPHA texture in GL3; use GL_RED
        float a = texture(tex, vuv).r;
        frag_color = vcolor * pow(a, 1.0 / vgamma);
        return;
    }

    // LCD On

    vec4 current  = texture(tex, vuv);
    vec4 previous = texture(tex, vuv + neg_one_x * subpixel.xy);
	vec3 mix_1;
	vec3 mix_2;
	float z;

    if (vshift <= 0.333) {
        z = vshift / 0.333;
		mix_1 = current.rgb;
		mix_2 = vec3(previous.b, current.r, current.g);
    } else if (vshift <= 0.666) {
        z = (vshift - 0.333) / 0.333;
		mix_1 = vec3(previous.b, current.r, current.g);
		mix_2 = vec3(previous.g, previous.b, current.r);
    } else if (vshift < 1.0) {
        z = (vshift - 0.666) / 0.334;
		mix_1 = vec3(previous.g, previous.b, current.r);
		mix_2 = previous.rgb;
    }

	vec3 gamma_color = mix(mix_1, mix_2, z);
    gamma_color = pow(gamma_color, vec3(1.0 / vgamma));

    frag_color = vec4(gamma_color * vcolor.rgb * vcolor.a, dot(gamma_color, one) / 3.0);
}


//------------------- Vertex.ES2 -------------------------
attribute vec3 			position;
attribute vec4 			color;
attribute vec2 			uv;
attribute float			shift;
attribute float 		gamma;

uniform mat4			mvp;
uniform vec4			global_tint;

varying float			vshift;
varying float			vgamma;
varying vec4			vcolor;
varying vec2			vuv;

void main()
{

    gl_Position = mvp * vec4(position, 1.0);
    vcolor = vec4(color.rgb * global_tint.rgb * global_tint.a, color.a * global_tint.a);
    vshift = shift;
    vgamma = gamma;
    vuv = uv;
}


//------------------- Fragment.ES2 -------------------------------
precision mediump float;

uniform sampler2D 	tex;

varying float 		vgamma;
varying float 		vshift;
varying vec4		vcolor;
varying vec2		vuv;

vec4				frag_color;

void main()
{

	float a = texture2D(tex, vuv).a;
	frag_color = vcolor * pow(a, 1.0 / vgamma);
	gl_FragColor = frag_color;
}