in vec2 v_coord;
out vec4 f_color;

uniform sampler2D u_texture0;

void main(){
	f_color = texture(u_texture0, v_coord);
}

