#include "PDS.h"
#include "town.h"
#include "townLCS.h"
#include "kernel/fileBundle.h"
#include "processModel.h"

sResCameraProperties LCSCollisionData;

void copyCameraPropertiesToRes()
{
    LCSCollisionData.m8_LCSWidth = 2;
    LCSCollisionData.mC_LCSHeight = 2;
    LCSCollisionData.m10_activeLCSType = 0;
    LCSCollisionData.m18 = graphicEngineStatus.m405C.m30_oneOverNearClip;
    LCSCollisionData.m24_LCSDepthMin = graphicEngineStatus.m405C.m10_nearClipDistance;
    LCSCollisionData.m2C_projectionWidthScale = graphicEngineStatus.m405C.m18_widthScale;
    LCSCollisionData.m30_projectionHeightScale = graphicEngineStatus.m405C.m1C_heightScale;
}

void setupDataForLCSCollision()
{
    LCSCollisionData.m10_activeLCSType = 0;
    LCSCollisionData.m28_LCSDepthMax = graphicEngineStatus.m405C.m14_farClipDistance;
    LCSCollisionData.m1C_LCSHeightMin = -(LCSCollisionData.m4_LCS_Y.getInteger() - LCSCollisionData.mC_LCSHeight); // todo: there might be a bug here, used to be clipped to 16 bit
    LCSCollisionData.m1E_LCSHeightMax = -(LCSCollisionData.m4_LCS_Y.getInteger() + LCSCollisionData.mC_LCSHeight);
    LCSCollisionData.m20_LCSWidthMin = LCSCollisionData.m0_LCS_X.getInteger() - LCSCollisionData.m8_LCSWidth;
    LCSCollisionData.m22_LCSWidthMax = LCSCollisionData.m0_LCS_X.getInteger() + LCSCollisionData.m8_LCSWidth;

    s32 var0[2];
    var0[0] = LCSCollisionData.m20_LCSWidthMin;
    var0[1] = LCSCollisionData.m2C_projectionWidthScale;
    fixedPoint r12 = FP_Div(0x10000, LCSCollisionData.m2C_projectionWidthScale);
    LCSCollisionData.m34_boundMinX = MTH_Mul(LCSCollisionData.m20_LCSWidthMin, r12);
    LCSCollisionData.m38_radiusScaleMinX = MTH_Mul(sqrt_I(MTH_Product2d(var0, var0)), r12);

    var0[0] = LCSCollisionData.m22_LCSWidthMax;
    LCSCollisionData.m3C_boundMaxX = MTH_Mul(LCSCollisionData.m22_LCSWidthMax, r12);
    LCSCollisionData.m40_radiusScaleMaxX = MTH_Mul(sqrt_I(MTH_Product2d(var0, var0)), r12);

    // same for height
    var0[0] = -LCSCollisionData.m1C_LCSHeightMin;
    var0[1] = LCSCollisionData.m30_projectionHeightScale;
    r12 = FP_Div(0x10000, LCSCollisionData.m30_projectionHeightScale);
    LCSCollisionData.m44_boundMinY = MTH_Mul(-LCSCollisionData.m1C_LCSHeightMin, r12);
    LCSCollisionData.m48_radiusScaleMinY = MTH_Mul(sqrt_I(MTH_Product2d(var0, var0)), r12);

    var0[0] = -LCSCollisionData.m1E_LCSHeightMax;
    LCSCollisionData.m4C_boundMaxY = MTH_Mul(-LCSCollisionData.m1E_LCSHeightMax, r12);
    LCSCollisionData.m50_radiusScaleMaxY = MTH_Mul(sqrt_I(MTH_Product2d(var0, var0)), r12);

    LCSCollisionData.m54 = MTH_Mul(LCSCollisionData.m38_radiusScaleMinX, gTownGrid.m2C);
    LCSCollisionData.m58 = MTH_Mul(LCSCollisionData.m40_radiusScaleMaxX, gTownGrid.m2C);
}

