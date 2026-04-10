$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_vdp1Color,    0);
SAMPLER2D(s_nbg0,         1);
SAMPLER2D(s_nbg1,         2);
SAMPLER2D(s_nbg3,         3);
SAMPLER2D(s_rbg0,         4);

uniform vec4 u_backColor;
uniform vec4 u_vdp2Regs0; // x=BGON, y=PRINA, z=PRINB, w=PRIR
uniform vec4 u_vdp2Regs1; // x=CCCTL, y=CCRNA, z=SFCCMD, w=SFCODE
uniform vec4 u_vdp2Regs2; // x=CCRNB, y=CCRR, z=unused, w=unused

void main()
{
    vec2 uv = v_texcoord0;

    float BGON  = u_vdp2Regs0.x;
    float PRINA = u_vdp2Regs0.y;
    float PRINB = u_vdp2Regs0.z;
    float PRIR  = u_vdp2Regs0.w;

    float CCCTL = u_vdp2Regs1.x;
    float CCRNA = u_vdp2Regs1.y;
    float SFCCMD = u_vdp2Regs1.z;
    float SFCODE = u_vdp2Regs1.w;
    float CCRNB = u_vdp2Regs2.x;
    float CCRR = u_vdp2Regs2.y;

    // Color calculation enable bits from CCCTL
    float nbg0CC = step(1.0, mod(CCCTL, 2.0));
    float nbg1CC = step(1.0, mod(floor(CCCTL / 2.0), 2.0));
    float rbg0CC = step(1.0, mod(floor(CCCTL / 16.0), 2.0));

    // CCMD: bit 8 of CCCTL (0 = ratio mode, 1 = add mode)
    float ccmd = step(1.0, mod(floor(CCCTL / 256.0), 2.0));

    // Color calculation ratios (0-31 range, normalized to 0-1)
    // TODO: This is likely incorrect, because a value of 0 should mean 31:1, and 32 should mean 0:32
    float nbg0Ratio = 1.f - (mod(CCRNA, 32.0) / 31.0);
    float nbg1Ratio = 1.f - (mod(CCRNB, 32.0) / 31.0);
    float rgb0Ratio = 1.f - (mod(CCRR, 32.0) / 31.0);

    // Special color calculation mode for NBG1 (bits 2-3 of SFCCMD)
    float nbg1SCCM = mod(floor(SFCCMD / 4.0), 4.0);

    // SFCODE threshold for NBG1 per-dot CC (upper nibble of code B)
    float nbg1SPCCN = floor(floor(SFCODE / 256.0) / 16.0);

    // Sample all layers
    vec4 vdp1Color = texture2D(s_vdp1Color, uv);
    vec4 nbg0Color = texture2D(s_nbg0, uv);
    vec4 nbg1Color = texture2D(s_nbg1, uv);
    vec4 nbg3Color = texture2D(s_nbg3, uv);
    vec4 rbg0Color = texture2D(s_rbg0, uv);

    // Decode VDP1 priority from alpha
    float vdp1Priority = floor(vdp1Color.a * 8.0 - 0.5);

    // Extract per-layer priorities
    float nbg0Priority = floor(mod(PRINA, 8.0));
    float nbg1Priority = floor(mod(PRINA / 256.0, 8.0));
    float nbg3Priority = floor(mod(PRINB / 256.0, 8.0));
    float rbg0Priority = floor(mod(PRIR, 8.0));

    // Layer keys: priority * 8 + tieBreakRank, -1 if disabled/transparent
    //   Sprite(5) > RBG0(4) > NBG0(3) > NBG1(2) > NBG3(0)
    float nbg3Key = (mod(BGON, 16.0) >= 8.0 && nbg3Color.a > 0.1) ? nbg3Priority * 8.0 + 0.0 : -1.0;
    float nbg1Key = (mod(BGON, 4.0) >= 2.0 && nbg1Color.a > 0.1)  ? nbg1Priority * 8.0 + 2.0 : -1.0;
    float nbg0Key = (mod(BGON, 2.0) >= 1.0 && nbg0Color.a > 0.1)  ? nbg0Priority * 8.0 + 3.0 : -1.0;
    float rbg0Key = (mod(BGON, 32.0) >= 16.0 && rbg0Color.a > 0.1) ? rbg0Priority * 8.0 + 4.0 : -1.0;
    float vdp1Key = (vdp1Color.a > 0.1) ? vdp1Priority * 8.0 + 5.0 : -1.0;

    // Composite back-to-front: 5 passes, each finds and consumes the lowest key
    vec4 result = u_backColor;

    for (int ci = 0; ci < 5; ci++)
    {
        // Find lowest valid key
        float minKey = 9999.0;
        int minIdx = -1;
        if (nbg3Key >= 0.0 && nbg3Key < minKey) { minKey = nbg3Key; minIdx = 0; }
        if (nbg1Key >= 0.0 && nbg1Key < minKey) { minKey = nbg1Key; minIdx = 1; }
        if (nbg0Key >= 0.0 && nbg0Key < minKey) { minKey = nbg0Key; minIdx = 2; }
        if (rbg0Key >= 0.0 && rbg0Key < minKey) { minKey = rbg0Key; minIdx = 3; }
        if (vdp1Key >= 0.0 && vdp1Key < minKey) { minKey = vdp1Key; minIdx = 4; }

        if (minIdx == 0)
        {
            result = vec4(nbg3Color.rgb, 1.0);
            nbg3Key = -1.0;
        }
        else if (minIdx == 1)
        {
            // NBG1: per-dot special CC
            if (nbg1SCCM > 0.0 && nbg1SPCCN > 0.0)
            {
                float dotColor = (nbg1Color.a - 0.5) * 32.0;
                if (dotColor >= nbg1SPCCN)
                {
                    if (ccmd > 0.0)
                        result = vec4(min(nbg1Color.rgb + result.rgb * nbg1Ratio, vec3(1.0, 1.0, 1.0)), 1.0);
                    else
                        result = vec4(mix(result.rgb, nbg1Color.rgb, nbg1Ratio), 1.0);
                }
                else
                {
                    result = vec4(nbg1Color.rgb, 1.0);
                }
            }
            else if (nbg1SCCM < 1.0 && nbg1CC > 0.0)
            {
                float nbg1Ratio = mod(floor(CCRNA / 256.0), 32.0) / 31.0;
                result = vec4(mix(result.rgb, nbg1Color.rgb, nbg1Ratio), 1.0);
            }
            else
            {
                result = vec4(nbg1Color.rgb, 1.0);
            }
            nbg1Key = -1.0;
        }
        else if (minIdx == 2)
        {
            // NBG0: global CC
            if (nbg0CC > 0.0)
            {
                if (ccmd > 0.0)
                    result = vec4(min(nbg0Color.rgb + result.rgb * nbg0Ratio, vec3(1.0, 1.0, 1.0)), 1.0);
                else
                    result = vec4(mix(result.rgb, nbg0Color.rgb, nbg0Ratio), 1.0);
            }
            else
            {
                result = vec4(nbg0Color.rgb, 1.0);
            }
            nbg0Key = -1.0;
        }
        else if (minIdx == 3)
        {
            // RBG0
            if (rbg0CC > 0.0)
            {
                if (ccmd > 0.0)
                    result = vec4(min(rbg0Color.rgb + result.rgb * rgb0Ratio, vec3(1.0, 1.0, 1.0)), 1.0);
                else
                    result = vec4(mix(result.rgb, rbg0Color.rgb, rgb0Ratio), 1.0);
            }
            else
            {
                result = vec4(rbg0Color.rgb, 1.0);
            }
            rbg0Key = -1.0;
        }
        else if (minIdx == 4)
        {
            result = vec4(vdp1Color.rgb, 1.0);
            vdp1Key = -1.0;
        }
        else
        {
            break;
        }
    }

    gl_FragColor = result;
}
