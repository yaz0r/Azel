$input a_position, a_texcoord0, a_color0
$output v_texcoord0, v_color0

#include "bgfx_shader.sh"

uniform vec2 u_2dOffset;
uniform mat4 u_customModelViewProj;

void main()
{
    gl_Position = mul(u_customModelViewProj, vec4(a_position.xyz, 1));
    //gl_Position.xy += u_2dOffset;
    v_texcoord0 = a_texcoord0;
    v_color0 = a_color0;
}