const std::array<s8[2], 25> townGridSearchPattern = {
    {{ 0,    0},
    { 1,    0},
    { 0,    1},
    {-1,    0},
    { 0,   -1},
    { 1,    1},
    {-1,    1},
    {-1,   -1},
    { 1,   -1},
    { 2,    0},
    { 0,    2},
    {-2,    0},
    { 0,   -2},
    { 2,    1},
    {-1,    2},
    {-2,   -1},
    { 1,   -2},
    { 2,   -1},
    { 1,    2},
    {-2,    1},
    {-1,   -2},
    { 2,    2},
    {-2,    2},
    {-2,   -2},
    { 2,  -2}}
};

void scriptUpdateSub2Sub0(sResCameraProperties* r4, s32 r5)
{
    sVec3_FP var14;
    var14[0] = pCurrentMatrix->m[0][1];
    var14[1] = pCurrentMatrix->m[1][1];
    var14[2] = pCurrentMatrix->m[2][1];

    sVec2_FP var0;
    var0[0] = performDivision(r4->m2C_projectionWidthScale, r4->m0_LCS_X);
    var0[1] = performDivision(r4->m30_projectionHeightScale, r4->m4_LCS_Y);

    fixedPoint r12 = var14[2] + MulVec2(sVec2_FP({ var14[0], var14[1] }), var0);
    if (r12 >= 0)
    {
        return;
    }

    sVec3_FP var8;
    var8[0] = pCurrentMatrix->m[0][3] + MTH_Mul(r5, pCurrentMatrix->m[0][1]);
    var8[1] = pCurrentMatrix->m[1][3] + MTH_Mul(r5, pCurrentMatrix->m[1][1]);
    var8[2] = pCurrentMatrix->m[2][3] + MTH_Mul(r5, pCurrentMatrix->m[2][1]);

    fixedPoint r4_fp = FP_Div(MTH_Product3d_FP(var14, var8), r12);
    if (r4->m28_LCSDepthMax > r4_fp)
    {
        r4->m10_activeLCSType = 0;
        r4->m14_activeLCS = 0;
        r4->m28_LCSDepthMax = r4_fp;
    }
}

struct sTransformedVertice
{
    s16 m0_X;
    s16 m2_Y;
    s32 m4_fullPrecisionZ;
    s32 m8_fullPrecisionX;
    s32 mC_fullPrecisionY;
    s32 m10_clippedX;
    s32 m14_clippedY;
    u32 m18_clipFlags;
    u32 m1C_clipFlags2;
    // size 0x20
};

void computeFinalProjectionMatrix(const sProcessed3dModel& r4, const sMatrix4x3& r5, sResCameraProperties* r6, std::array<s16, 9> & outputMatrix, s32& r8_outputTranslationX, s32& r9_outputTranslationY, s32& r10_outputTranslationZ, s16*& r11, std::vector<sVec3_S16_12_4>::const_iterator& r12_pVertices, u32& r13_numVertices)
{
    //r6 is off by 0x14 compared to asm
    outputMatrix[0] = (r5.m[0][0] * (r6->m2C_projectionWidthScale * 16)) >> 16;
    outputMatrix[1] = (r5.m[0][1] * (r6->m2C_projectionWidthScale * 16)) >> 16;
    outputMatrix[2] = (r5.m[0][2] * (r6->m2C_projectionWidthScale * 16)) >> 16;

    r8_outputTranslationX = (r5.m[0][3] * r6->m2C_projectionWidthScale);

    outputMatrix[3] = (r5.m[1][0] * (-r6->m30_projectionHeightScale * 16)) >> 16;
    outputMatrix[4] = (r5.m[1][1] * (-r6->m30_projectionHeightScale * 16)) >> 16;
    outputMatrix[5] = (r5.m[1][2] * (-r6->m30_projectionHeightScale * 16)) >> 16;

    r9_outputTranslationY = (r5.m[1][3] * -r6->m30_projectionHeightScale);

    outputMatrix[6] = r5.m[2][0] >> 4;
    outputMatrix[7] = r5.m[2][1] >> 4;
    outputMatrix[8] = r5.m[2][2] >> 4;

    r10_outputTranslationZ = r5.m[2][3] << 8;

    r11 = &outputMatrix[6];
    r13_numVertices = r4.m4_numVertices;
    r12_pVertices = r4.m8_vertices.begin();

    // note: this also init DIV register
}

