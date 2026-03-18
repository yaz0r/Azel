$input v_color0, v_depth

#include "bgfx_shader.sh"

uniform vec4 u_spritePriority;

void main()
{
    gl_FragColor = vec4(v_color0.rgb, (u_spritePriority.x + 1.0) / 8.0);
    gl_FragDepth = v_depth;
}
