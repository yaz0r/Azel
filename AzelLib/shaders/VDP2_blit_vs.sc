$input a_position
$output v_texcoord0

#include "bgfx_shader.sh"

void main()
{
   gl_Position = vec4(a_position, 1);
   //v_texcoord0 = (a_position.xy+vec2(1,1))/2.0;
   v_texcoord0 = (a_position.xy+vec2(1,1))/2.0;
   v_texcoord0.y = 1 - v_texcoord0.y;
}
