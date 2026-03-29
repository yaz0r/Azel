$input a_position, a_texcoord0
$output v_texcoord0

#include "bgfx_shader.sh"

uniform mat4 u_customModelViewProj;

void main()
{
    gl_Position = mul(u_customModelViewProj, vec4(a_position.xyz, 1));
    v_texcoord0 = a_texcoord0;
}
