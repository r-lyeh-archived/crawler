attribute vec4 coord;
uniform mat4 mvp;
varying vec4 texcoord;

void main(void) {
	// Just pass the original vertex coordinates to the fragment shader as texture coordinates
	texcoord = coord;

	// Apply the model-view-projection matrix to the xyz components of the vertex coordinates
	gl_Position = mvp * vec4(coord.xyz, 1);
}