void transformVerticesClipped(const sProcessed3dModel& r4, const sMatrix4x3& r5, sResCameraProperties* r6, std::array<sTransformedVertice, 256> & r7_transformedVertices)
{
    std::array<s16, 9> projectionMatrix;
    s32 r8_translationX;
    s32 r9_translationY;
    s32 r10_translationZ;
    s16* dummy_r11;
    std::vector<sVec3_S16_12_4>::const_iterator dummy_r12;
    u32 r13_numVertices;
    computeFinalProjectionMatrix(r4, r5, r6, projectionMatrix, r8_translationX, r9_translationY, r10_translationZ, dummy_r11, dummy_r12, r13_numVertices);
    std::vector<sVec3_S16_12_4>::const_iterator r12_pVertices = r4.m8_vertices.begin(); // this is moved out of the previous function;
    // r14 is now 0xFFFFFF00 hardware

    std::array<sTransformedVertice, 256>::iterator r7 = r7_transformedVertices.begin();

    s64 mac = 0;
    s32 DVDNTUL = 0;
    s32 r1 = 0;
    s32 r0 = 0;
    do
    {
        //Z
        mac = r10_translationZ;
        mac += ((s64)(projectionMatrix[6]) * (s64)(r12_pVertices->m_value[0]));
        mac += ((s64)(projectionMatrix[7]) * (s64)(r12_pVertices->m_value[1]));
        mac += ((s64)(projectionMatrix[8]) * (s64)(r12_pVertices->m_value[2]));
        DVDNTUL = 0x10000000000 / (mac & 0xFFFFFFFF);
        r7->m4_fullPrecisionZ = mac;

        //X
        mac = r8_translationX;
        mac += ((s64)(projectionMatrix[0]) * (s64)(r12_pVertices->m_value[0]));
        mac += ((s64)(projectionMatrix[1]) * (s64)(r12_pVertices->m_value[1]));
        mac += ((s64)(projectionMatrix[2]) * (s64)(r12_pVertices->m_value[2]));
        r7->m8_fullPrecisionX = mac;

        //Y
        mac = r9_translationY;
        mac += ((s64)(projectionMatrix[3]) * (s64)(r12_pVertices->m_value[0]));
        mac += ((s64)(projectionMatrix[4]) * (s64)(r12_pVertices->m_value[1]));
        mac += ((s64)(projectionMatrix[5]) * (s64)(r12_pVertices->m_value[2]));
        r12_pVertices++;
        r7->mC_fullPrecisionY = mac;

        s32 r3 = r7->m4_fullPrecisionZ >= (r6->m28_LCSDepthMax << 8);
        r3 <<= 1;

        s32 depthClipped = r7->m4_fullPrecisionZ <= (r6->m24_LCSDepthMin << 8);
        r3 |= depthClipped;

        if (!depthClipped)
        {
            //0601F36C
            s32 r2 = ((s64)DVDNTUL * (s64)r7->mC_fullPrecisionY) >> 32;
            s32 r1 = ((s64)DVDNTUL * (s64)r7->m8_fullPrecisionX) >> 32;

            r7->m2_Y = r2;
            r7->m0_X = r1;

            r3 <<= 1;
            r3 |= r2 > r6->m1C_LCSHeightMin;
            r3 <<= 1;
            r3 |= r2 < r6->m1E_LCSHeightMax;
            r3 <<= 1;
            r3 |= r1 > r6->m22_LCSWidthMax;
            r3 <<= 1;
            r3 |= r1 < r6->m20_LCSWidthMin;
            r7->m18_clipFlags = r3;
        }
        else
        {
            r7->m18_clipFlags = r3 << 4;

        }
        r7++;
    } while (--r13_numVertices);
}
bool testQuadsForCollisionSub2(sResCameraProperties* r6, sTransformedVertice& r8, sTransformedVertice& r9)
{
    s32 r0 = (r6->m24_LCSDepthMin.asS32() << 8) - r8.m4_fullPrecisionZ;
    s32 r2 = r9.m4_fullPrecisionZ - r8.m4_fullPrecisionZ;

    s32 r1 = r0 / r2;
    r1 <<= 8;
    s64 mac = r1 * (r9.m8_fullPrecisionX - r8.m8_fullPrecisionX);
    r2 = mac >> 16;
    r2 += r8.m8_fullPrecisionX;

    mac = r1 * (r9.mC_fullPrecisionY - r8.mC_fullPrecisionY);
    s32 r3 = mac >> 16;
    r3 += r8.mC_fullPrecisionY;

    r8.m14_clippedY = r6->m18 * r3;
    r8.m10_clippedX = r6->m18 * r2;
    return false;
}

