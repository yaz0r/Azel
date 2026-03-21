$input a_position, a_texcoord0, a_color0, a_texcoord1
$output v_texcoord0, v_color0, v_clipPos, v_quadIndex

#include "bgfx_shader.sh"

uniform vec2 u_2dOffset;
uniform mat4 u_customModelViewProj;

void main()
{
    vec4 clipPos = mul(u_customModelViewProj, vec4(a_position.xyz, 1));
    gl_Position = clipPos;
    v_texcoord0 = a_texcoord0;
    v_color0 = a_color0;
    v_clipPos = vec3(clipPos.xy, clipPos.w);
    v_quadIndex = a_texcoord1;
}
