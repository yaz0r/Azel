$input v_texcoord0, v_depth

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);

uniform vec4 u_spritePriority;

void main()
{
    vec4 txcol = texture2D(s_texture, v_texcoord0);
    if(txcol.a <= 0.f) discard;
    // Encode priority in alpha: (priority + 1) / 8 so alpha=0 means transparent
    gl_FragColor = vec4(txcol.rgb, (u_spritePriority.x + 1.0) / 8.0);
    gl_FragDepth = v_depth;
}
