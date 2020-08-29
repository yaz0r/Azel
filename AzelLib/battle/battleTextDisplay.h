#pragma once

struct sBattleTextDisplayTask : public s_workAreaTemplate<sBattleTextDisplayTask>
{
    sSaturnPtr m0_texts;
    sSaturnPtr m4;
    s_vdp2StringTask* m8;
    s16 m10;
    s16 m12_textIndex;
    s16 m14;
    s8 m16;
    s8 m17;
    s8 m18;
    s8 m19;
    s8 m1A;
    // size 0x1C
};

void createBattleTextDisplay(p_workArea parent, sSaturnPtr data);
void displayFormationName(short uParm1, char uParm2, char uParm3);
