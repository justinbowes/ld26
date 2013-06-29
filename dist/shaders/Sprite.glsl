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

uniform sampler2D 	tex;

in vec4				vcolor;
in vec2				vuv;
in vec2             vscreen_uv;

out vec4			frag_color;

void main()
{
    vec4 tc = texture(tex, vuv);
    frag_color.rgb = vcolor.rgb * vcolor.a * tc.rgb * tc.a;
	frag_color.a = vcolor.a * tc.a;
}

//------------------- Vertex.ES2 -------------------------

attribute vec2 			position;
attribute vec4 			color;
attribute vec2 			uv;

uniform mat4 		mvp;

varying vec4			vcolor;
varying vec2			vuv;
varying vec2            vscreen_uv;

void main()
{
    vec4 transformed_position = mvp * vec4(position, 0.0, 1.0);
    gl_Position = transformed_position;
    vcolor = color; 
    vuv = uv;
    vscreen_uv = transformed_position.xy / transformed_position.w + 1.0 * 0.5;
}


//------------------- Fragment.ES2 -------------------------------
precision lowp float;
uniform sampler2D 	tex;

varying vec4				vcolor;
varying vec2				vuv;
varying vec2             vscreen_uv;

vec4			frag_color;

void main()
{
    vec4 tc = texture2D(tex, vuv);
    vec4 frag_color = vec4(vcolor.rgb * vcolor.a * tc.rgb * tc.a, vcolor.a * tc.a);
	gl_FragColor = frag_color;
}