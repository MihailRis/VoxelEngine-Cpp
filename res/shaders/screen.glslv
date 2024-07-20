layout (location = 0) in vec2 v_position;

out vec2 v_coord;

uniform ivec2 u_screenSize;
uniform float u_timer;
uniform float u_dayTime;

void main(){
    v_coord = v_position * 0.5 + 0.5;
    gl_Position = vec4(v_position, 0.0, 1.0);
}

