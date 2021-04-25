#version 330 core

in vec4 a_color;
in vec2 a_texCoord;
out vec4 f_color;

uniform sampler2D u_texture0;

void main(){
	vec4 tex_color = texture(u_texture0, a_texCoord);
	if (tex_color.a < 0.5)
		discard;
	f_color = a_color * tex_color;
}
