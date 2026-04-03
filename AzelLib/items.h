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
    m7_dinysChip = 7,
    m8_stolarium = 8,
    m9_shellPlate = 9,
    mA_gipsonLens = 0xA,
    mB_boneSlasher = 0xB,
    mC_olfactoryLobe = 0xC,
    mD_albertsonFan = 0xD,
    mE_nanoDrill = 0xE,
    mF_gemStone = 0xF,
    m10_cooliaDung = 0x10,
    m11_gara = 0x11,
    m12_goliaShell = 0x12,
    m13_goliaFang = 0x13,
    m14_goliaTail = 0x14,
    m15_goliaPod = 0x15,
    m16_relicShoes = 0x16,
    m17_zaalNuts = 0x17,
    m18_lampOil = 0x18,
    m19_gemLens = 0x19,
    m1A_amirkhanRing = 0x1A,
    m1B_zoahHarvest = 0x1B,
    m1C_macralLiquid = 0x1C,
    m1D_macralShell = 0x1D,
    m1E_ascuncionMeat = 0x1E,
    m1F_feenerArmor = 0x1F,
    m20_lickuernEyes = 0x20,
    m21_lindstrumUnit = 0x21,
    m22_relicWeaponry = 0x22,
    m23_relicArmor = 0x23,
    m24_relicEngine = 0x24,
    m25_macranJewel = 0x25,
    m26_elixirMedis = 0x26,
    m27_elixirMaxis = 0x27,
    m28_fullElixir = 0x28,
    m29_berserkMinor = 0x29,
    m2A_berserkMedis = 0x2A,
    m2B_berserkMaxis = 0x2B,
    m2C_ambrosia = 0x2C,
    m2D_revive = 0x2D,
    m2E_antidote = 0x2E,
    m2F_recover = 0x2F,
    m30_shieldChip = 0x30,
    m31_powerChip = 0x31,
    m32_armorChip = 0x32,
    m33_speedChip = 0x33,
    m34_restoreSpeed = 0x34,
    m35_freeAction = 0x35,
    m36_anesthetic = 0x36,
    m37_unbind = 0x37,
    m38_telepathyShard = 0x38,

    // only 0x38 first will appear in battle as items

    // Gun types (mA_weaponType values, set when equipping gun parts)
    m39_notEquip = 0x39,        // Default / no gun equipped
    m3A_highVulcan = 0x3A,      // High fire rate, 5 shots
    m3B_mauler = 0x3B,          // Standard, 6 shots
    m3C_pulverizer = 0x3C,      // High damage, 5 shots
    m3D_berserkVampire = 0x3D,  // Restores HP, 5 shots
    m3E_berserkLeech = 0x3E,    // Restores BP, 5 shots
    m3F_sniper = 0x3F,          // Ignores defense, 5 shots
    m40_triBurst = 0x40,        // Fires at 3 targets, 5 shots
    m41_assassin = 0x41,        // Chance for critical, 1 shot

    // Debug items (0x42-0x4C)
    m42_HPMAX = 0x42,
    m43_BPMAX = 0x43,
    m44_NORMAL = 0x44,
    m45_NOENEMY = 0x45,
    m46_HPBPMAX = 0x46,
    m47_DONTRUN = 0x47,
    m48_TWICE = 0x48,
    m49_EQUIPGUN = 0x49,
    m4A_TYPEMISS = 0x4A,
    m4B_NOBP = 0x4B,
    m4C_NOGAUGE = 0x4C,

    // Key items (0x4D+)
    m55_dragonCrest = 0x55,
    m69_pictureBoard = 0x69,
    m8C_recording = 0x8C,

    // Berserks (0x91+)
    m91_phantomSlashers = 0x91,
    m92_wraithSlashers = 0x92,
    m93_onslaught = 0x93,
    m94_vengeanceOrbs = 0x94,
    m95_judgementDay = 0x95,
    m96_armageddon = 0x96,
    m97_cleansingWave = 0x97,
    m98_dragonPhoenix = 0x98,
    m99_prismLaser = 0x99,
    m9A_assaultWing = 0x9A,
    m9B_plasmaVortex = 0x9B,
    m9C_lightningStorm = 0x9C,
    m9D_plasmaSwarm = 0x9D,
    m9E_berserkerRage = 0x9E,
    m9F_laserStorm = 0x9F,
    mA0_chainLaser = 0xA0,
    mA1_holySphere = 0xA1,
    mA2_huntingScythe = 0xA2,
    mA3_astralPhantoms = 0xA3,
    mA4_energyPrism = 0xA4,
    mA5_shield = 0xA5,

    mA6_unlearned = 0xA6, // also "Heal" placeholder for skills not yet learned

    mAA_healMaxis = 0xAA,
    mAB_genesis = 0xAB,
    mAC_berserkerWing = 0xAC,
    mAD_recover = 0xAD,
    mAE_protectionWing = 0xAE,
    mAF_swiftWing = 0xAF,
    mB0_escape = 0xB0,
    mB1_healingWing = 0xB1,
    mB2_fieldMap = 0xB2,

    max = 0xB4,
};

// Dragon status modifier bitfield (s_battleDragon::m1C0_statusModifiers)
// 0x001 = enables certain berserk actions
// 0x002 = status 2
// 0x004 = Poisoned
// 0x008 = Dead
// 0x010 = status: enables homing laser bonus
// 0x020 = status: affects power gauge
// 0x07F = any negative status mask
// 0x080 = Attack buff active (Power Chip / berserk)
// 0x100 = Defense buff active (Armor Chip / berserk)
// 0x200 = Agility/Speed buff active (Speed Chip / berserk, clears poison)
// 0x400 = Shield/Barrier active (Shield Chip)
// 0x8000 = temporary flag (cleared after processing)
// 0x20000 = laser charge state

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
