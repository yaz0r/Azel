#include "PDS.h"
#include "vdp1AnimatedQuad.h"
#include "3dEngine_textureCache.h"
#include "renderer.h"
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

struct sParticleBillboard
{
    float positions[3]; // view-space position
    float halfWidth;
    float halfHeight;
    float color[4];     // gouraud tint (RGBA float)
    u16 CMDPMOD;
    u16 CMDCOLR;
    u16 CMDSRCA;
    u16 CMDSIZE;
};

static std::vector<sParticleBillboard> gPendingParticleBillboards;

void particleInitSub(sAnimatedQuad* pThis, u16 vdp1Memory, const std::vector<sVdp1Quad>* param_3) {
    pThis->m6 = 0;
    pThis->m7_currentFrame = 0;
    pThis->m0_quad = param_3;
    pThis->m4_vdp1Memory = vdp1Memory;
}

int sGunShotTask_UpdateSub4(sAnimatedQuad* pThis) {
    const sVdp1Quad& pQuad = pThis->m0_quad->at(pThis->m7_currentFrame);

    int returnValue = 0;
    if (pQuad.m1 <= pThis->m6++) {
        pThis->m6 = 0; // reset?
        returnValue = 1;
        if (pQuad.m0_isLast == 1) {
            pThis->m7_currentFrame = 0;
            returnValue = 3;
        }
        else {
            pThis->m7_currentFrame++;
        }
    }
    return returnValue;
}


int clipQuad(std::array<fixedPoint, 4>& quad) {
    Unimplemented();
    return 0;
}

int drawProjectedParticle(sAnimatedQuad* pThis, sVec3_FP* position) {
    const sVdp1Quad& pQuad = pThis->m0_quad->at(pThis->m7_currentFrame);

    sVec3_FP projectedVector;
    transformAndAddVecByCurrentMatrix(position, &projectedVector);

    if ((graphicEngineStatus.m405C.m10_nearClipDistance < projectedVector[2]) && (projectedVector[2] < (int)graphicEngineStatus.m405C.m14_farClipDistance)) {
        fixedPoint proj = FP_Div(0x10000, projectedVector[2]);

        std::array<fixedPoint, 4> quad;
        quad[0] = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale, projectedVector[0] + pQuad.m14_X, proj);
        quad[1] = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, projectedVector[1] + pQuad.m18_Y, proj);
        quad[2] = quad[0] + MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale, pQuad.mC_width, proj);
        quad[3] = quad[1] + MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, pQuad.m10_height, proj);

        if (clipQuad(quad) != 2) {

            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = pQuad.m2_CMDCTRL;
            vdp1WriteEA.m4_CMDPMOD = pQuad.m4_CMDPMOD | 0x400;
            if ((pQuad.m4_CMDPMOD & 0x38) == 8) {
                vdp1WriteEA.m6_CMDCOLR = pThis->m4_vdp1Memory + pQuad.mA_CMDCOLR;
            }
            else {
                vdp1WriteEA.m6_CMDCOLR = pQuad.mA_CMDCOLR;
            }

            vdp1WriteEA.m8_CMDSRCA = pThis->m4_vdp1Memory + pQuad.m6_CMDSRCA;
            vdp1WriteEA.mA_CMDSIZE = pQuad.m8_CMDSIZE;
            vdp1WriteEA.mC_CMDXA = quad[0];
            vdp1WriteEA.mE_CMDYA = -quad[1];
            vdp1WriteEA.m14_CMDXC = quad[2];
            vdp1WriteEA.m16_CMDYC = -quad[3];

            //s_vd1ExtendedCommand* pExtendedCommand = createVdp1ExtendedCommand(vdp1WriteEA);
            //pExtendedCommand->depth = (float)proj.asS32() / (float)graphicEngineStatus.m405C.m14_farClipDistance.asS32();

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = fixedPoint(proj * graphicEngineStatus.m405C.m38_oneOverFarClip).getInteger();
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            return 1;
        }
    }
    return 0;
}

