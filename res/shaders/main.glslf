in vec4 a_color;
in vec2 a_texCoord;
in float a_distance;
in vec3 a_dir;
out vec4 f_color;

uniform sampler2D u_texture0;
uniform samplerCube u_cubemap;
uniform vec3 u_fogColor;
uniform float u_fogFactor;
uniform float u_fogCurve;

void main(){
	vec3 fogColor = texture(u_cubemap, a_dir).rgb;
	vec4 tex_color = texture(u_texture0, a_texCoord);
	float depth = (a_distance/256.0);
	float alpha = a_color.a * tex_color.a;
	// anyway it's any alpha-test alternative required
	if (alpha < 0.3f)
		discard;
	f_color = mix(a_color * tex_color, vec4(fogColor,1.0), min(1.0, pow(depth*u_fogFactor, u_fogCurve)));
	f_color.a = alpha;
}
