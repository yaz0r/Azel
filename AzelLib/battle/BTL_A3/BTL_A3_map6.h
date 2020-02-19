#pragma once

struct s_BTL_A3_Env : public s_workAreaTemplate<s_BTL_A3_Env>
{
    s32 m38;
    npcFileDeleter* m58;
    // 0x9C
};

void initGridForBattle(npcFileDeleter* pFile, sSaturnPtr r5_envConfig, s32 r6_sizeX, s32 r7_sizeY, s32 r8_cellSize);
void s_BTL_A3_Env_InitVdp2(s_BTL_A3_Env* pThis);
void s_BTL_A3_Env_Update(s_BTL_A3_Env* pThis);
void s_BTL_A3_Env_Draw(s_BTL_A3_Env* pThis);

p_workArea Create_BTL_A3_map6(p_workArea);
