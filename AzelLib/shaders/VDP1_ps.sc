$input v_texcoord0, v_color0

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);

uniform float s_textureInfluence;
uniform float s_ambientInfluence;
uniform vec4 u_spritePriority;

void main()
{
    vec2 UV = v_texcoord0 / vec2(textureSize(s_texture, 0));
    vec4 txcol = texture2D(s_texture, UV);
    if(txcol.a <= 0.f) discard;
    gl_FragColor = vec4(txcol.rgb, (u_spritePriority.x + 1.0) / 8.0);
}
