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
// -- Background.Fragment.GL32

#include "Lib.Noise.Simplex"
#include "Lib.Blackbody"

uniform float   time;

in vec2         vuv;
out vec4        frag_color;

void main()
{
    vec2 xy = (gl_FragCoord.xy + vec2(0.0, time)) * vec2(0.00002, 0.02);
    
    float v = snoise(vec3(xy, 0.1 * time));
    float k = 20000 * snoise(vec3(xy + 17.0, -time));

    vec4 bb = blackbody_rgbi(k);
    
    frag_color = vec4(bb.rgb * min(pow(bb.a, 0.1), 1.0), 1.0);
}
