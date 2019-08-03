#version 120
varying vec2 f_texcoord;
uniform sampler2D myTexture;


void main() {
	//first texture (flipped so that it's the right way round)
	vec2 flipped_texcoord = vec2(f_texcoord.x, 1.0 - f_texcoord.y);
	gl_FragColor = texture2D(myTexture, flipped_texcoord);
}
