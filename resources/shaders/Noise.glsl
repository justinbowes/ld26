/*
Copyright © 2013 Informi Software Inc.
All rights reserved.

Luminance noise shader
*/

// ----------------------- Vertex.GL32 ------------------------------

in vec3 			position;

out vec2			vuv;

void main()
{
    gl_Position = vec4(position, 1.0);
    vuv = position.xy * 0.5 + vec2(0.5);
}


// ----------------------- Fragment.GL32 -----------------------------

uniform sampler2D 	texture;
uniform vec2        screen_size;
uniform float       time;

in vec2				vuv;

out vec4			frag_color;


float noise(vec2 v) {
    int n = int(dot(v, vec2(40.0, 6400.0)));
    n = (n << 13) ^ n;
    return float((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 2147483648.0;
}

const vec3          luminance       = vec3(0.299, 0.587, 0.114);
const float			amount			= 0.02;

void main() {
    vec4 tex = texture(texture, vuv);
	float noise = noise(screen_size * (vuv + fract(time / 1000.0))) * amount - 0.5 * amount;
    frag_color = vec4(tex.rgb + noise * luminance, 1.0);
}

