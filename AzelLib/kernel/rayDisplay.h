#pragma once

void displayRaySegment(std::array<sVec3_FP, 2>& param_1, s32 param_2, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode);
void displayRaySegment_2Width(std::array<sVec3_FP, 2>& param_1, std::array<fixedPoint, 2>& param_2, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode);
s32 rayComputeDisplayMatrix_fixedWidth(std::array<sVec3_FP, 2>& param_1, s32 param_2, s_graphicEngineStatus_405C& param_3, sScreenQuad3& param_4);
s32 rayComputeDisplayMatrix_2Width(std::array<sVec3_FP, 2>& param_1, std::array<fixedPoint, 2>& param_2, s_graphicEngineStatus_405C& param_3, sScreenQuad3& param_4);

void displayRaySegmentFromViewSpace(std::array<sVec3_FP, 2>& viewSpacePoints, s32 width, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode);

extern bool gDirectRayRendering;
void flushRayQuads3D();

// 3D colored-line primitive used by shared VDP1 line submitters (e.g.
// vdp1EmitRayVertex_0602e136) when gDirectRayRendering is enabled. The
// endpoints are in VIEW space (post current-matrix transform). Colour for
// each endpoint is derived from the VDP1 CMDCOLR base (direct-colour RGB555
// when bit 15 is set) modulated by a per-vertex VDP1 gouraud offset
// (5-bit signed per channel, biased at 16). BGFX interpolates the colour
// across the segment automatically. The projection-only path in
// flushRayLines3D mirrors flushRayQuads3D.
void enqueueRayLine3D(const sVec3_FP& viewSpaceP0, const sVec3_FP& viewSpaceP1,
                      u16 cmdcolr, u16 gouraud0, u16 gouraud1);
void flushRayLines3D();