int vdp1DrawQuadScaled(sAnimatedQuad* pThis, sVec3_FP* position, fixedPoint scale) {
    const sVdp1Quad& pQuad = pThis->m0_quad->at(pThis->m7_currentFrame);

    sVec3_FP projectedVector;
    transformAndAddVecByCurrentMatrix(position, &projectedVector);

    if ((graphicEngineStatus.m405C.m10_nearClipDistance < projectedVector[2]) && (projectedVector[2] < (int)graphicEngineStatus.m405C.m14_farClipDistance)) {
        fixedPoint proj = FP_Div(0x10000, projectedVector[2]);

        std::array<fixedPoint, 4> quad;
        quad[0] = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale, projectedVector[0] + MTH_Mul(pQuad.m14_X, scale), proj);
        quad[1] = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, projectedVector[1] + MTH_Mul(pQuad.m18_Y, scale), proj);
        quad[2] = (s16)MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale, MTH_Mul(pQuad.mC_width, scale), proj).getInteger() + quad[0].getInteger();
        quad[3] = quad[1].getInteger() - (s16)MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, MTH_Mul(pQuad.m10_height, scale), proj).getInteger();

        if (clipQuad(quad) != 2) {

            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = pQuad.m2_CMDCTRL;
            vdp1WriteEA.m4_CMDPMOD = pQuad.m4_CMDPMOD | 0x400;
            if ((pQuad.m4_CMDPMOD & 0x38) == 8) {
                vdp1WriteEA.m6_CMDCOLR = pThis->m4_vdp1Memory + pQuad.mA_CMDCOLR;
            }
            else {
                vdp1WriteEA.m6_CMDCOLR = pQuad.mA_CMDCOLR;
            }

            vdp1WriteEA.m8_CMDSRCA = pThis->m4_vdp1Memory + pQuad.m6_CMDSRCA;
            vdp1WriteEA.mA_CMDSIZE = pQuad.m8_CMDSIZE;
            vdp1WriteEA.mC_CMDXA = quad[0];
            vdp1WriteEA.mE_CMDYA = -quad[1];
            vdp1WriteEA.m14_CMDXC = quad[2];
            vdp1WriteEA.m16_CMDYC = -quad[3];

            //s_vd1ExtendedCommand* pExtendedCommand = createVdp1ExtendedCommand(vdp1WriteEA);
            //pExtendedCommand->depth = (float)proj.asS32() / (float)graphicEngineStatus.m405C.m14_farClipDistance.asS32();

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = fixedPoint(proj * graphicEngineStatus.m405C.m38_oneOverFarClip).getInteger();
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            return 1;
        }
    }
    return 0;
}

// 0602d0dc
int drawProjectedParticleWithGouraud(sAnimatedQuad* pQuad, sVec3_FP* position, u16* gouraudColors)
{
    const sVdp1Quad& quad = pQuad->m0_quad->at(pQuad->m7_currentFrame);

    sVec3_FP viewPos;
    transformAndAddVecByCurrentMatrix(position, &viewPos);

    if ((graphicEngineStatus.m405C.m10_nearClipDistance < viewPos[2]) &&
        (viewPos[2] < (int)graphicEngineStatus.m405C.m14_farClipDistance))
    {
        sParticleBillboard bb;
        bb.positions[0] = (float)viewPos[0].asS32() / (float)0x10000;
        bb.positions[1] = (float)viewPos[1].asS32() / (float)0x10000;
        bb.positions[2] = (float)viewPos[2].asS32() / (float)0x10000;
        bb.halfWidth = (float)quad.mC_width.asS32() / (float)0x10000 * 0.5f;
        bb.halfHeight = (float)quad.m10_height.asS32() / (float)0x10000 * 0.5f;
        bb.CMDPMOD = quad.m4_CMDPMOD;
        bb.CMDSRCA = pQuad->m4_vdp1Memory + quad.m6_CMDSRCA;
        bb.CMDSIZE = quad.m8_CMDSIZE;
        if ((quad.m4_CMDPMOD & 0x38) == 8)
            bb.CMDCOLR = pQuad->m4_vdp1Memory + quad.mA_CMDCOLR;
        else
            bb.CMDCOLR = quad.mA_CMDCOLR;

        if (gouraudColors)
        {
            // Saturn gouraud: RGB555 as signed offset centered at 16
            u16 gc = gouraudColors[0];
            bb.color[0] = ((float)((gc >> 0) & 0x1F) - 16.0f) / 16.0f;
            bb.color[1] = ((float)((gc >> 5) & 0x1F) - 16.0f) / 16.0f;
            bb.color[2] = ((float)((gc >> 10) & 0x1F) - 16.0f) / 16.0f;
            bb.color[3] = 1.0f;
        }
        else
        {
            bb.color[0] = bb.color[1] = bb.color[2] = bb.color[3] = 0.0f;
        }

        gPendingParticleBillboards.push_back(bb);
        return 1;
    }
    return 0;
}

