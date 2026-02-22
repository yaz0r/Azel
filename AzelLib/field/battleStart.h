#pragma once


struct sBattleLoadingTask : public s_workAreaTemplateWithArg<sBattleLoadingTask>
{
    s32 m0_enemyId;
    s32 m4;
    s32 m8_status;
    // size 0xC
};


extern const sBattleLoadingTask::TypedTaskDefinition battleStartTaskDefinition;
