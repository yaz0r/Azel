#pragma once

extern sSaturnMemoryFile* gFLD_A3;

p_workArea overlayStart_FLD_A3(p_workArea workArea, u32 arg);
s32 startFieldScript(s32 r4, s32 r5);

void initDragonSpeed(u32 arg);
void dragonFieldTaskInitSub4(s_dragonTaskWorkArea* pTypedWorkArea);
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
void setupDragonPosition(sVec3_FP* r4, sVec3_FP* r5);

p_workArea create_fieldA3_0_task0(p_workArea workArea); // shared subfield 0 and 1
void subfieldA3_1Sub0Sub0();
void subfieldA3_1Sub0Sub2(s32 r4, s32 r5);
s_cutsceneData* loadCutsceneData(sSaturnPtr EA);
void startCutscene(s_cutsceneData* r4);
void createFieldPaletteTask(p_workArea parent);
void adjustVerticalLimits(fixedPoint r4, fixedPoint r5);
void subfieldA3_1_Sub0();
void nullBattle();
void getFieldDragonPosition(sVec3_FP* r4);
u32 gridCellDraw_GetDepthRange(fixedPoint r4);
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
    s16 m1C_receivedItemId;
    s8 m1E_receivedItemQuantity;
    s8 m1F;
};

s_LCSTask340Sub* LCSTaskDrawSub1Sub2Sub0Sub2Sub0(s_LCSTask* r4, sLaserArgs* r5, s8 r6);

struct s_itemBoxDefinition
{
    sVec3_FP m0_pos;
    sVec3_FP mC;
    sVec3_FP m18;
    sVec3_FP m24;
    fixedPoint m30_scale;
    fixedPoint m34;
    s32 m38;
    s32 m3C_receivedItemId;
    s8 m40_receivedItemQuantity;
    s8 m41_LCSType; // 2 : savepoint
    s8 m42;
    s8 m43;
    s8 m44;
};
s_itemBoxDefinition* readItemBoxDefinition(sSaturnPtr ptr);
p_workArea fieldA3_1_createItemBoxes_Sub1(s_itemBoxDefinition* r4);

s_cameraScript* readCameraScript(sSaturnPtr EA);
void dispatchTutorialMultiChoiceSub2();

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
