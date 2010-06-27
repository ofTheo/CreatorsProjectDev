/*
 I'm not totally convinced this normal calculation is right,
 but it seems to behave correctly.
*/

uniform float depthScaling;
uniform vec3 lightPosition;
uniform sampler2D displacementMap;
uniform vec2 normalOffset;

varying vec4 color;
varying vec3 normal;
varying float lightIntensity;

void main() {
	// make depth
	color = texture2D(displacementMap, gl_MultiTexCoord0.xy);
	vec4 pos = gl_Vertex;
	pos.z += color.a * depthScaling;
	gl_Position = gl_ModelViewProjectionMatrix * pos;

	// get neighbor coords
	vec2 leftCoord = gl_MultiTexCoord0.xy;
	leftCoord.x -= normalOffset.x;
	vec2 rightCoord = gl_MultiTexCoord0.xy;
	rightCoord.x += normalOffset.x;

	vec2 upCoord = gl_MultiTexCoord0.xy;
	upCoord.y -= normalOffset.y;
	vec2 downCoord = gl_MultiTexCoord0.xy;
	downCoord.y += normalOffset.y;

	// lookup neighbors
	vec4 left = texture2D(displacementMap, leftCoord);
	vec4 right = texture2D(displacementMap, rightCoord);
	vec4 up = texture2D(displacementMap, upCoord);
	vec4 down = texture2D(displacementMap, downCoord);

	// get derivatives
	float xd = right.a - left.a;
	float yd = down.a - up.a;

	xd *= depthScaling;
	yd *= depthScaling;

	// make tangent vectors & make normal
	vec3 xtan = vec3(2, 0, xd);
	vec3 ytan = vec3(0, 2, yd);
	vec3 norm = cross(xtan, ytan);
	normal = gl_NormalMatrix * normalize(norm);

	// solve lighting
	vec3 eyeDirection = (gl_ModelViewMatrix * gl_Vertex).xyz;
	lightIntensity = max(dot(normalize(lightPosition - eyeDirection), normal), 0.0);
}
