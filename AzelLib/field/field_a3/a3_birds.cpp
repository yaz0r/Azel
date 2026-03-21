#include "PDS.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"

extern void itemBoxType1InitSub0(s_3dModel* r4, s32 r5);

// Bird init data from Saturn table at 0x06090cf4 (0x18 bytes per bird, 16 birds)
struct sBirdInitData
{
    s32 m0;  // → m20
    s32 m4;  // → m24
    s32 m8;  // → m28
    s32 mC;  // → m34
    s32 m10; // → m2C
    s32 m14; // → m30
};

struct sBirdTask : public s_workAreaTemplateWithArg<sBirdTask, sBirdInitData*>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { Init, Update, Draw, nullptr };
        return &taskDefinition;
    }

    // 0605B7E6
    static void Init(sBirdTask* pThis, sBirdInitData* pArg)
    {
        getMemoryArea(&pThis->m0_memoryArea, 5);

        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        init3DModelRawData(pThis, &pThis->m38_model, 0, pBundle, 4,
            pBundle->getAnimation(0x64),
            pBundle->getStaticPose(0x60, pBundle->getAnimation(0x64)->m2_numBones),
            nullptr, nullptr);

        stepAnimation(&pThis->m38_model);
        itemBoxType1InitSub0(&pThis->m38_model, randomNumber() & 7);

        pThis->m20_center[0] = pArg->m0;  // center X
        pThis->m20_center[1] = pArg->m4;  // center Y
        pThis->m20_center[2] = pArg->m8;  // center Z
        pThis->m34 = pArg->mC;            // orbit radius
        pThis->m2C = pArg->m10;           // initial angle
        pThis->m30 = pArg->m14;           // angular velocity

        pThis->m14_rotation[2] = 0;
        pThis->m14_rotation[1] = 0;
        pThis->m14_rotation[0] = 0;
    }

    // 0605B788
    static void Update(sBirdTask* pThis)
    {
        // Ghidra: iVar1 = m2C + m30; m2C = iVar1;
        s32 angle = pThis->m2C + pThis->m30;
        pThis->m2C = angle;

        // Ghidra: posX = MTH_Mul(m34, sinTable[angle_int]) + m20
        s32 angleInt = (s32)((u32)angle >> 16) & 0xFFF;
        pThis->m8_position[0] = MTH_Mul(pThis->m34, getSin(angleInt)) + pThis->m20_center[0];

        // Ghidra: posY = m24
        pThis->m8_position[1] = pThis->m20_center[1];

        // Ghidra: posZ = MTH_Mul(m34, cosTable[angle_int]) + m28
        pThis->m8_position[2] = MTH_Mul(pThis->m34, getCos(angleInt)) + pThis->m20_center[2];

        // Ghidra: *(dword*)(param_1 + 0x18) = m2C + 0x4000000
        pThis->m14_rotation[1] = pThis->m2C + 0x4000000;

        stepAnimation(&pThis->m38_model);
    }

    // 0605B75C
    static void Draw(sBirdTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_position);

        rotateCurrentMatrixYXZ(pThis->m14_rotation);

        pThis->m38_model.m18_drawFunction(&pThis->m38_model);
        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea; // 0x00
    sVec3_FP m8_position;            // 0x08
    sVec3_FP m14_rotation;           // 0x14 — rotation angles (big-endian: upper16 of each = Y, X, Z)
    sVec3_FP m20_center;             // 0x20 — orbit center position
    s32 m2C;                         // 0x2C — current angle
    s32 m30;                         // 0x30 — angular velocity
    s32 m34;                         // 0x34 — orbit radius
    s_3dModel m38_model;             // 0x38
    // size 0x88
};

// 0605b87a
void fieldA3_3_createBirdsTask(p_workArea workArea)
{
    sSaturnPtr birdData = gFLD_A3->getSaturnPtr(0x06090cf4);
    for (u32 i = 0; i < 16; i++)
    {
        sBirdInitData data;
        sSaturnPtr entry = birdData + i * 0x18;
        data.m0 = readSaturnS32(entry + 0x00);
        data.m4 = readSaturnS32(entry + 0x04);
        data.m8 = readSaturnS32(entry + 0x08);
        data.mC = readSaturnS32(entry + 0x0C);
        data.m10 = readSaturnS32(entry + 0x10);
        data.m14 = readSaturnS32(entry + 0x14);
        createSubTaskWithArg<sBirdTask>(workArea, &data);
    }
}
