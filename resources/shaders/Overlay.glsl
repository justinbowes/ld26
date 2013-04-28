/*
 Copyright © 2012 Informi Software Inc.
 All rights reserved.
*/


// ---------------------------------------------------------------
// -- Vertex.GL32

in vec3 			position;

out vec2            vuv;

void main()
{
    gl_Position = vec4(position, 1.0);
    vuv = position.xy;
}

// ---------------------------------------------------------------
// -- Fragment.GL32

#include "Lib.Effects"

uniform float   strength;
uniform vec4    color;
uniform float   scanline_amount;

in vec2         vuv;
out vec4        frag_color;

void main()
{
    float darken = scanline(scanline_amount);
    frag_color = vec4(0.0, 0.0, 0.0, darken);
    
    vec2 vignette_uv = 0.4 * vuv + 0.5;
    darken = 1.0 - vignette(vignette_uv, 1.0);
    frag_color += vec4(0.0, 0.0, 0.0, darken);
	
    float intensity = color.a * strength;
    frag_color += vec4(color.rgb * intensity, intensity);
	
	//frag_color.r = frag_color.a;
}
