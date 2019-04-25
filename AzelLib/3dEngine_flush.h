#pragma once

void flushObjectsToDrawList();

void NormalSpriteDrawGL(u32 vdp1EA);
void ScaledSpriteDrawGL(u32 vdp1EA);
void PolyLineDrawGL(u32 vdp1EA);

void drawDebugLine(const sVec3_FP& position1, const sVec3_FP& position2);
void drawDebugArrow(const sVec3_FP& position, const sVec3_FP& normal, const fixedPoint& magnitude);
