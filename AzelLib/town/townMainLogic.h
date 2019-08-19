#pragma once

#include "town.h"

struct sMainLogic : public s_workAreaTemplate<sMainLogic>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sMainLogic::Init, &sMainLogic::Update, &sMainLogic::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(sMainLogic* pThis);
    static void Update(sMainLogic* pThis);
    static void Draw(sMainLogic* pThis);

    s8 m0;
    s8 m1_cameraParamsIndex;
    s8 m2_cameraFollowMode;
    s8 m3;
    s32 m4_flags;
    s32 m8_inputX;
    s32 mC_inputY;
    void (*m10)(sMainLogic*);
    struct sEdgeTask* m14_EdgeTask;
    sVec3_FP m18_position;
    fixedPoint m24_distance;
    fixedPoint m2C;
    fixedPoint m30;
    sVec3_FP m38_interpolatedCameraPosition;
    sVec3_FP m44_cameraTarget;
    sVec3_FP m50_upVector;
    sVec3_FP m5C_rawCameraPosition;
    sVec3_FP m68_cameraRotation;
    sMainLogic_74 m74_townCamera;
    s32 m118_autoWalkDuration;
    sVec3_FP m11C_autoWalkStartPosition;
    sVec3_FP m128_autoWalkStartRotation;
    sVec3_FP m134_autoWalkPositionStep;
    // size 0x140
};

extern sMainLogic* twnMainLogicTask;

p_workArea startMainLogic(p_workArea pParent);
void mainLogicInitSub2();
