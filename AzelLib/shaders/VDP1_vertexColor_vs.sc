$input a_position, a_color0
$output v_color0

#include "bgfx_shader.sh"

uniform mat4 u_customModelViewProj;

void main()
{
    gl_Position = mul(u_customModelViewProj, vec4(a_position.xyz, 1));
    v_color0 = a_color0;
}
