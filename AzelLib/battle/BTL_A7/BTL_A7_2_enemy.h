#pragma once

#include "battle/battleFormationBase.h"

struct sBTL_A7_2_FormationTask;

p_workArea createAtolmBodyEntity(sBTL_A7_2_FormationTask* pFormation, sEntityGroup* pGroup, int entityIndex);
p_workArea createAtolmArmEntity(sBTL_A7_2_FormationTask* pFormation, sEntityGroup* pGroup, int entityIndex, int param4);
