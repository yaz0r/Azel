$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_texture,  0);

void main()
{
    vec4 txcol = texture2D(s_texture, v_texcoord0);
    if(txcol.a <= 0.f) discard;
    gl_FragColor = txcol;
    gl_FragColor.w = 1.f;
}
