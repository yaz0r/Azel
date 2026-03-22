#include "PDS.h"
#include "fieldItemBox.h"
#include "fieldItemBox_type2.h"
#include "fieldVisibilityGrid.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "field_a3/o_fld_a3.h"

// Save point particle task
// Each instance manages 12 particles orbiting around the save point
// 3 instances are created per save point (total 36 particles)
struct sSavePointParticleTask : public s_workAreaTemplate<sSavePointParticleTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, &Update, &Draw, nullptr };
        return &taskDefinition;
    }

    // 0607C3B8
    static void Update(sSavePointParticleTask* pThis)
    {
        s_itemBoxType1* pParent = pThis->m8_parent;
        if (pParent->mEA_wasRendered == 0)
            return;

        // Advance animation frames for all active particles
        s32 numParticles = 12;
        for (s32 i = 0; i < numParticles; i++)
        {
            u8 frame = pThis->m1C_quads[i].m7_currentFrame + 1;
            if ((s32)frame >= numParticles)
                frame = 0;
            pThis->m1C_quads[i].m7_currentFrame = frame;
        }

        // Compute current position using orbital motion
        s32 posIndex = performModulo(12, pThis->m118_frameCounter);
        u32 byteIndex = posIndex * 12;

        // X = parentPos.X + radius * getSin(angleY) * getCos(angleX)
        fixedPoint radius = pThis->mC_radius;
        s32 angleY = pThis->m10_angleY.getInteger();
        s32 angleX = pThis->m14_angleX.getInteger();

        fixedPoint x = pParent->m3C_pos.m0_X + MTH_Mul_5_6(radius, getSin(angleY & 0xFFF), getCos(angleX & 0xFFF));
        pThis->m7C_positions[posIndex].m0_X = x;
        pThis->m10C_lastPosition.m0_X = x;

        // Y = parentPos.Y + radius * getCos(angleY)
        fixedPoint y = pParent->m3C_pos.m4_Y + MTH_Mul(radius, getCos(angleY & 0xFFF));
        pThis->m7C_positions[posIndex].m4_Y = y;
        pThis->m10C_lastPosition.m4_Y = y;

        // Z = parentPos.Z + radius * getSin(angleY) * getSin(angleX)
        fixedPoint z = pParent->m3C_pos.m8_Z + MTH_Mul_5_6(radius, getSin(angleY & 0xFFF), getSin(angleX & 0xFFF));
        pThis->m7C_positions[posIndex].m8_Z = z;
        pThis->m10C_lastPosition.m8_Z = z;

        // Advance rotation angle
        pThis->m10_angleY = pThis->m10_angleY + pThis->m18_rotationSpeed;
        pThis->m118_frameCounter++;
    }

    // 0607C53C
    static void Draw(sSavePointParticleTask* pThis)
    {
        s_itemBoxType1* pParent = pThis->m8_parent;
        if (pParent->mEA_wasRendered == 0)
            return;

        s32 count = pThis->m118_frameCounter;
        if (count > 12)
            count = 12;

        for (s32 i = 0; i < count; i++)
        {
            drawProjectedParticleWithGouraud(&pThis->m1C_quads[i], &pThis->m7C_positions[i], pThis->m11C_gouraudColors);
        }
    }

    s_memoryAreaOutput m0_memArea;   // +0x00, filled by getMemoryArea
    s_itemBoxType1* m8_parent;      // +0x08
    fixedPoint mC_radius;           // +0x0C
    fixedPoint m10_angleY;          // +0x10
    fixedPoint m14_angleX;          // +0x14
    fixedPoint m18_rotationSpeed;   // +0x18
    sAnimatedQuad m1C_quads[12];    // +0x1C, 12 * 8 = 0x60
    sVec3_FP m7C_positions[12];     // +0x7C, 12 * 12 = 0x90
    sVec3_FP m10C_lastPosition;     // +0x10C
    s32 m118_frameCounter;          // +0x118
    u16* m11C_gouraudColors;        // gouraud tint colors (4 x RGB555)
    // size 0x120
};

