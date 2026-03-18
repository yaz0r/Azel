$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_vdp1Color,    0);
SAMPLER2D(s_nbg0,         1);
SAMPLER2D(s_nbg1,         2);
SAMPLER2D(s_nbg3,         3);
SAMPLER2D(s_rbg0,         4);

uniform vec4 u_backColor;
uniform vec4 u_vdp2Regs0; // x=BGON, y=PRINA, z=PRINB, w=PRIR

void main()
{
    vec2 uv = v_texcoord0;

    float BGON  = u_vdp2Regs0.x;
    float PRINA = u_vdp2Regs0.y;
    float PRINB = u_vdp2Regs0.z;
    float PRIR  = u_vdp2Regs0.w;

    // Sample all layers
    vec4 vdp1Color = texture2D(s_vdp1Color, uv);
    vec4 nbg0Color = texture2D(s_nbg0, uv);
    vec4 nbg1Color = texture2D(s_nbg1, uv);
    vec4 nbg3Color = texture2D(s_nbg3, uv);
    vec4 rbg0Color = texture2D(s_rbg0, uv);

    // Decode VDP1 priority from alpha: priority = alpha * 8 - 1 (alpha=0 means no pixel)
    float vdp1Priority = floor(vdp1Color.a * 8.0 - 0.5);

    // Start with back screen color
    vec4 result = u_backColor;
    float bestKey = -1.0;

    // Composite key = priority * 8.0 + tieBreakRank
    // Tie-breaking (same priority, highest to lowest):
    //   Sprite(5) > RBG0(4) > NBG0(3) > NBG1(2) > NBG2(1) > NBG3(0)

    // NBG3: rank 0
    float nbg3Priority = floor(mod(PRINB / 256.0, 8.0));
    if (mod(BGON, 16.0) >= 8.0 && nbg3Color.a > 0.0)
    {
        float key = nbg3Priority * 8.0;
        if (key > bestKey)
        {
            bestKey = key;
            result = vec4(nbg3Color.rgb, 1.0);
        }
    }

    // NBG1: rank 2
    float nbg1Priority = floor(mod(PRINA / 256.0, 8.0));
    if (mod(BGON, 4.0) >= 2.0 && nbg1Color.a > 0.0)
    {
        float key = nbg1Priority * 8.0 + 2.0;
        if (key > bestKey)
        {
            bestKey = key;
            result = vec4(nbg1Color.rgb, 1.0);
        }
    }

    // NBG0: rank 3
    float nbg0Priority = floor(mod(PRINA, 8.0));
    if (mod(BGON, 2.0) >= 1.0 && nbg0Color.a > 0.0)
    {
        float key = nbg0Priority * 8.0 + 3.0;
        if (key > bestKey)
        {
            bestKey = key;
            result = vec4(nbg0Color.rgb, 1.0);
        }
    }

    // RBG0: rank 4
    float rbg0Priority = floor(mod(PRIR, 8.0));
    if (mod(BGON, 32.0) >= 16.0 && rbg0Color.a > 0.0)
    {
        float key = rbg0Priority * 8.0 + 4.0;
        if (key > bestKey)
        {
            bestKey = key;
            result = vec4(rbg0Color.rgb, 1.0);
        }
    }

    // Sprite (VDP1): rank 5
    if (vdp1Color.a > 0.0)
    {
        float key = vdp1Priority * 8.0 + 5.0;
        if (key > bestKey)
        {
            bestKey = key;
            result = vec4(vdp1Color.rgb, 1.0);
        }
    }

    gl_FragColor = result;
}
