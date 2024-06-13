in vec2 v_coord;
out vec4 f_color;

uniform sampler2D u_texture0;
uniform ivec2 u_screenSize;

// Vignette
vec4 apply_vignette(vec4 color)
{
    vec2 position = (gl_FragCoord.xy / u_screenSize) - vec2(0.5);
    float dist = length(position);

    float radius = 2.0;
    float softness = 1.7;
    float vignette = smoothstep(radius, radius - softness, dist);

    color.rgb = color.rgb - (1.0 - vignette);

    return color;
}

void main(){
	f_color = texture(u_texture0, v_coord);
	f_color = apply_vignette(f_color);
}

