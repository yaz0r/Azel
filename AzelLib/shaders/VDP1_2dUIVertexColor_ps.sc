$input v_color0, v_depth

#include "bgfx_shader.sh"

uniform vec4 u_spritePriority;

void main()
{
    gl_FragData[0] = vec4(v_color0.rgb, 1.0);
    gl_FragData[1] = vec4(u_spritePriority.x / 7.0, 0.0, 0.0, 1.0);
    gl_FragDepth = v_depth;
}