bool testQuadsForCollisionSub1(s32 r1, sResCameraProperties* r6, bool(*r2Callback)(sResCameraProperties* r6, sTransformedVertice&, sTransformedVertice&), sTransformedVertice& r10, sTransformedVertice& r11, sTransformedVertice& r12, sTransformedVertice& r13)
{
    if (r13.m1C_clipFlags2 & r1)
    {
        //601F7E0
        if (r12.m1C_clipFlags2 & r1)
        {
            //601F828
            if (r11.m1C_clipFlags2 & r1)
            {
                //0601F852
                if (r10.m1C_clipFlags2 & r1)
                {
                    // all clipped!
                    return true;
                }
                //601F852
                //r13 r12 r11 clipped
                r2Callback(r6, r11, r10);
                r2Callback(r6, r12, r10);
                return r2Callback(r6, r13, r10);
            }
            if (r10.m1C_clipFlags2 & r1)
            {
                //601F840
                //r13 r12 r10 clipped
                r2Callback(r6, r10, r11);
                r2Callback(r6, r13, r11);
                return r2Callback(r6, r12, r11);
            }
            //0601F830
            // r13 r12 clipped
            r2Callback(r6, r13, r10);
            return r2Callback(r6, r12, r11);
        }
        if (r11.m1C_clipFlags2 & r1)
        {
            //0601F810
            if (r10.m1C_clipFlags2 & r1)
            {
                //601F816
                //r13 r11 r10 clipped
                r2Callback(r6, r10, r12);
                r2Callback(r6, r13, r12);
                return r2Callback(r6, r11, r12);
            }
            //0601F812
            //r13 r11 clipped
            return true;
        }
        if (r10.m1C_clipFlags2 & r1)
        {
            //601F800
            // r13 r10 clipped
            r2Callback(r6, r10, r11);
            return r2Callback(r6, r13, r12);
        }

        // 601F7EE
        // only r13 clipped
        if (r12.m4_fullPrecisionZ > r10.m4_fullPrecisionZ)
        {
            return r2Callback(r6, r13, r12);
        }
        else
        {
            return r2Callback(r6, r13, r10);
        }
    }
    if (r12.m1C_clipFlags2 & r1)
    {
        //601F7A2
        if (r11.m1C_clipFlags2 & r1)
        {
            //601F7BC
            if (r10.m1C_clipFlags2 & r1)
            {
                //601F7CE
                // r12 r11 r10 clipped
                r2Callback(r6, r11, r13);
                r2Callback(r6, r12, r13);
                return r2Callback(r6, r10, r13);
            }
            //601F7BC
            // r12 r11 clipped
            r2Callback(r6, r11, r10);
            return r2Callback(r6, r12, r13);
        }

        if (r10.m1C_clipFlags2 & r1)
        {
            //601F812
            //r12 r10 clipped
            return true;
        }

        //601F7AA
        // only r12 clipped
        if (r13.m4_fullPrecisionZ > r11.m4_fullPrecisionZ)
        {
            return r2Callback(r6, r12, r13);
        }
        else
        {
            return r2Callback(r6, r12, r11);
        }
    }
    if (r11.m1C_clipFlags2 & r1)
    {
        //601F77E
        if (r10.m1C_clipFlags2 & r1)
        {
            //601F792
            // r11 r10 clipped
            r2Callback(r6, r10, r13);
            return r2Callback(r6, r11, r12);
        }
        //601F780
        // only 11 clipped
        if (r10.m4_fullPrecisionZ > r12.m4_fullPrecisionZ)
        {
            return r2Callback(r6, r11, r10);
        }
        else
        {
            return r2Callback(r6, r11, r12);
        }
    }
    if (r10.m1C_clipFlags2 & r1)
    {
        //601F76C
        // only r10 clipped
        if (r11.m4_fullPrecisionZ > r13.m4_fullPrecisionZ)
        {
            return r2Callback(r6, r10, r11);
        }
        else
        {
            return r2Callback(r6, r10, r13);
        }
    }

    return false;
}

