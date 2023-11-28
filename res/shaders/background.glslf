in vec3 v_coord;
out vec4 f_color;

uniform samplerCube u_cubemap;

void main(){
	vec3 dir = normalize(v_coord);
	f_color = texture(u_cubemap, dir);
}
