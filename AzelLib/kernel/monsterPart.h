#pragma once

#include <vector>

// Shared segmented-body primitive used by monsters (A3 Baldor, T0 middle
// boss), several town cutscene props, and A5 subfield-0 worm-hole decor.
// Named "monsterPart" after the default update/draw/drawPart callbacks
// (monsterPart_defaultUpdate / Draw / DrawPart) — callers that wanted the
// spring-damping physics variant switch the callbacks to baldorPart_* via
// monsterBody_loadPartData(body, 1, ...).

struct sMonsterBodyPart
{
    sMonsterBodyPart* m0_child;
    sVec3_FP          m4_worldPosition;
    sVec3_FP          m10_translation;
    sVec3_FP          m1C_rotation;
    sVec3_FP          m28_rotationVelocity;
    sVec3_FP          m34_rotationAcceleration;
    s16               m40_modelIndex;
    sVec3_FP          m44_springStiffness;
    fixedPoint        m50_damping;
    // Saturn size 0x54
};

struct sMonsterBody
{
    sVec3_FP          m0_translation;
    sVec3_FP          mC_rotation;
    sVec3_FP          m18_rotationTarget;
    void (*m24_update)   (sMonsterBodyPart*, const sVec3_FP*, const sVec3_FP*, const sVec3_FP*);
    // m28_draw/m2C_drawPart are typed as (bundle, part) in C++ instead of
    // (task, part) because the Saturn versions read the bundle via
    // `*task` (first field). C++ task layouts don't put the bundle at
    // offset 0, so the caller extracts it and passes it directly.
    void (*m28_draw)     (struct s_fileBundle*, sMonsterBodyPart*);
    void (*m2C_drawPart) (struct s_fileBundle*, sMonsterBodyPart*);
    std::vector<sMonsterBodyPart> m30_parts;
    // Saturn size 0x34
};

// --- Construction ---

// 06013C72 — allocate a new sMonsterBody with `numEntries` parts linked
// head→tail, default callbacks installed.
sMonsterBody* monsterBody_create(p_workArea parent, int numEntries);

// 06013D80 — load per-part data from a Saturn array. When arg2 == 1 the
// callbacks are swapped for baldorPart_update/draw/drawPart (spring/damping
// physics), otherwise they stay as the monsterPart_default* stubs.
void monsterBody_loadPartData(sMonsterBody* pThis, int arg2, sSaturnPtr arg3);

// 06013E02 — copy a single 0x14-byte Saturn record into a part
// (`m40_modelIndex`, `m44_springStiffness`, `m50_damping`).
void monsterBodyPart_loadTemplate(sMonsterBodyPart* dest, sSaturnPtr source);

// 06013C70 — seed the body's translation/rotation/rotationTarget from
// `pPos`/`pRot`, reset every linked part's rotation state, then kick the
// body's update callback once so the parts settle into the initial pose.
void monsterBody_initPose(sMonsterBody* pBody, const sVec3_FP* pPos, const sVec3_FP* pRot);

// --- Default callbacks ---

void monsterPart_defaultUpdate(sMonsterBodyPart*, const sVec3_FP*, const sVec3_FP*, const sVec3_FP*);
void monsterPart_defaultDraw(struct s_fileBundle*, sMonsterBodyPart*);
void monsterPart_defaultDrawPart(struct s_fileBundle*, sMonsterBodyPart*);
