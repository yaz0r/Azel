#pragma once

// 0x14-byte entry in the destination/encounter list
struct sRadarDestEntry
{
    const char* m0_text;     // 0x00 — display string
    s32 m4;                  // 0x04
    s32 m8;                  // 0x08
    s32 mC;                  // 0x0C
    s32 m10;                 // 0x10
    // size 0x14
    // Accessed via raw offsets from entry base:
    //   offset 0x14 (entry[5]) = script arg / choice value (next entry's m0)
    //   offset 0x18 (entry[6]) = bit index for availability check (next entry's m4)
    // These overlap with the NEXT entry — the list is read as flat s32 array
};

struct s_FieldRadar : public s_workAreaTemplate<s_FieldRadar>
{
    s_memoryAreaOutput m0;
    s16 m8;
    s16 mA;
    s16 mC_X;
    s16 mE_Y;
    s16 m10;
    s16 m12;
    s32 m14_encounterCount;
    s32 m18_currentSelection;
    sRadarDestEntry* m1C_encounterList;
    s32* m20_choiceArray;        // heap-allocated filtered choice pointers
    s32 m24_filteredChoiceCount;
    s16 m28;
    s16 m2A;
    s32 m30;
    s32 m34;
    fixedPoint m38_dragonDirection;
    u32 m3C;
    s32 m40_paletteFadeDuration;
    s8 m44;
    s8 m48;
    s8 m49;
    s8 m4A_newPaletteIndex;
    s8 m4B;
    s8 m4C_dangerLevel;
    s32 m50;
    s32 m54;
    s32 m58_altitudeGaugeEnabled;
    s8 m59;
    s8 m5A_isHidden;
    s8 m5B;
    s32 m5C;
    s32 m60;
    fixedPoint m64_encounterDistance;
    fixedPoint m68_encounterDistanceSq;
};

void fieldRadar_hide();
void fieldRadar_show();
void fieldRadar_enableAltitudeGauge();
void fieldRadar_disableAltitudeGauge();

void createFieldRadar(s_workArea* pWorkArea);
void fieldRadar_setEncounterDistance(fixedPoint distance);
void fieldRadar_initRandomBattle(s32 scriptIndex);
