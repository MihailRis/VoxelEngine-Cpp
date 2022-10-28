#version 330 core

in vec4 a_color;
in vec2 a_texCoord;
in float a_distance;
out vec4 f_color;

uniform sampler2D u_texture0;
uniform vec3 u_fogColor;
uniform float u_fogFactor;

void main(){
	vec4 tex_color = texture(u_texture0, a_texCoord);
	float depth = (a_distance/256.0);
	float alpha = a_color.a * tex_color.a;
	// anyway it's any alpha-test alternative required
	if (alpha < 0.1f)
		discard;
	f_color = mix(a_color * tex_color, vec4(u_fogColor,1.0), min(1.0, depth*u_fogFactor));
	f_color.a = alpha;
}
