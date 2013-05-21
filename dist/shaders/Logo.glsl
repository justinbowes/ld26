/*
 Copyright © 2012 Informi Software Inc.
 All rights reserved.
 
 Debug shaders
*/


//------------------- Vertex.GL32 -------------------------

in vec3 			position;

uniform mat4 		mvp;
uniform vec4        color;

out vec4			vcolor;

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
    vcolor = color;
}


//------------------- Brick.Fragment.GL32 -------------------------------

in vec4             vcolor;
out vec4			frag_color;

void main()
{
    frag_color = vcolor;
}
