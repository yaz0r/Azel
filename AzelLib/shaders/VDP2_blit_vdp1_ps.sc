$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);
SAMPLER2D(s_priority, 1);

uniform vec4 u_priorityLevel;

void main()
{
    vec4 txcol = texture2D(s_texture, v_texcoord0);
    if(txcol.a <= 0.0) discard;

    float storedPriority = texture2D(s_priority, v_texcoord0).r;
    int priority = int(storedPriority * 7.0 + 0.5);
    if(priority != int(u_priorityLevel.x)) discard;

    gl_FragColor = txcol;
    gl_FragColor.w = 1.0;
}
