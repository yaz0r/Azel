#pragma once

/*
" 1 PCM OFF MODE      "
" 2 SHOT EQUIP CHANGE "
" 3 LASER EQUIP CHANGE"
" 4 PRELOCK CHANGE    "
" 5 COLLI ENTRY NUMBER"
" 6 CAMERA SPRING     "
" 7 CAMERA FIX        "
" 8 CAMERA DISTANCE   "
" 9 CAMERA SCRNANG    "
"10 PLAYER SPEED      "
"11 AXIS ROTATE       "
"12 SCROLL DOT RATIO  "
"13 CAM DATA DISP     "
"14 LIGHT SETTING     "
"15 COLLI SPHERE      "
"16 COLLI VALUE DISP  "
"17 GAUGE INVALID     "
"18 BTL DATA DISP     "
"19 NO DEATH MODE     "
"20 CYLINDER SIZE     "
"21 FUTAKAWA MODE     "
"22 YAMAKEN MODE      "
"23 MITA MODE         "
"24 SHINYA MODE       "
"25 YASUHARA MODE     "
"26 TAKAMA MODE       "
"27 MOVE MODE         "
"28 HEIGHT SET        "
"29 ENEMY GAUGE DISP  "
*/

struct s_battleDebug : public s_workAreaTemplate<s_battleDebug>
{
    std::array<s8, 0x22> mFlags;
    //size 0x22
};

void createInBattleDebugTask(p_workArea parent);

