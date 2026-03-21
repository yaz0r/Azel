$input v_texcoord0, v_color0, v_clipPos, v_quadIndex

#include "bgfx_shader.sh"

SAMPLER2D(s_texture, 0);
SAMPLER2D(s_ndcCorners, 1);
SAMPLER2D(s_objCorners, 2);

uniform vec4 u_spritePriority;
uniform vec4 u_quadCornersParams; // x = texture width (same for both corners textures)
uniform mat4 u_invModelViewProj;

float cross2d(vec2 a, vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

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
            return vec2(-1.0, -1.0);
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

vec4 fetchNdc(float texW, int idx)
{
    return texture2D(s_ndcCorners, vec2((float(idx) + 0.5) / texW, 0.5));
}

vec4 fetchObj(float texW, int idx)
{
    return texture2D(s_objCorners, vec2((float(idx) + 0.5) / texW, 0.5));
}

// NDC path: perspective-correct bilinear using u/w, v/w, 1/w
vec2 ndcPath(vec2 fragNDC, float texW, int base)
{
    vec4 c0 = fetchNdc(texW, base + 0);
    vec4 c1 = fetchNdc(texW, base + 1);
    vec4 c2 = fetchNdc(texW, base + 2);
    vec4 c3 = fetchNdc(texW, base + 3);
    vec4 wData = fetchNdc(texW, base + 4);

    vec2 st = inverseBilinear(fragNDC, c0.xy, c1.xy, c2.xy, c3.xy);
    st = clamp(st, vec2_splat(0.0), vec2_splat(1.0));

    vec2 uvOverW = mix(mix(c0.zw, c1.zw, st.x), mix(c3.zw, c2.zw, st.x), st.y);
    float oneOverW = mix(mix(wData.x, wData.y, st.x), mix(wData.w, wData.z, st.x), st.y);

    return uvOverW / oneOverW;
}

// Raytrace path: ray-plane intersection in object space
vec2 raytracePath(vec2 fragNDC, float texW, int base, vec2 fallbackUV)
{
    // Reconstruct ray in object space
    vec4 nearObj4 = mul(u_invModelViewProj, vec4(fragNDC, -1.0, 1.0));
    vec4 farObj4  = mul(u_invModelViewProj, vec4(fragNDC,  1.0, 1.0));
    vec3 rayOrig = nearObj4.xyz / nearObj4.w;
    vec3 rayDir  = farObj4.xyz / farObj4.w - rayOrig;

    // Fetch object-space corners
    vec4 t0 = fetchObj(texW, base + 0);
    vec4 t1 = fetchObj(texW, base + 1);
    vec4 t2 = fetchObj(texW, base + 2);
    vec4 t3 = fetchObj(texW, base + 3);
    vec4 tV = fetchObj(texW, base + 4);

    vec3 p0 = t0.xyz, p1 = t1.xyz, p2 = t2.xyz, p3 = t3.xyz;

    // Robust normal from diagonals
    vec3 normal = cross(p2 - p0, p3 - p1);
    float denom = dot(rayDir, normal);

    if (abs(denom) < 0.00001)
        return fallbackUV; // fallback

    float t = dot(p0 - rayOrig, normal) / denom;
    vec3 hitPoint = rayOrig + t * rayDir;

    // Project to 2D: drop axis with largest normal component
    vec3 absN = abs(normal);
    vec2 hit2d, q0, q1, q2, q3;
    if (absN.z >= absN.x && absN.z >= absN.y)
    {
        hit2d = hitPoint.xy; q0 = p0.xy; q1 = p1.xy; q2 = p2.xy; q3 = p3.xy;
    }
    else if (absN.y >= absN.x)
    {
        hit2d = hitPoint.xz; q0 = p0.xz; q1 = p1.xz; q2 = p2.xz; q3 = p3.xz;
    }
    else
    {
        hit2d = hitPoint.yz; q0 = p0.yz; q1 = p1.yz; q2 = p2.yz; q3 = p3.yz;
    }

    vec2 st = inverseBilinear(hit2d, q0, q1, q2, q3);
    st = clamp(st, vec2_splat(0.0), vec2_splat(1.0));

    vec2 uv0 = vec2(t0.w, tV.x);
    vec2 uv1 = vec2(t1.w, tV.y);
    vec2 uv2 = vec2(t2.w, tV.z);
    vec2 uv3 = vec2(t3.w, tV.w);
    return mix(mix(uv0, uv1, st.x), mix(uv3, uv2, st.x), st.y);
}

void main()
{
    vec2 fragNDC = v_clipPos.xy / v_clipPos.z;

    int quadIdx = int(v_quadIndex + 0.5);
    float texW = u_quadCornersParams.x;
    int base = quadIdx * 5;

    // Check sentinel: if all 1/w values are zero, this quad crosses near clip
    vec4 wData = fetchNdc(texW, base + 4);
    bool nearClip = (wData.x == 0.0 && wData.y == 0.0 && wData.z == 0.0 && wData.w == 0.0);

    vec2 uv;
    if (nearClip)
        uv = raytracePath(fragNDC, texW, base, v_texcoord0);
    else
        uv = ndcPath(fragNDC, texW, base);

    // Sample texture (UVs in pixel coords)
    vec2 UV = uv / vec2(textureSize(s_texture, 0));
    vec4 txcol = texture2D(s_texture, UV);

    if(txcol.a <= 0.f) discard;
    gl_FragColor = vec4(txcol.rgb, (u_spritePriority.x + 1.0) / 8.0);
}