bool setTBitIfCollisionWithQuadSub0(sResCameraProperties* r6, sTransformedVertice& r8_input, sTransformedVertice& r9_input)
{
    s32 r2 = r9_input.m10_clippedX - r8_input.m10_clippedX;
    s32 r14 = r9_input.m14_clippedY - r8_input.m14_clippedY;

    s32 r8 = r6->m1C_LCSHeightMin - r8_input.m14_clippedY;
    s32 r9 = r6->m1E_LCSHeightMax - r8_input.m14_clippedY;

    r8 *= r2;
    s32 r3 = (r6->m20_LCSWidthMin - r8_input.m10_clippedX) * r14;

    if (r8 > r3)
    {
        return false;
    }

    r9 *= r2;
    if (r9 > r3)
    {
        return false;
    }

    r3 = (r6->m22_LCSWidthMax - r8_input.m10_clippedX) * r14;

    if (r8 > r3)
    {
        return false;
    }
    if (r9 > r3)
    {
        return false;
    }

    return true;
}

bool setTBitIfCollisionWithQuad(sResCameraProperties* r6, sTransformedVertice& r10, sTransformedVertice& r11, sTransformedVertice& r12, sTransformedVertice& r13)
{
    // if all points have at least one common clip flag, the quad is clipped
    if (r10.m18_clipFlags & r11.m18_clipFlags & r12.m18_clipFlags & r13.m18_clipFlags)
        return true;

    // if at least a single vertices is clipped by 0x20 (far clip?), the quad is clipped
    if ((r10.m18_clipFlags | r11.m18_clipFlags | r12.m18_clipFlags | r13.m18_clipFlags) & 0x20)
        return true;

    r10.m1C_clipFlags2 = r10.m18_clipFlags;
    r11.m1C_clipFlags2 = r11.m18_clipFlags;
    r12.m1C_clipFlags2 = r12.m18_clipFlags;
    r13.m1C_clipFlags2 = r13.m18_clipFlags;

    r10.m10_clippedX = r10.m0_X;
    r10.m14_clippedY = r10.m2_Y;

    r11.m10_clippedX = r11.m0_X;
    r11.m14_clippedY = r11.m2_Y;

    r12.m10_clippedX = r12.m0_X;
    r12.m14_clippedY = r12.m2_Y;

    r13.m10_clippedX = r13.m0_X;
    r13.m14_clippedY = r13.m2_Y;

    if (!testQuadsForCollisionSub1(0x10, r6, testQuadsForCollisionSub2, r10, r11, r12, r13))
    {
        s32 r0 = (r13.m14_clippedY - r11.m14_clippedY) * (r12.m10_clippedX - r10.m10_clippedX);
        s32 r2 = (r12.m14_clippedY - r10.m14_clippedY) * (r13.m10_clippedX - r11.m10_clippedX);

        if (r2 > r0)
        {
            return true;
        }

        if (setTBitIfCollisionWithQuadSub0(r6, r10, r11))
            return true;

        if (setTBitIfCollisionWithQuadSub0(r6, r12, r13))
            return true;

        if (setTBitIfCollisionWithQuadSub0(r6, r11, r12))
            return true;

        if (setTBitIfCollisionWithQuadSub0(r6, r13, r10))
            return true;

        return false;
    }
    return true;
}

