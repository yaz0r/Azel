$input v_texcoord0, v_color0

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);

uniform float s_textureInfluence;
uniform float s_ambientInfluence;
uniform vec4 u_spritePriority;

void main()
{
    //float distanceValue = mix(0, 10, 1-gl_FragCoord.z);
    //distanceValue = clamp(distanceValue, 0, 1);
    //vec4 fallout = texture2D(s_falloff, vec2(distanceValue,0));
    vec2 UV = v_texcoord0 / vec2(textureSize(s_texture, 0));
    vec4 txcol = texture2D(s_texture, UV);
    if(txcol.a <= 0.f) discard;
    gl_FragData[0] = vec4(txcol.rgb, 1.0);
    gl_FragData[1] = vec4(u_spritePriority.x / 7.0, 0.0, 0.0, 1.0);
}
