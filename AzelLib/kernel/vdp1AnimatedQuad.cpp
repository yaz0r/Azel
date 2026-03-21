#include "PDS.h"
#include "vdp1AnimatedQuad.h"

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
        quad[2] = quad[0] + MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale, MTH_Mul(pQuad.mC_width, scale), proj);
        quad[3] = quad[1] + MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, MTH_Mul(pQuad.m10_height, scale), proj);

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
