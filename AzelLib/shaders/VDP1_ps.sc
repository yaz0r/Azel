$input v_texcoord0, v_color0, v_clipPos, v_quadIndex

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);
SAMPLER2D(s_quadCorners, 1);

uniform vec4 u_spritePriority;
uniform vec4 u_quadCornersParams; // x = texture width

float cross2d(vec2 a, vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

// Compute u from v using the more numerically stable axis
float computeU(vec2 h, vec2 e, vec2 f, vec2 g, float v)
{
    vec2 num = h - f * v;
    vec2 den = e + g * v;
    if (abs(den.x) > abs(den.y))
        return num.x / den.x;
    else
        return num.y / den.y;
}

vec2 inverseBilinear(vec2 p, vec2 a, vec2 b, vec2 c, vec2 d)
{
    vec2 e = b - a;
    vec2 f = d - a;
    vec2 g = a - b + c - d;
    vec2 h = p - a;

    float k2 = cross2d(g, f);
    float k1 = cross2d(e, f) + cross2d(h, g);
    float k0 = cross2d(h, e);

    float u, v;

    if (abs(k2) < 0.0001)
    {
        if (abs(k1) < 0.0001)
            return vec2(0.5, 0.5);
        v = -k0 / k1;
        u = computeU(h, e, f, g, v);
        return vec2(u, v);
    }

    float disc = k1 * k1 - 4.0 * k0 * k2;
    if (disc < 0.0)
        disc = 0.0;
    disc = sqrt(disc);

    float v1 = (-k1 - disc) / (2.0 * k2);
    float v2 = (-k1 + disc) / (2.0 * k2);

    float u1 = computeU(h, e, f, g, v1);
    float u2 = computeU(h, e, f, g, v2);

    float d1 = max(max(-u1, u1 - 1.0), max(-v1, v1 - 1.0));
    float d2 = max(max(-u2, u2 - 1.0), max(-v2, v2 - 1.0));

    if (d1 <= d2)
        return vec2(u1, v1);
    return vec2(u2, v2);
}

vec4 fetchCorner(float texW, int idx)
{
    return texture2D(s_quadCorners, vec2((float(idx) + 0.5) / texW, 0.5));
}

void main()
{
    vec2 fragNDC = v_clipPos.xy / v_clipPos.z;

    // Texture layout: 5 texels per quad
    //   0-3: (ndcX, ndcY, u/w, v/w) per corner
    //   4:   (1/w0, 1/w1, 1/w2, 1/w3)
    int quadIdx = int(v_quadIndex + 0.5);
    float texW = u_quadCornersParams.x;
    int base = quadIdx * 5;

    vec4 c0 = fetchCorner(texW, base + 0);
    vec4 c1 = fetchCorner(texW, base + 1);
    vec4 c2 = fetchCorner(texW, base + 2);
    vec4 c3 = fetchCorner(texW, base + 3);
    vec4 wData = fetchCorner(texW, base + 4);

    // Inverse bilinear in NDC space to find screen-space (s,t)
    vec2 st = inverseBilinear(fragNDC, c0.xy, c1.xy, c2.xy, c3.xy);
    st = clamp(st, vec2_splat(0.0), vec2_splat(1.0));

    // Perspective-correct UV interpolation:
    // Bilinearly interpolate u/w, v/w, and 1/w using screen-space (s,t),
    // then divide to recover perspective-correct UVs
    vec2 uvOverW = mix(mix(c0.zw, c1.zw, st.x), mix(c3.zw, c2.zw, st.x), st.y);
    float oneOverW = mix(mix(wData.x, wData.y, st.x), mix(wData.w, wData.z, st.x), st.y);

    vec2 uv = uvOverW / oneOverW;

    vec2 UV = uv / vec2(textureSize(s_texture, 0));
    vec4 txcol = texture2D(s_texture, UV);

    if(txcol.a <= 0.f) discard;
    gl_FragColor = vec4(txcol.rgb, (u_spritePriority.x + 1.0) / 8.0);
}
