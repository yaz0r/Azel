#pragma once

struct s_battleEngine;

// BTL_A3::06080b5c — Phantom Slashers / Wraith Slashers / Onslaught
void berserk_createPhantomSlashers(s_battleEngine* pThis, u8 level);

// BTL_A3::06096530 — Chain Laser / Prism Laser
s32 berserk_createLaser(s_battleEngine* pThis, u8 type);

// BTL_A3::06099f40 — Cleansing Wave
void berserk_createCleansingWave(s_battleEngine* pThis);

// BTL_A3::06085c3c — Dragon Phoenix
void berserk_createDragonPhoenix(s_battleEngine* pThis);

// BTL_A3::0607df10 — Plasma Vortex
void berserk_createPlasmaVortex(s_battleEngine* pThis);

// BTL_A3::06091998 — Lightning Storm
void berserk_createLightningStorm(s_battleEngine* pThis);

// BTL_A3::06094f68 — Plasma Swarm
void berserk_createPlasmaSwarm(s_battleEngine* pThis);

// BTL_A3::060928c4 — Berserker Rage
void berserk_createBerserkerRage(s_battleEngine* pThis, s32 attackType, sVec3_FP* pHotpoint);

// BTL_A3::0608f100 — Laser Storm
void berserk_createLaserStorm(s_battleEngine* pThis);

// BTL_A3::06083b08 — Hunting Scythe
void berserk_createHuntingScythe(s_battleEngine* pThis);

// BTL_A3::0608add8 — Energy Prism
void berserk_createEnergyPrism(s_battleEngine* pThis);

// BTL_A3::0608f3a0 — Vengeance Orbs shield visual
void berserk_createVengeanceOrbsShield(s_battleEngine* pThis);

// BTL_A3::06096f8c — Astral Phantoms shield visual
void berserk_createAstralPhantomsShield(s_battleEngine* pThis);
