#version 120
varying vec3 f_color;

void main() {
	gl_FragColor = vec4(f_color.r, f_color.b, f_color.g, 1.0);
}