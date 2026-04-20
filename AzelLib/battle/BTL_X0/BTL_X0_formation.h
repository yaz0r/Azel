#pragma once

struct sBTL_X0_EnemyModel;

struct sBTL_X0_FormationTask : public s_workAreaTemplate<sBTL_X0_FormationTask>
{
    u8 m0_pad[0x18];
    s8 m18_subBattleParam;
    u8 m19_pad[0x64 - 0x19];
    u32 m64_formationData;
    u16 m68_counter;
    u16 m6A_counter2;
    u8 m6C_pad[4];
    s8 m70_state;
    u8 m71_pad[3];
    p_workArea m74_enemyModelTask;
    p_workArea m78_introEffectTask;
    s8 m7C_subState;
    s8 m7D_act0;
    s8 m7E_act1;
    s8 m7F_variantIndex;
    s8 m80_mode1a;
    s8 m81_mode1b;
    s8 m82_mode2a;
    s8 m83_mode2b;
    s8 m84_mode3a;
    s8 m85_mode3b;
    s8 m86_mode4a;
    s8 m87_mode4b;
    // size 0x88
};

void BTL_X0_createFormation0(s_workAreaCopy* pParent, u32 arg0);
void BTL_X0_createFormation1(s_workAreaCopy* pParent, u32 arg0);
void BTL_X0_createFormation2(s_workAreaCopy* pParent, u32 arg0);
