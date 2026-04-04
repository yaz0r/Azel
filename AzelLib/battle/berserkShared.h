#pragma once

struct sVec3_FP;
struct s_battleEngine;

// Shared helpers used across berserk effect files

// BTL_A3::060a2fc8 — inner camera orbit computation
void berserkCameraOrbitInner(sVec3_FP* pOut, u32 angle, s32* pPerQuadrantAltitude);

// BTL_A3::060a3120 — quadrant-based camera orbit wrapper
void berserkCameraOrbit(sVec3_FP* pOut, s32 rotAngle, s32* pPerQuadrantAltitude);

// Helper: read camera data from overlay battle data table
sSaturnPtr getOverlayCameraData(s_battleEngine* pEngine);

sVec3_FP readOverlayCameraOffset(s_battleEngine* pEngine, s8 quadrant);

// Offset 0x30 in the sub-battle camera block (used for onslaught camera lerp target)
sVec3_FP readOverlayCameraOffset2(s_battleEngine* pEngine);

// Hit flash fade task
void berserk_createHitFlashFade(p_workArea pParent, s16 clofsl, u16 color1, u16 color2, u16 duration1, u16 color3, u16 color4, u16 duration2);

// Helper: sign-extend 28-bit angle to 32-bit
s32 signExtend28(s32 v);

// Helper: fixed-point to color byte (0..255)
u8 fpToColorByte(fixedPoint v);

// 060073f8 — scale base damage by dragon SPR stat
s16 computeSprScaledDamage(s16 baseDamage);

// BTL_A3::0608d7f0 — compute berserk base damage scaled by SPR
s16 phantomSlasher_getBaseDamage(s16 baseDamage);

// BTL_A3::060a2e78
s32 battleTypeId_match(u16 battleId, u8 category);

// BTL_A3::060a2eb6 — get current battle type ID
s8 getBattleTypeId();

// BTL_A3::060a2efc — random spherical offset for bolt scatter
sVec3_FP randomSphericalOffset(s32 maxRadius);

// BTL_A3::060a2efc — init per-quadrant camera altitude deltas
void initPerQuadrantCameraAltitude(s32* pOut);
