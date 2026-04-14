#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldRadar.h"
#include "audio/soundDriver.h"

// B entity 0 (size 0x18, boss arena trigger/controller)
struct sA5BossEntity0 : public s_workAreaTemplate<sA5BossEntity0>
{
    s_memoryAreaOutput m0_memoryArea;
    s32 m8_scrollX;
    s32 mC_horizonY;
    s32 m10_baseOffset;
    s16 m14_vdp1Memory;
    u8 m16_pad[2];
    // size 0x18
};

// 060556D2
static void a5BossEntity0_Init(sA5BossEntity0* pThis)
{
    getMemoryArea(&pThis->m0_memoryArea, 8);
    pThis->m14_vdp1Memory = (s16)((pThis->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    pThis->m10_baseOffset = -4;
}

// 060556F6
static void a5BossEntity0_Update(sA5BossEntity0* pThis)
{
    s32 camY = cameraProperties2.m0_position.m4_Y.asS32();
    s32 heightFactor = (camY < 1) ? 0 : (camY >> 2);

    s32 pitchAngle = atan2(heightFactor, 0x80000);
    u16 angleIdx = ((u16)cameraProperties2.mC_rotation[0] + (s16)pitchAngle) & 0xFFF;

    s32 tanVal = FP_Div(getSin(angleIdx), getCos(angleIdx)).asS32();
    s32 horizonOffset = MTH_Mul(graphicEngineStatus.m405C.m1C_heightScale, fixedPoint(tanVal)).asS32();
    pThis->mC_horizonY = pThis->m10_baseOffset - horizonOffset;

    s32 yawVal = MTH_Mul(fixedPoint((s32)(s16)cameraProperties2.mC_rotation[1]), fixedPoint(0x3243F)).asS32();
    s32 scrollX = MTH_Mul(graphicEngineStatus.m405C.m18_widthScale, fixedPoint(yawVal << 5)).asS32();
    pThis->m8_scrollX = performModulo(0x50, scrollX) - 0x50;
}

// 06055822 — draw boss background: tiles VDP1 normal sprites across the screen.
// Uses raw VDP1 command submission (0x06055678) which needs bgfx adaptation.
static void a5BossEntity0_Draw(sA5BossEntity0* pThis)
{
    Unimplemented(); // VDP1 normal sprite tiling — needs bgfx rendering path
}

// 060558d8 — create subfield B entity (boss controller)
static void createA5_B_entity0(p_workArea parent)
{
    static sA5BossEntity0::TypedTaskDefinition td = { &a5BossEntity0_Init, &a5BossEntity0_Update, &a5BossEntity0_Draw, nullptr };
    createSubTask<sA5BossEntity0>(parent, &td);
}

// 06057154 — create worm segment entity for B
// 06057154
static void createA5_B_wormSegment(p_workArea parent)
{
    createA5_wormSegment(parent, gFLD_A5->getSaturnPtr(0x06098D90));
}

// B entity 1 (size 0x2C, boss arena entity)
struct sA5BossEntity1 : public s_workAreaTemplate<sA5BossEntity1>
{
    u8 m0_data[0x2C];
    // size 0x2C
};

// 0605C13A
static void a5BossEntity1_Init(sA5BossEntity1* pThis) { Unimplemented(); }
// 0605C364
static void a5BossEntity1_Update(sA5BossEntity1* pThis) { Unimplemented(); }
// 0605C7C0
static void a5BossEntity1_Draw(sA5BossEntity1* pThis) { Unimplemented(); }

// 0605c7e0 — create subfield B entity
static void createA5_B_entity1(p_workArea parent)
{
    static sA5BossEntity1::TypedTaskDefinition td = { &a5BossEntity1_Init, &a5BossEntity1_Update, &a5BossEntity1_Draw, nullptr };
    createSubTask<sA5BossEntity1>(parent, &td);
}

// 06065904 — set field data scale
static void a5_B_setFieldScale()
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    pFieldData->m5C_scale = fixedPoint(0x10000);
}

// 0605ab0e
static void fieldA5_B_startTasks(p_workArea workArea)
{
    createFieldSpecificDataTask_A5(workArea);
    createA5_B_entity0(workArea);
    createA5_B_wormSegment(workArea);
    createA5_B_entity1(workArea);
    a5_B_setFieldScale();
}

// 0605434c — init dragon light params for subfield B
static void initDragonLight_A5_B()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragon->mC8_normalLightColor = { 0x10, 0x10, 0x10 };
    pDragon->mCB_falloffColor0 = { 9, 9, 9 };
    pDragon->mCE_falloffColor1 = { (s8)0xFB, (s8)0xFB, (s8)0xFB };
    pDragon->mD1_falloffColor2 = { 0x19, 0x19, 0x19 };
    pDragon->mC0_lightRotationAroundDragon = fixedPoint(0xC000000);
    pDragon->mC4 = 0;
}

// 06054D44
void subfieldA5_B(p_workArea workArea)
{
    Unimplemented(); // FUN_FLD_A5__06078dc0 — setupField variant with data tables and visibility

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;
    Unimplemented(); // FUN_FLD_A5__060558f0 — camera config (calls 06069620 with data 06089c8c, count 2)
    initDragonLight_A5_B();
    createA5_B_Vdp2Task(workArea);
    startFieldScript(0x13, -1);
    startCutscene(loadCutsceneData(gFLD_A5->getSaturnPtr(0x0608D1B0)));
    adjustVerticalLimits((s32)0xFFFE2000, 0x1E000);
    fieldRadar_enableAltitudeGauge();

    graphicEngineStatus.m405C.m14_farClipDistance = 0x200000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);
    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    // 06072d80
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m64 = 1;
}
