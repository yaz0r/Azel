#pragma once

struct FLD_A3_data : public sSaturnMemoryFile
{
    void init();

    std::vector<s_scriptData1> m6083244;

    FLD_A3_data() : sSaturnMemoryFile("FLD_A3.PRG")
    {

    }
};

extern FLD_A3_data* gFLD_A3;

p_workArea overlayStart_FLD_A3(p_workArea workArea, u32 arg);
s32 startFieldScript(s32 r4, s32 r5);
s32 startFieldScriptSkippable(s32 r4, s32 r5);

sFieldCameraStatus* getFieldCameraStatus();
void dragonFieldTaskInitSub4Sub3(u8 r4);

fixedPoint interpolateRotation(fixedPoint r10_currentValue, fixedPoint r12_targetValue, fixedPoint stack0, fixedPoint r11, s32 r13);

void initDragonSpeed(u32 arg);
void dragonAutomaticMovementWhenEnteringNewField(s_dragonTaskWorkArea* pTypedWorkArea);
void fieldOverlaySubTaskInitSub1(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*));
u32 fieldOverlaySubTaskInitSub3(u32 r4);
p_workArea createLCSShootTask(s_LCSTask* r4, sLCSTarget* r5);
void startScript_cantDestroy();
void LCSUpdateCursorFromInputSub0(s32 r4, sVec3_FP* r5);
s8 LCSTaskDrawSub1Sub6();
void LCSTaskDrawSub1Sub3();
s32 fieldScriptTaskUpdateSub4();
void LCSTaskDrawSub1Sub3();
void LCSTaskDrawSub1Sub4();

s_DataTable3* readDataTable3(sSaturnPtr EA);
std::vector<std::vector<sCameraVisibility>>* readCameraVisbility(sSaturnPtr EA, s_DataTable3* pDataTable3);
s_DataTable2* readDataTable2(sSaturnPtr EA);
void setupField(s_DataTable3* r4, s_DataTable2* r5, void(*r6)(p_workArea workArea), std::vector<std::vector<sCameraVisibility>>* r7);
void setupField2(s_DataTable3* r4, void(*r5)(p_workArea workArea));
void setupDragonPosition(const sVec3_FP* r4, const sVec3_FP* r5);

p_workArea create_fieldA3_0_task0(p_workArea workArea); // shared subfield 0 and 1
void subfieldA3_1Sub0Sub0();
void subfieldA3_1Sub0Sub2(s32 r4, s32 r5);
s_cutsceneData* loadCutsceneData(sSaturnPtr EA);
void loadScriptData1(std::vector<s_scriptData1>& output, const sSaturnPtr& EA);
void startCutscene(s_cutsceneData* r4);
void createFieldPaletteTask(p_workArea parent);
void adjustVerticalLimits(fixedPoint r4, fixedPoint r5);
void nullBattle();
void getFieldDragonPosition(sVec3_FP* r4);
void fieldScriptTaskUpdateSub2Sub1Sub1Sub1Sub2(s_LCSTask340Sub* r4);
void setupField3(s_DataTable3* r4, void(*r5)(p_workArea workArea), std::vector<std::vector<sCameraVisibility>>* r6);
s32 checkPositionVisibilityAgainstFarPlane(sVec3_FP* r4);
void createSmokePufTask(p_workArea pThis, sVec3_FP* r5, s32 r6);

struct s_itemType0* fieldA3_1_createItemBoxes_Sub0(p_workArea workArea, struct s_itemBoxDefinition* r5);

struct sLaserArgs
{
    p_workArea m0;
    const sVec3_FP* m4;
    s32 m8;
    const sVec3_FP* mC;
    sVec3_FP* m10;
    sLCSTarget* m14;
    s_LCSTask340* m18;
    eItems m1C_receivedItemId;
    s8 m1E_receivedItemQuantity;
    s8 m1F;
};

s_LCSTask340Sub* LCSTaskDrawSub1Sub2Sub0Sub2Sub0(s_LCSTask* r4, sLaserArgs* r5, s8 r6);

s_cameraScript* readCameraScript(sSaturnPtr EA);
void dispatchTutorialMultiChoiceSub2();
s32 queueNewFieldScript(sSaturnPtr r4, s32 r5);
void fieldPaletteTaskInitSub0();

struct s_fieldA3_1_task4 : public s_workAreaTemplate<s_fieldA3_1_task4>
{
    fixedPoint m0;
    fixedPoint m4;
    fixedPoint m8;
    // size C
};

#include "o_fld_a3_0.h"
#include "o_fld_a3_1.h"
#include "o_fld_a3_2.h"
#include "o_fld_a3_3.h"
#include "o_fld_a3_4.h"
#include "o_fld_a3_5.h"
#include "o_fld_a3_8.h"
#include "o_fld_a3_A.h"
#include "o_fld_a3_C.h"