// 0607C5A8
static p_workArea createSavePointParticleInstance(s_itemBoxType1* parent, fixedPoint initialAngleY, fixedPoint initialAngleX)
{
    sSavePointParticleTask* pTask = createSubTask<sSavePointParticleTask>(parent);
    if (!pTask)
        return nullptr;

    if (!gFLD_A3)
    {
        gFLD_A3 = new FLD_A3_data();
    }

    getMemoryArea(&pTask->m0_memArea, 0);

    u16 vdp1Memory = 0;
    if (pTask->m0_memArea.m4_characterArea)
    {
        vdp1Memory = (u16)((pTask->m0_memArea.m4_characterArea - 0x25C00000) >> 3);
    }

    pTask->m8_parent = parent;
    pTask->m10_angleY = initialAngleY;
    pTask->m14_angleX = initialAngleX;
    pTask->m18_rotationSpeed = 0x666666;
    pTask->m118_frameCounter = 0;

    static u16 gouraudColors[4] = { 0 };
    if (gouraudColors[0] == 0)
    {
        sSaturnPtr gouraudPtr = gFLD_A3->m_savePointGouraudColors;
        for (int i = 0; i < 4; i++)
        {
            gouraudColors[i] = readSaturnU16(gouraudPtr + i * 2);
        }
    }
    pTask->m11C_gouraudColors = gouraudColors;

    u8 frameCounter = 11;
    for (s32 i = 0; i < 12; i += 3)
    {
        particleInitSub(&pTask->m1C_quads[i], vdp1Memory, &gFLD_A3->m_savePointParticleQuad);
        pTask->m1C_quads[i].m7_currentFrame = frameCounter;
        pTask->mC_radius = MTH_Mul(0x20000, parent->m78_scale);

        particleInitSub(&pTask->m1C_quads[i + 1], vdp1Memory, &gFLD_A3->m_savePointParticleQuad);
        pTask->m1C_quads[i + 1].m7_currentFrame = frameCounter - 1;
        pTask->mC_radius = MTH_Mul(0x20000, parent->m78_scale);

        particleInitSub(&pTask->m1C_quads[i + 2], vdp1Memory, &gFLD_A3->m_savePointParticleQuad);
        pTask->m1C_quads[i + 2].m7_currentFrame = frameCounter - 2;
        pTask->mC_radius = MTH_Mul(0x20000, parent->m78_scale);

        frameCounter -= 3;
    }

    return pTask;
}

// 0607C6DC
p_workArea createSavePointParticles(s_itemBoxType1* parent)
{
    p_workArea first = createSavePointParticleInstance(parent, 0x02AAAAAA, 0x00000000);
    createSavePointParticleInstance(parent, 0x08000000, 0x05555555);
    createSavePointParticleInstance(parent, 0x0D555555, 0x0AAAAAAA);
    return first;
}

void LCSItemBox_UpdateType2(s_itemBoxType1* pThis)
{
    if (LCSItemBox_shouldSpin(pThis))
    {
        pThis->m20 = 0;
    }
    else
    {
        pThis->m20 |= 2;
    }

    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m3C_pos);
        rotateCurrentMatrixZYX(&pThis->m6C_rotation);
        scaleCurrentMatrixRow0(pThis->m78_scale);
        scaleCurrentMatrixRow1(pThis->m78_scale);
        scaleCurrentMatrixRow2(pThis->m78_scale);

        transformAndAddVecByCurrentMatrix(&LCSItemBox_Table6[pThis->m8B_LCSType], &pThis->m60);
        LCSItemBox_UpdateType0Sub0(pThis, 0x54, 0x198, 0x7C);
        popMatrix();
    }

    if (pThis->m8D)
    {
        pThis->m20 |= 1;
    }

    updateLCSTarget(&pThis->m8_LCSTarget);
}

void LCSItemBox_DrawType2(s_itemBoxType1* pThis)
{
    s_visibilityGridWorkArea* pGridTask = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m3C_pos);
    rotateCurrentMatrixZYX(&pThis->m6C_rotation);
    scaleCurrentMatrixRow0(pThis->m78_scale);
    scaleCurrentMatrixRow1(pThis->m78_scale);
    scaleCurrentMatrixRow2(pThis->m78_scale);

    u32 depthRangeIndex = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);

    if (depthRangeIndex <= pGridTask->m1300)
    {
        LCSItemBox_DrawType0Sub0(pThis->m0.m0_mainMemoryBundle, 8, 0x14C);
        pThis->mEA_wasRendered = 1;
    }
    else
    {
        pThis->mEA_wasRendered = 0;
    }

    popMatrix();
}
