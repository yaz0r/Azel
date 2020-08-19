$input v_color0, v_depth

#include "bgfx_shader.sh"

void main()
{
    gl_FragColor = v_color0;
    gl_FragColor.w = 1.f;
    gl_FragDepth = v_depth;
}
