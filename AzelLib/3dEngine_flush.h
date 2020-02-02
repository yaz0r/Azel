#pragma once

void flushObjectsToDrawList();

void NormalSpriteDrawGL(u32 vdp1EA);
void ScaledSpriteDrawGL(u32 vdp1EA);
void PolyLineDrawGL(u32 vdp1EA);
void PolyDrawGL(u32 vdp1EA);

struct sFColor
{
    float R;
    float G;
    float B;
    float A;
};

void drawDebugFilledQuad(const sVec3_FP& position0, const sVec3_FP& position1, const sVec3_FP& position2, const sVec3_FP& position3, const sFColor& color = { 1,0,0,1 });
void drawDebugLine(const sVec3_FP& position1, const sVec3_FP& position2, const sFColor& color = { 1,0,0,1 });
void drawDebugArrow(const sVec3_FP& position, const sVec3_FP& normal, const fixedPoint& magnitude);
