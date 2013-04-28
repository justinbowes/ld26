/*
 Copyright © 2012 Informi Software Inc.
 All rights reserved.

 Sprite shaders
*/


//------------------- Vertex.GL32 -------------------------

in vec2 			position;
in vec4 			color;
in vec2 			uv;

uniform mat4 		mvp;

out vec4			vcolor;
out vec2			vuv;
out vec2            vscreen_uv;

void main()
{
    vec4 transformed_position = mvp * vec4(position, 0.0, 1.0);
    gl_Position = transformed_position;
    vcolor = color; 
    vuv = uv;
    vscreen_uv = transformed_position.xy / transformed_position.w + 1.0 * 0.5;
}


//------------------- Fragment.GL32 -------------------------------

#include "Lib.Effects"

uniform sampler2D 	texture;

in vec4				vcolor;
in vec2				vuv;
in vec2             vscreen_uv;

out vec4			frag_color;

void main()
{
    vec4 tc = texture(texture, vuv);
    frag_color.rgb = vcolor.rgb * vcolor.a * tc.rgb * tc.a;
	frag_color.a = vcolor.a * tc.a;
}