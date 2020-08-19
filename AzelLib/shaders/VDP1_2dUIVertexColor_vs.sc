$input a_position, a_color0
$output v_depth, v_color0

#include "bgfx_shader.sh"

void main()
{
    gl_Position = vec4(a_position, 1);
    gl_Position.x = (a_position.x / (352.f/2.f)) - 1.f;
    gl_Position.y = 1.f - (a_position.y / (224.f/2.f));
    v_depth = a_position.z;
    v_color0 = a_color0;
}
