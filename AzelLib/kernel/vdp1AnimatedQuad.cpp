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
