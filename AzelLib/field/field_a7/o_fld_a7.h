#pragma once

struct FLD_A7_data : public sFieldOverlay
{
    FLD_A7_data() : sFieldOverlay("FLD_A7.PRG")
    {
    }

    void dispatchCellObjectCreation(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6) override;
    s32 executeNative(sSaturnPtr ptr) override;
    s32 executeNative(sSaturnPtr ptr, s32 arg0) override;
};

extern FLD_A7_data* gFLD_A7;

#include "field/fieldModelRender.h"

struct s_fieldSpecificData_A7 : public s_workAreaTemplate<s_fieldSpecificData_A7>
{
    u8 m0_pad[0x274];
    s16 m274;
    u8 m276;
    u8 m277_segmentsReturned;
    u8 m278;
    u8 m279;
    u8 m27A;
    u8 m27B;
    u8 m27C_pad[4];
    void* m280;
    // size 0x284
};

inline s_fieldSpecificData_A7* getFieldSpecificData_A7() { return (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC; }

p_workArea overlayStart_FLD_A7(p_workArea workArea, u32 arg);

void subfieldA7_0(p_workArea workArea);
void subfieldA7_1(p_workArea workArea);
void subfieldA7_2(p_workArea workArea);

// Shared helpers called from subfield files
void createFieldSpecificDataTask_A7(p_workArea parent);
void initDragonParams_A7_0();
void initDragonParams_A7_1();
void initDragonParams_A7_2();
void setGameFlagsA7_0();
void countActiveObjectsA7();
void createA7_simpleSubtask(p_workArea parent);
s32 a7CenteredRandom(u32 mask);
void createA7_itemBoxes_0(p_workArea parent);
void createA7_fieldScriptSubtask(p_workArea parent);
void createA7_proximityCheck(p_workArea parent);
void createA7_encounterCheckSubtask(p_workArea parent);
void createA7_emptySubtask(p_workArea parent);
void createA7_soundCleanup_0x65(p_workArea parent);
void createA7_soundCleanup_0x6f(p_workArea parent);
void createA7_envEntity_a456(p_workArea parent);
void createA7_envEntity_9e6e(p_workArea parent, sSaturnPtr arg);
void createA7_itemBoxes_1();
void createA7_envEntity_a2fe(p_workArea parent);
void createA7_envEntity_51ee(p_workArea parent, sSaturnPtr arg, u8 param3);
void createA7_envEntity_e7c2(p_workArea parent);
void createA7_envEntity_e034(p_workArea parent);
void createA7_visibilityObjects();
void startTasksA7_0(p_workArea parent);
void startTasksA7_1(p_workArea parent);
void startTasksA7_2(p_workArea parent);

// VDP2 tasks
void createA7_0_envTask(p_workArea parent);
void createA7_1_envTask(p_workArea parent);
void createA7_2_envTask(p_workArea parent);

// Dragon callbacks (used from subfield files)
void dragonCallback_A7_1(s_dragonTaskWorkArea* pDragon);
void dragonCallback_A7_2(s_dragonTaskWorkArea* pDragon);
