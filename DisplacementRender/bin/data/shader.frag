uniform vec3 baseColor;

varying vec4 color;
varying vec3 normal;
varying vec3 eyeDirection;
varying float lightIntensity;

void main(void) {
	if(color.a == 0.)
		discard;
	color.a = 1.;
	gl_FragColor = lightIntensity * .5 + color;
}
