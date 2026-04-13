#pragma once

struct FLD_A5_data : public sSaturnMemoryFile
{
    FLD_A5_data() : sSaturnMemoryFile("FLD_A5.PRG")
    {
    }
};

extern FLD_A5_data* gFLD_A5;

// A5 exit entity task struct (size 0x38)
struct sA5ExitEntity : public s_workAreaTemplate<sA5ExitEntity>
{
    s32 m0;
    s32 m4_angle;
    s32 m8;
    sVec3_FP mC_direction;
    s32 m18;
    s32 m1C_exitTargetX_mode1;
    s32 m20_exitTargetZ_mode1;
    s32 m24_exitTargetX_mode3;
    s32 m28_exitTargetZ_mode3;
    s32 m2C_exitMode;
    s32 m30_radius;
    u8 m34;
    u8 m35;
    u8 m36;
    // size 0x38
};

// A5 field-specific data task struct (size 0x68)
struct s_fieldSpecificData_A5 : public s_workAreaTemplate<s_fieldSpecificData_A5>
{
    void* m0_pEncounterConfig;
    sA5ExitEntity* m4_pExitEntity;
    void* m8_pWormData;
    void* mC_wormColorTable0;
    void* m10_wormColorTable1;
    s32 m14_wormColorState0;
    s32 m18_wormColorState1;
    u8 m1C_pad[0x38];
    void* m54;
    u8 m58_pad[4];
    fixedPoint m5C_scale;
    u8 m60_pad[8];
    // size 0x68
};

p_workArea overlayStart_FLD_A5(p_workArea workArea, u32 arg);

void subfieldA5_0(p_workArea workArea);
void subfieldA5_1(p_workArea workArea);
void subfieldA5_2(p_workArea workArea);
void subfieldA5_3(p_workArea workArea);
void subfieldA5_4(p_workArea workArea);
void subfieldA5_5(p_workArea workArea);
void subfieldA5_6(p_workArea workArea);
void subfieldA5_7(p_workArea workArea);
void subfieldA5_8(p_workArea workArea);
void subfieldA5_9(p_workArea workArea);
void subfieldA5_A(p_workArea workArea);
void subfieldA5_B(p_workArea workArea);
void createA5Vdp2Task(p_workArea parent);
void createA5NightVdp2Task(p_workArea parent);
void createA5_B_Vdp2Task(p_workArea parent);

// Shared startTasks helpers
void createFieldSpecificDataTask_A5(p_workArea parent);
void createA5_3dSceneManager(p_workArea parent);
void createA5_wormObjectTask(p_workArea parent);
void createA5_exitEntityTask(p_workArea parent);
void setFieldExitConfig_A5(s32 mode, s32* pParams);
void createA5_corridorWorm_1(p_workArea parent);
void createA5_corridorWorm_3(p_workArea parent);
void createA5_corridorWorm_5(p_workArea parent);
void createA5_corridorWorm_6(p_workArea parent);
void createA5_corridorWorm_A(p_workArea parent);
void createA5_envObject_5();
void createA5_envObject_6();
void createA5_envObject_A();
void createA5_wormDustTask(p_workArea parent);
void createA5_wormSegment(p_workArea parent, sSaturnPtr data);
void createA5_wormSegments_day(p_workArea parent);
void createA5_wormSegments_night(p_workArea parent);
void createA5_proximityAlert_day(p_workArea parent);
void createA5_proximityAlert_night(p_workArea parent);
void createA5_fieldEventCheck(p_workArea parent);
void createA5_baldorWormBody(p_workArea parent);
void createA5_decorObjects_day();
void createA5_encounterConfig(p_workArea parent, s32 subfieldId, s16 param1, s16 colorMask, s16 param3, s16 param4, s16 param5, s16 param6);
void createA5_triggerZones_day(p_workArea parent);
void createA5_triggerZones_night(p_workArea parent);

// Scenery entity (subfield 9)
p_workArea createA5_sceneryEntity(p_workArea parent);

// Exit trigger entity
struct sA5ExitTriggerArg;
struct sA5ExitTriggerEntity;
sA5ExitTriggerEntity* a5_createExitTriggerEntity_06056870(p_workArea parent, const sA5ExitTriggerArg* arg);
void createA5_envObjects_sub7_exits(p_workArea parent);

// Per-subfield environment objects
void createA5_envObjects_sub0(p_workArea parent);
void createA5_envObjects_sub4_particle(p_workArea parent);
void createA5_envObjects_sub4_light(p_workArea parent);
void createA5_envObjects_sub4_wormSegments(p_workArea parent);
void createA5_envObjects_sub2_sandfall(p_workArea parent);
void createA5_envObjects_sub2_skyEntity(p_workArea parent, sSaturnPtr arg);
void createA5_envObjects_sub2_weatherTask(p_workArea parent);
void createA5_corridorWorm_2(p_workArea parent);
void createA5_corridorWorm_4(p_workArea parent);
void createA5_envObjects_sub0_exits(p_workArea parent);
void createA5_decorObjects_night_2();
void createA5_decorObjects_night_4();
void initA5_wormColorTables_day();
void initA5_wormColorTables_night();

// Post-battle sound callbacks
void postBattleSound_A5_day();
void postBattleSound_A5_nightA();
void postBattleSound_A5_nightB();

// Shared gameplay helpers (getDragonPosition, getDragonAngle, triggerSubfieldChange) are in field.h

// Follow mode setup
void a5_wormSegmentEntity_startFollowMode_060694D8(s32 followMode);
void a5_activateFollowMode_06069508(s32 followMode);
sFieldCameraStatus* a5_wormSegmentEntity_getActiveCameraSlot_06069490();

// Shared helpers
void setupCameraConfig_A5_0();
void setupCameraConfig_A5_2();
void setupCameraConfig_A5_4();
void setupCameraConfig_A5_7();
void setupCameraConfig_A5_8();
void setupCameraConfig_A5_9();
void setupCameraConfig_A5_corridor();
void initDragonParams_A5_open();
void initDragonParams_A5_corridor();
void initDragonParams_A5_B();

// Dragon cutscene helpers (used by scenery entity effect)
struct s_scriptData3;
// a5_dragonTransitionToNormal — moved to shared dragonTransitionToNormal() in fieldDragonInput.h
// a5_setCutsceneKeyFrame, a5_clearCutsceneKeyFrame, a5_enableScriptSkippingAndExit
// — moved to shared fieldDragonInput.h as setCutsceneKeyFrame, clearCutsceneKeyFrame, enableScriptSkippingAndExit
