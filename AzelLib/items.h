#pragma once

enum eItems : s16 {
    min = -1,
    mMinusOne = -1,

    m0_dummy = 0,
    m1_blastChip = 1,
    m2_dualBlastChip = 2,
    m3_triBlastChip = 3,
    m4_flashChip = 4,
    m5_elixirMinor = 5,
    m6_berserkMicro = 6,
    m7_dinsysChip = 7,
    m8_stolarium = 8,
    m9_shellPlate = 9,
    mA_gipsonLens = 0xA,
    mB_boneSlasher = 0xB,
    mC_olfactoryLobe = 0xC,
    mD_albertsonFan = 0xD,
    mE_nanoDrill = 0xE,
    mF_gemStone = 0xF,
    m10_cooliaDung = 0x10,

    // only  0x38 first will appear in battle as items

    m8C_recording = 0x8C, // imperial ship recording in A3

    // Quantities only valid for items up to 0x91 (the end of inventory and start of berzerks)

    mA6_unlearned = 0xA6, // used for skills not yet learned

    max = 0xB4,
};

struct sObjectListEntry
{
    s8 m0_flags;
    s8 m1_type;
    s8 m2;
    s8 m3;
    std::string m4_name;
    std::string m8_description;
};

const sObjectListEntry* getObjectListEntry(eItems entry);
s32 getObjectIcon(eItems objectID);
