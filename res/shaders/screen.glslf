in vec2 v_coord;
out vec4 f_color;

uniform sampler2D u_texture0;
uniform float u_timer;
uniform float radius;
uniform vec2 dir;
uniform ivec2 u_screenSize;
uniform float u_stripe;

float random(vec2 st){
    return fract(sin(dot(st.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// Vignette
vec4 apply_vignette(vec4 color)
{
    vec2 position = (gl_FragCoord.xy / u_screenSize) - vec2(0.5);
    float dist = length(position);

    float radius = 0.88;
    float softness = 0.365;
    float vignette = smoothstep(radius, radius - softness, dist);

    color.rgb = color.rgb - (1.0 - vignette);

    return color;
}

// Cromatic
vec4 apply_cromatic(vec4 col) {
    vec2 uv = v_coord;
    vec2 uv1 = v_coord;
    vec2 uv2 = v_coord;
    uv1 = (uv / 1.0125) + 0.00625;
    uv2 = (uv / 1.025) + 0.0125;
    vec4 r_color1 = texture(u_texture0, uv);
    vec4 g_color1 = texture(u_texture0, uv);
    vec4 b_color1 = texture(u_texture0, uv);
    vec4 r_color2 = texture(u_texture0, uv1);
    vec4 g_color2 = texture(u_texture0, uv1);
    vec4 b_color2 = texture(u_texture0, uv1);
    vec4 r_color3 = texture(u_texture0, uv2);
    vec4 g_color3 = texture(u_texture0, uv2);
    vec4 b_color3 = texture(u_texture0, uv2);
    col = vec4((r_color1.r + r_color2.r + r_color3.r) / 3, (g_color1.g + g_color2.g + g_color2.g) / 3, 
    (b_color1.b + b_color2.b + b_color3.b) / 3, g_color1.a);
    return col;
}

// Retro-filter
void main() {
    float totalWeight = 0.0;

    vec2 uv = v_coord;

    vec3 color;

    f_color = texture(u_texture0, v_coord);

    float offset = sin(u_timer * 0.000005) * 0.1;

    vec4 textureColor = texture(u_texture0, v_coord);

    vec2 glitchedTexCoord = uv + vec2(offset, 0.0);

    float stripeMask = smoothstep(uv.y - 0.1 / 2.0, uv.y + 0.1 / 2.0, fract(uv.y * 250));

    vec2 centerCoord = vec2(0.5, 0.5);

    vec4 centerColor = texture(u_texture0, centerCoord);

    float centerBrightness = dot(centerColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    float glitch = random(vec2(1.0, u_timer)) * 0.5 + 0.5;
    vec2 uvRedGlitch = mix(uv + 0.001, glitchedTexCoord, stripeMask);
    vec2 uvGreenGlitch = mix(uv, glitchedTexCoord, stripeMask);
    vec2 uvBlueGlitch = mix(uv - 0.001, glitchedTexCoord, stripeMask);

    vec2 redUV = uvRedGlitch + vec2(0.0005, 0.0);
    vec4 redColor = texture(u_texture0, redUV);

    vec2 greenUV = uvGreenGlitch - vec2(0.0, 0.002);
    vec4 greenColor = texture(u_texture0, greenUV);

    vec2 blueUV = uvBlueGlitch - vec2(-0.0005, 0.0);
    vec4 blueColor = texture(u_texture0, blueUV);

    f_color = vec4(redColor.r, greenColor.g, blueColor.b, 1.0);

    f_color = vec4(f_color.r + 0.175, f_color.g, f_color.b + 0.2, f_color.a) - 0.125;

    vec4 col1 = apply_cromatic(f_color);

    f_color = vec4((f_color + col1) / 2);

    f_color = apply_vignette(f_color);

    float grayscale = (f_color.r + f_color.g + f_color.b) / 3.0;

    f_color = vec4(grayscale + (f_color.r * 0.9), grayscale + (f_color.g * 0.9), grayscale + (f_color.b * 0.9), f_color.a);
    f_color = vec4(f_color.rgb * 0.65, f_color.a);
}

/*
// Parallax
void main() {
    vec2 uv = v_coord;
    vec4 textureColor = texture(u_texture0, uv);
    vec2 uv1 = (uv / 1.1) + 0.05;
    vec2 uv2 = (uv / 1.2) + 0.1;
    vec4 texCol1 = texture(u_texture0, uv1);
    vec4 texCol2 = texture(u_texture0, uv2);
    textureColor = vec4((textureColor + texCol1 + texCol2) / 3);
    f_color = textureColor;
}
*/

/*
// Blur
void main() {
    vec2 uv = v_coord;
    vec4 textureColor = texture(u_texture0, uv);
    vec2 uv1 = vec2(uv.x - 0.0005, uv.y);
    vec2 uv2 = vec2(uv.x - 0.001, uv.y);
    vec2 uv3 = vec2(uv.x + 0.001, uv.y);
    vec2 uv4 = vec2(uv.x + 0.0005, uv.y);
    vec2 uv5 = vec2(uv.x - 0.005, uv.y);
    vec2 uv6 = vec2(uv.x - 0.01, uv.y);
    vec2 uv7 = vec2(uv.x + 0.005, uv.y);
    vec2 uv8 = vec2(uv.x + 0.01, uv.y);
    vec2 uv9 = vec2(uv.x, uv.y - 0.0005);
    vec2 uv10 = vec2(uv.x , uv.y- 0.001);
    vec2 uv11 = vec2(uv.x, uv.y + 0.001);
    vec2 uv12 = vec2(uv.x, uv.y + 0.0005);
    vec2 uv13 = vec2(uv.x, uv.y - 0.005);
    vec2 uv14 = vec2(uv.x, uv.y - 0.01);
    vec2 uv15 = vec2(uv.x, uv.y + 0.005);
    vec2 uv16 = vec2(uv.x, uv.y + 0.01);
    vec4 textureColor1 = texture(u_texture0, uv1);
    vec4 textureColor2 = texture(u_texture0, uv2);
    vec4 textureColor3 = texture(u_texture0, uv3);
    vec4 textureColor4 = texture(u_texture0, uv4);
    vec4 textureColor5 = texture(u_texture0, uv5);
    vec4 textureColor6 = texture(u_texture0, uv6);
    vec4 textureColor7 = texture(u_texture0, uv7);
    vec4 textureColor8 = texture(u_texture0, uv8);
    vec4 textureColor9 = texture(u_texture0, uv9);
    vec4 textureColor10 = texture(u_texture0, uv10);
    vec4 textureColor11 = texture(u_texture0, uv11);
    vec4 textureColor12 = texture(u_texture0, uv12);
    vec4 textureColor13 = texture(u_texture0, uv13);
    vec4 textureColor14 = texture(u_texture0, uv14);
    vec4 textureColor15 = texture(u_texture0, uv15);
    vec4 textureColor16 = texture(u_texture0, uv16);
    f_color = vec4((textureColor + textureColor1 + textureColor2 + 
    textureColor3 + textureColor4 + textureColor5 + textureColor6 + textureColor7 + 
    textureColor8 + textureColor9 + textureColor10 + textureColor11 + textureColor12 + 
    textureColor13 + textureColor14 + textureColor15 + textureColor16) / 17);
}
*/