void flushParticleBillboards()
{
    if (gPendingParticleBillboards.empty()) return;

    glm::mat4 getProjectionMatrix();
    bgfx::ProgramHandle loadBgfxProgram(const std::string& VSFile, const std::string& PSFile);
    bgfx::TextureHandle getTextureForQuadBGFX(s_quad& quad);

    static bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(program))
    {
        program = loadBgfxProgram("VDP1_worldTextured_vs", "VDP1_worldTextured_ps");
    }

    static bgfx::UniformHandle u_mvp = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(u_mvp))
    {
        u_mvp = bgfx::createUniform("u_customModelViewProj", bgfx::UniformType::Mat4);
    }

    static bgfx::UniformHandle u_priority = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(u_priority))
    {
        u_priority = bgfx::createUniform("u_spritePriority", bgfx::UniformType::Vec4);
    }

    static bgfx::UniformHandle u_texture = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(u_texture))
    {
        u_texture = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
    }

    struct BillboardVertex
    {
        float pos[3];
        float uv[2];
        float color[4];
    };

    bgfx::VertexLayout layout;
    layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();

    glm::mat4 projMatrix = getProjectionMatrix();
    u16 indices[6] = { 2, 1, 0, 0, 3, 2 };

    for (auto& bb : gPendingParticleBillboards)
    {
        bgfx::TransientVertexBuffer vb;
        bgfx::TransientIndexBuffer ib;
        if (!bgfx::allocTransientBuffers(&vb, layout, 4, &ib, 6))
            break;

        float cx = bb.positions[0];
        float cy = bb.positions[1];
        float cz = bb.positions[2];
        float hw = bb.halfWidth;
        float hh = bb.halfHeight;

        // Camera-facing billboard in view space (camera looks down +Z)
        float r = bb.color[0], g = bb.color[1], b = bb.color[2], a = bb.color[3];
        BillboardVertex verts[4] = {
            { {cx - hw, cy + hh, cz}, {0, 0}, {r, g, b, a} },
            { {cx + hw, cy + hh, cz}, {1, 0}, {r, g, b, a} },
            { {cx + hw, cy - hh, cz}, {1, 1}, {r, g, b, a} },
            { {cx - hw, cy - hh, cz}, {0, 1}, {r, g, b, a} },
        };

        memcpy(vb.data, verts, sizeof(verts));
        memcpy(ib.data, indices, sizeof(indices));

        s_quad tempQuad = {};
        tempQuad.CMDPMOD = bb.CMDPMOD;
        tempQuad.CMDCOLR = bb.CMDCOLR;
        tempQuad.CMDSRCA = bb.CMDSRCA;
        tempQuad.CMDSIZE = bb.CMDSIZE;

        bgfx::TextureHandle tex = getTextureForQuadBGFX(tempQuad);
        if (!bgfx::isValid(tex))
            continue;

        bgfx::setUniform(u_mvp, &projMatrix[0][0]);

        float priority[4] = { (float)(vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA & 7), 0, 0, 0 };
        bgfx::setUniform(u_priority, priority);

        bgfx::setTexture(0, u_texture, tex);
        bgfx::setVertexBuffer(0, &vb);
        bgfx::setIndexBuffer(&ib);

        bgfx::setState(0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA
        );

        bgfx::submit(vdp1_gpuView, program);
    }

    gPendingParticleBillboards.clear();
}

