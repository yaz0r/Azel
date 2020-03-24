#pragma once

struct s_BTL_A3_Env : public s_workAreaTemplate<s_BTL_A3_Env>
{
    s32 m38;
    s32 m3C;
    sSaturnPtr m40;
    u32 m44;
    u32 m48;
    s32 m4C;
    s8 m50;
    s8 m51;
    s8 m52;
    s8 m55;
    npcFileDeleter* m58;
    // 0x9C
};

void initGridForBattle(npcFileDeleter* pFile, sSaturnPtr r5_envConfig, s32 r6_sizeX, s32 r7_sizeY, s32 r8_cellSize);
void s_BTL_A3_Env_InitVdp2(s_BTL_A3_Env* pThis);
void s_BTL_A3_Env_Update(s_BTL_A3_Env* pThis);
void s_BTL_A3_Env_Draw(s_BTL_A3_Env* pThis);

p_workArea Create_BTL_A3_map6(p_workArea);
