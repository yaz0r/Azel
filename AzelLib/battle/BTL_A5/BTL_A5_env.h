#pragma once

p_workArea Create_BTL_A5_env(p_workArea parent);
p_workArea Create_BTL_A5_env_grid(p_workArea parent);

void BTL_A5_buildGroundRotation(struct s_BTL_A3_Env* pThis);
void BTL_A5_env_Draw(struct s_BTL_A3_Env* pThis);

struct sBTL_A5_PaletteAnim;
sBTL_A5_PaletteAnim* createPaletteAnimTask(p_workArea parent);
