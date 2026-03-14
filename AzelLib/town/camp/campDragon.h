#pragma once

struct sTownObject* createCampDragon(p_workArea parent, sSaturnPtr arg);
void initDragonHotpoints(struct sTownDragon* pThis);
s32 sCampDragon_InitSub1(struct sTownDragon* pThis);
void increaseGameResource(int param_1, int param_2);
void sCampDragon_startAnimSequence(struct sTownDragon* pThis, sSaturnPtr cmdPtr);
