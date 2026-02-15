$input v_color0

#include "bgfx_shader.sh"

void main()
{
    gl_FragColor = v_color0;
    gl_FragColor.w = 1.f;

}
