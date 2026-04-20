#pragma once

#include "shared/vdp2PlaneTask.h"

p_workArea Create_BTL_A5_env(p_workArea parent);
p_workArea Create_BTL_A5_env_grid(p_workArea parent);

void BTL_A5_buildGroundRotation(sVdp2PlaneTask* pThis);
void BTL_A5_env_Draw(sVdp2PlaneTask* pThis);

struct sBTL_A5_PaletteAnim;
sBTL_A5_PaletteAnim* createPaletteAnimTask(p_workArea parent);
