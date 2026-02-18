#pragma once

void displayRaySegment(std::array<sVec3_FP, 2>& param_1, s32 param_2, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode);
s32 rayComputeDisplayMatrix_fixedWidth(std::array<sVec3_FP, 2>& param_1, s32 param_2, s_graphicEngineStatus_405C& param_3, sMatrix4x3& param_4);
s32 rayComputeDisplayMatrix_2Width(std::array<sVec3_FP, 2>& param_1, std::array<fixedPoint, 2>& param_2, s_graphicEngineStatus_405C& param_3, sMatrix4x3* param_4);
