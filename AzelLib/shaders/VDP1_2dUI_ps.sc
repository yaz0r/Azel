$input v_texcoord0, v_depth

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);

uniform vec4 u_spritePriority;

void main()
{
    vec4 txcol = texture2D(s_texture, v_texcoord0);
    if(txcol.a <= 0.f) discard;
    gl_FragData[0] = vec4(txcol.rgb, 1.0);
    gl_FragData[1] = vec4(u_spritePriority.x / 7.0, 0.0, 0.0, 1.0);
    gl_FragDepth = v_depth;
}