// 0602f610
int drawImmediateBillboardSprite(const sVec3_FP* points, const sBillboardSpriteParams* params)
{
    // Transform both points (position + up reference) to view space
    sVec3_FP viewPos0, viewPos1;
    transformAndAddVecByCurrentMatrix(&points[0], &viewPos0);
    transformAndAddVecByCurrentMatrix(&points[1], &viewPos1);

    // Near clip: both behind = reject
    s32 nearClip = (s32)graphicEngineStatus.m405C.m10_nearClipDistance;
    if ((s32)viewPos0[2] < nearClip && (s32)viewPos1[2] < nearClip)
        return 2;

    // Far clip: both beyond = reject
    s32 farClip = (s32)graphicEngineStatus.m405C.m14_farClipDistance;
    if ((s32)viewPos0[2] > farClip && (s32)viewPos1[2] > farClip)
        return 2;

    // Clip against near plane if one point is behind
    sVec3_FP clipped0 = viewPos0, clipped1 = viewPos1;
    if ((s32)clipped0[2] < nearClip)
    {
        fixedPoint dz = clipped0[2] - clipped1[2];
        if ((s32)dz != 0)
        {
            clipped0[0] = setDividend((s32)(clipped0[0] - clipped1[0]), nearClip - (s32)clipped1[2], (s32)dz) + clipped1[0];
            clipped0[1] = setDividend((s32)(clipped0[1] - clipped1[1]), nearClip - (s32)clipped1[2], (s32)dz) + clipped1[1];
        }
        clipped0[2] = nearClip;
    }
    else if ((s32)clipped1[2] < nearClip)
    {
        fixedPoint dz = clipped1[2] - clipped0[2];
        if ((s32)dz != 0)
        {
            clipped1[0] = setDividend((s32)(clipped1[0] - clipped0[0]), nearClip - (s32)clipped0[2], (s32)dz) + clipped0[0];
            clipped1[1] = setDividend((s32)(clipped1[1] - clipped0[1]), nearClip - (s32)clipped0[2], (s32)dz) + clipped0[1];
        }
        clipped1[2] = nearClip;
    }

    // Compute screen-space angle between the two projected points for billboard rotation
    s32 angle = atan2(clipped0[1] - clipped1[1], clipped0[0] - clipped1[0]) & 0xFFF;
    fixedPoint sinA = getSin(angle);
    fixedPoint cosA = getCos(angle);

    // Compute billboard corner offsets using sin/cos rotation
    fixedPoint dx0 = MTH_Mul(params->m0_halfWidth, sinA - cosA);
    fixedPoint dx1 = MTH_Mul(params->m0_halfWidth, sinA + cosA);
    fixedPoint dy0 = MTH_Mul(params->m4_halfHeight, sinA - cosA);
    fixedPoint dy1 = MTH_Mul(params->m4_halfHeight, sinA + cosA);

    // Perspective project point 0
    fixedPoint invZ0 = FP_Div(0x10000, clipped0[2]);
    s32 sx0 = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale,  clipped0[0] - dx0, invZ0).getInteger();
    s32 sx1 = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale,  clipped0[0] + dx1, invZ0).getInteger();
    s32 sy0 = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, clipped0[1] + dx1, invZ0).getInteger();
    s32 sy1 = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, clipped0[1] + dx0, invZ0).getInteger();

    // Perspective project point 1
    fixedPoint invZ1 = FP_Div(0x10000, clipped1[2]);
    s32 sx2 = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale,  clipped1[0] + dy0, invZ1).getInteger();
    s32 sx3 = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale,  clipped1[0] - dy1, invZ1).getInteger();
    s32 sy2 = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, clipped1[1] - dy1, invZ1).getInteger();
    s32 sy3 = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, clipped1[1] - dy0, invZ1).getInteger();

    // Screen clip: clamp all corners to VDP1 clip area
    s16 xMin = graphicEngineStatus.m405C.VDP1_X1, xMax = graphicEngineStatus.m405C.VDP1_X2;
    s16 yMin = graphicEngineStatus.m405C.VDP1_Y1, yMax = graphicEngineStatus.m405C.VDP1_Y2;
    auto clampS16 = [](s32 v, s16 lo, s16 hi) -> s16 {
        if (v < lo) return lo; if (v > hi) return hi; return (s16)v;
    };

    // Write VDP1 distorted sprite command
    s_vdp1Command& cmd = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    cmd.m0_CMDCTRL = 0x1002; // distorted sprite
    cmd.m4_CMDPMOD = params->m8_CMDPMOD | 0x84;
    cmd.m8_CMDSRCA = params->mC_CMDSRCA;
    cmd.mA_CMDSIZE = params->mE_CMDSIZE;
    cmd.m6_CMDCOLR = params->mA_CMDCOLR;
    cmd.mC_CMDXA  = clampS16(sx0, xMin, xMax);
    cmd.mE_CMDYA  = -clampS16(sy0, yMin, yMax);
    cmd.m10_CMDXB = clampS16(sx1, xMin, xMax);
    cmd.m12_CMDYB = -clampS16(sy1, yMin, yMax);
    cmd.m14_CMDXC = clampS16(sx2, xMin, xMax);
    cmd.m16_CMDYC = -clampS16(sy2, yMin, yMax);
    cmd.m18_CMDXD = clampS16(sx3, xMin, xMax);
    cmd.m1A_CMDYD = -clampS16(sy3, yMin, yMax);

    // Depth / priority
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes =
        fixedPoint(invZ1 * graphicEngineStatus.m405C.m38_oneOverFarClip).getInteger();
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &cmd;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;

    return 0;
}
