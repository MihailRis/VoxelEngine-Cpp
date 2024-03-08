in vec2 v_coord;
out vec4 f_color;

uniform sampler2D u_texture0;
uniform float u_timer;

void main(){
    vec2 coord = v_coord;
    coord.x += sin(u_timer*5.0+coord.x*4.0) * 0.02;
    coord.y += cos(u_timer*5.0+coord.y*4.0) * 0.02;
	f_color = texture(u_texture0, coord);
}

