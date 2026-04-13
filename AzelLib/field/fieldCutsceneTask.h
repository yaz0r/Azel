#pragma once

#include <vector>

struct s_scriptData1;
struct s_scriptData2;
struct s_scriptData3;
struct sVec3_FP;
struct sSaturnPtr;

struct s_cutsceneData
{
    std::vector<s_scriptData3> m0;
    std::vector<s_scriptData1> m4;
    std::vector<s_scriptData2> m4bis;
    u8 m8;
};

struct s_cutsceneTask : public s_workAreaTemplateWithArg<s_cutsceneTask, struct s_cutsceneData*>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &s_cutsceneTask::Init, &s_cutsceneTask::Update, NULL, NULL};
        return &taskDefinition;
    }
    static void Init(s_cutsceneTask*, struct s_cutsceneData* argument);
    static void Update(s_cutsceneTask*);

    u32 m0;
    u32 m4_changeField;
    s32 m8_fieldIndex;
    s32 mC_fieldParam;
    s32 m10_fieldExitIndex;
    s16 m14;
    u32 m18_frameCount;

    //size = 0x1C
};

// Parsing
s_cutsceneData* loadCutsceneData(sSaturnPtr EA);
void loadScriptData1(std::vector<s_scriptData1>& output, const sSaturnPtr& EA);

// Starting cutscenes
void startCutscene(s_cutsceneData* r4);
void startExitFieldCutscene2(p_workArea parent, s_cutsceneData* pScript, s32 param, s32 exitIndex, s32 arg0);
void startExitFieldCutscene2Sub0(p_workArea parent, s_cutsceneData* pScript, s32 fieldIndex, s32 param, s32 exitIndex, s32 arg0);

// Init sub-functions
u32 cutsceneTaskInitSub0(std::vector<s_scriptData3>& r4, std::vector<s_scriptData3>& r5);
void cutsceneTaskInitSub1(s_scriptData3* r15);

// Update sub-functions
void cutsceneTaskUpdateSub0();

// Camera
struct sFieldCameraManager;
struct sFieldCameraStatus;
void updateCutsceneCameraInterpolation(sFieldCameraManager* r4, sFieldCameraStatus* r5);
void startCutsceneCameraTracking(sVec3_FP* r4, sVec3_FP* r5);
void endCutsceneCameraWithRestore();
void endCutsceneCameraNoRestore();
