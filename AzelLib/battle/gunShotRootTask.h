#pragma once

void createGunShotRootTask(s_workAreaCopy* parent);

s32 sGunShotTask_UpdateSub1Sub2(struct sBattleTargetable* pTargetable, s16 param_2, s32 param_3);
s32 isGunShotVisible(std::array<sVec3_FP, 2>& param_1, s_graphicEngineStatus_405C& param_2);

void sGunShotTask_DrawSub1Sub3(sMatrix4x3& param_1, fixedPoint& param_2, u16 param_3, s16 param_4, u16 param_5, const sF0Color* param_6, s32 param_7);
