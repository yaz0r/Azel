#pragma once

struct sBattleTargetable;

void createDamageNumberFromTargetable(s_workAreaCopy* parent, sBattleTargetable* targetable, s32 animMode);
void createDamageNumberFromValue(s_workAreaCopy* parent, s16 damageValue, sVec3_FP* position);