void testQuadsForCollisionSub0(const sProcessed3dModel::sQuad& r4, s32& r5_result, sResCameraProperties* r6, sTransformedVertice& r10, sTransformedVertice& r11, sTransformedVertice& r12, sTransformedVertice& r13)
{
    s32 r1 = r10.m4_fullPrecisionZ >> 16;
    s32 r2 = r11.m4_fullPrecisionZ >> 16;
    s32 r3 = r12.m4_fullPrecisionZ >> 16;
    s32 r8 = r13.m4_fullPrecisionZ >> 16;

    u8 r0 = r4.m8_lightingControl & 0xF0;
    switch (r0)
    {
    case 0:
        //601E634
        // basically max of r1/r2/r3/r8
        if (r1 < r2)
        {
            r1 = r2;
        }
        if (r3 < r8)
        {
            r3 = r8;
        }
        if (r1 <= r3)
        {
            r1 = r3;
        }
        break;
    case 0x10:
        //601E648
        // basically min of r1/r2/r3/r8
        if (r1 >= r2)
        {
            r1 = r2;
        }
        if (r3 >= r8)
        {
            r3 = r8;
        }
        if (r1 >= r3)
        {
            r1 = r3;
        }
        break;
    default:
        r1 += r2;
        r1 += r3;
        r1 += r8;
        r1 >>= 2;
        break;
    }

    //601E65A
    r1 *= 0x100;
    if (r1 <= r6->m28_LCSDepthMax)
    {
        r6->m28_LCSDepthMax = r1;
        r5_result = r4.m12_onCollisionScriptIndex;
    }
}

void testQuadsForCollision(const std::vector<sProcessed3dModel::sQuad>& r4_vector, s32& r5_result, sResCameraProperties* r6, std::array<sTransformedVertice, 256> & r7_transformeVertices)
{
    std::vector<sProcessed3dModel::sQuad>::const_iterator r4 = r4_vector.begin();
    do
    {
        if ((r4->m8_lightingControl & 0xF0) == 0)
        {
            sTransformedVertice& r10 = r7_transformeVertices[r4->m0_indices[0]];
            sTransformedVertice& r11 = r7_transformeVertices[r4->m0_indices[1]];
            sTransformedVertice& r12 = r7_transformeVertices[r4->m0_indices[2]];
            sTransformedVertice& r13 = r7_transformeVertices[r4->m0_indices[3]];

            if (!setTBitIfCollisionWithQuad(r6, r10, r11, r12, r13))
            {
                testQuadsForCollisionSub0(*r4, r5_result, r6, r10, r11, r12, r13);
            }
        }

        r4++;
    } while (r4 != r4_vector.end());

}

s32 testMeshForCollision(const sProcessed3dModel& r4, const sMatrix4x3& r5, sResCameraProperties* r6)
{
    std::array<sTransformedVertice, 256> transformedVertices;
    transformVerticesClipped(r4, r5, r6, transformedVertices);
    // note: normally, r4 would have been increased by 0xC by the previous function (skip over to quad info)

    s32 r5_result = -1;

    testQuadsForCollision(r4.mC_Quads, r5_result, r6, transformedVertices);

    return r5_result;
}

