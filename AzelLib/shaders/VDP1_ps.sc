$input v_texcoord0, v_color0

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);

uniform float s_textureInfluence;
uniform float s_ambientInfluence;

void main()
{
    //float distanceValue = mix(0, 10, 1-gl_FragCoord.z);
    //distanceValue = clamp(distanceValue, 0, 1);
    //vec4 fallout = texture2D(s_falloff, vec2(distanceValue,0));
    vec4 txcol = texture2D(s_texture, v_texcoord0/textureSize(s_texture, 0));
    if(txcol.a <= 0) discard;
    gl_FragColor = (clamp(txcol, 0, 1) * s_textureInfluence) + v_color0;
    gl_FragColor = txcol;
    gl_FragColor.w = 1;
}
