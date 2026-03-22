$input v_texcoord0, v_color0

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);

uniform vec4 u_spritePriority;

void main()
{
    vec4 txcol = texture2D(s_texture, v_texcoord0);
    if(txcol.a <= 0.f) discard;
    // VDP1 gouraud: additive signed offset per vertex
    gl_FragColor = vec4(clamp(txcol.rgb + v_color0.rgb, 0.0, 1.0), (u_spritePriority.x + 1.0) / 8.0);
}