s32 testMeshVisibility(sResCameraProperties* r14, const sProcessed3dModel& r11)
{
    // check depth
    if (pCurrentMatrix->m[2][3] < r14->m24_LCSDepthMin - r11.m0_radius)
        return -1;

    if (pCurrentMatrix->m[2][3] > r14->m28_LCSDepthMax + r11.m0_radius)
        return -1;

    // check X
    if (pCurrentMatrix->m[0][3] < MTH_Mul(pCurrentMatrix->m[2][3], r14->m34_boundMinX) - MTH_Mul(r11.m0_radius, r14->m38_radiusScaleMinX))
        return -1;

    if (pCurrentMatrix->m[0][3] > MTH_Mul(pCurrentMatrix->m[2][3], r14->m3C_boundMaxX) + MTH_Mul(r11.m0_radius, r14->m40_radiusScaleMaxX))
        return -1;

    //0601412C
    //check Y
    if (pCurrentMatrix->m[1][3] < MTH_Mul(pCurrentMatrix->m[2][3], r14->m44_boundMinY) - MTH_Mul(r11.m0_radius, r14->m48_radiusScaleMinY))
        return -1;

    if (pCurrentMatrix->m[1][3] > MTH_Mul(pCurrentMatrix->m[2][3], r14->m4C_boundMaxY) + MTH_Mul(r11.m0_radius, r14->m50_radiusScaleMaxY))
        return -1;

    return testMeshForCollision(r11, *pCurrentMatrix, r14); //we should be passing r14->m14, but that seems to be a handrolled hack to alias r14->m14 to s_graphicEngineStatus_405C
}

void findLCSCollisionInCell(sResCameraProperties* r14, sTownCellTask* r12)
{
    if (r12 == nullptr)
        return;

    if (r12->m8_cellPtr.isNull())
        return;

    if (readSaturnS32(r12->m8_cellPtr + 0x14) == 0)
        return;

    pushCurrentMatrix();
    translateCurrentMatrix(r12->mC_position);
    if (
        (pCurrentMatrix->m[2][3] >= r14->m24_LCSDepthMin - gTownGrid.m2C) && (pCurrentMatrix->m[2][3] <= r14->m2C_projectionWidthScale + gTownGrid.m2C) &&
        (pCurrentMatrix->m[0][3] >= MTH_Mul(pCurrentMatrix->m[2][3], r14->m34_boundMinX) - r14->m54) && (pCurrentMatrix->m[0][3] <= MTH_Mul(pCurrentMatrix->m[2][3], r14->m3C_boundMaxX) + r14->m58)
        )
    {
        sSaturnPtr r13 = readSaturnEA(r12->m8_cellPtr + 0x14);
        while (readSaturnS32(r13))
        {
            pushCurrentMatrix();
            translateCurrentMatrix(readSaturnVec3(r13 + 4));
            sProcessed3dModel* model = r12->m0_fileBundle->getCollisionModel(readSaturnU32(r13));
            s32 r0 = testMeshVisibility(r14, *model);
            if (r0 != -1)
            {
                if (r0)
                {
                    r14->m10_activeLCSType = 1;
                    r14->m14_activeLCSEnvironmentIndex = r0;
                }
                else
                {
                    r14->m10_activeLCSType = 0;
                }
            }
            popMatrix();
            r13 += 0x10;
        }
    }
    //60140B8
    popMatrix();
}

const std::vector<sVec3_S16_12_4> townLCSVisibilityTestMeshIniVertices = {
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0}
};

std::vector<sProcessed3dModel::sQuad> townLCSVisibilityTestMeshIniQuad = {
    {
        {0,1,2,3},
        0x2900,
        0x5,
        0xE8,
        0,
        0,
        0,
        {
            {{0,0x1000,0}, {0,0,0}}
        }
    },
};

sProcessed3dModel townLCSVisibilityTestMesh(fixedPoint(0), townLCSVisibilityTestMeshIniVertices, townLCSVisibilityTestMeshIniQuad);

