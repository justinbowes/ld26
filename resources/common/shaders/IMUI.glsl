//---- Vertex.GL32 ----

in vec3 position;
in vec4 color;

uniform mat4 mvp;

out vec4 o_color;

void main() {
    gl_Position = mvp * vec4(position, 1.0);
    o_color = color;
}



//---- Fragment.GL32 ----

in vec4 o_color;
out vec4 color;

void main() {
    color = o_color;
}


//---- Vertex.ES2 ----

attribute vec3 position;
attribute vec4 color;

uniform mat4 mvp;

varying vec4 o_color;

void main() {
    gl_Position = mvp * vec4(position, 1.0);
    o_color = color;
}



//---- Fragment.ES2 ----
precision lowp float;

varying vec4 o_color;

void main() {
    gl_FragColor = o_color;
}