void findLCSCollision()
{
    scriptUpdateSub2Sub0(&LCSCollisionData, resData.m0);

    for (int i = 0; i < townGridSearchPattern.size(); i++)
    {
        findLCSCollisionInCell(&LCSCollisionData, gTownGrid.m40_cellTasks[(gTownGrid.mC + townGridSearchPattern[i][1]) & 7][(gTownGrid.m8 + townGridSearchPattern[i][0]) & 7]);
    }

    // find LCS collision with NPC objects
    sResData* var10 = &resData;
    sProcessed3dModel& var4 = townLCSVisibilityTestMesh;

    sVec3_S16_12_4& r10 = townLCSVisibilityTestMesh.m8_vertices[0];
    sVec3_S16_12_4& r8 = townLCSVisibilityTestMesh.m8_vertices[1];
    sVec3_S16_12_4& var0 = townLCSVisibilityTestMesh.m8_vertices[2];
    sVec3_S16_12_4& r9 = townLCSVisibilityTestMesh.m8_vertices[3];

    for (int i = 0; i < 5; i++)
    {
        sResData1C* r12 = var10->m8_headOfLinkedList[i];
        while (r12)
        {
            sMainLogic_74* r14 = r12->m4;
            r12 = r12->m0_pNext;
            if (r14->m0_collisionSetup.m1)
            {
                var4.m0_radius = r14->m4_collisionRadius;

                s16 r4 = r14->m14_collisionClip[0] >> 4;
                r10[0] = r4;
                r9[0] = r4;

                var0[0] = -r4;
                r8[0] = -r4;

                r4 = r14->m14_collisionClip[1] >> 4;
                r8[1] = r4;
                r10[1] = r4;

                r9[1] = -r4;
                var0[1] = -r4;

                var0[2] = 0;
                r9[2] = 0;
                r8[2] = 0;
                r10[2] = 0;

                pushCurrentMatrix();
                translateCurrentMatrix(r14->m8_position);
                sMatrix4x3& r5 = cameraProperties2.m88_billboardViewMatrix;

                pCurrentMatrix->m[0][0] = r5.m[0][0];
                pCurrentMatrix->m[0][1] = r5.m[0][1];
                pCurrentMatrix->m[0][2] = r5.m[0][2];
                pCurrentMatrix->m[1][0] = r5.m[1][0];
                pCurrentMatrix->m[1][1] = r5.m[1][1];
                pCurrentMatrix->m[1][2] = r5.m[1][2];
                pCurrentMatrix->m[2][0] = r5.m[2][0];
                pCurrentMatrix->m[2][1] = r5.m[2][1];
                pCurrentMatrix->m[2][2] = r5.m[2][2];

                /**/
                if (0)
                {
                    pCurrentMatrix->m[0][0] = 0xFFFF0000;
                    pCurrentMatrix->m[0][1] = 0x00000000;
                    pCurrentMatrix->m[0][2] = 0x00000000;
                    pCurrentMatrix->m[0][3] = 0xFFFFF8CD;
                    pCurrentMatrix->m[1][0] = 0x00000000;
                    pCurrentMatrix->m[1][1] = 0x0000FF16;
                    pCurrentMatrix->m[1][2] = 0xFFFFEA6E;
                    pCurrentMatrix->m[1][3] = 0xFFFFE58C;
                    pCurrentMatrix->m[2][0] = 0x00000000;
                    pCurrentMatrix->m[2][1] = 0x00001592;
                    pCurrentMatrix->m[2][2] = 0x0000FF16;
                    pCurrentMatrix->m[2][3] = 0x00007F27;

                    LCSCollisionData.m34_boundMinX = 0xFFFFF295;
                    LCSCollisionData.m3C_boundMaxX = 0xFFFFF70E;

                    LCSCollisionData.m44_boundMinY = 0xFFFFC8DC;
                    LCSCollisionData.m4C_boundMaxY = 0xFFFFCE1C;

                    LCSCollisionData.m1C_LCSHeightMin = 0x2A;
                    LCSCollisionData.m1E_LCSHeightMax = 0x26;
                    LCSCollisionData.m20_LCSWidthMin = 0xFFF4;
                    LCSCollisionData.m22_LCSWidthMax = 0xFFF8;
                }
                /**/

                if (testMeshVisibility(&LCSCollisionData, var4) == 0)
                {
                    if (!r14->m3C_scriptEA.isNull())
                    {
                        LCSCollisionData.m10_activeLCSType = 2;
                        LCSCollisionData.m14_activeLCS = r14;
                    }
                    else
                    {
                        LCSCollisionData.m10_activeLCSType = 0;
                    }
                }
                popMatrix();
            }
        }
    }
}
