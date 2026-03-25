#include "PDS.h"
#include "BTL_A3_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleDragon.h"
#include "battle/battleGenericData.h"
#include "battle/particleEffect.h"
#include "audio/systemSounds.h"
#include "mainMenuDebugTasks.h"

p_workArea BaldorAttack_createAttackModel(sVec3_FP* partPosition, sVec3_FP* target, sSaturnPtr param3);

// ============================================================
// Pattargo healer particle effect (urchin variant 7 idle glow)
// ============================================================

struct sPattergoHealerEffectTask : public s_workAreaTemplate<sPattergoHealerEffectTask>
{
    sVec3_FP* m0_positionPtr;
};

// 06054d40
static void PattergoHealerParticleEffect_update(sPattergoHealerEffectTask* pThis)
{
    int local_1c[3];
    randomNumber();
    u32 uVar1 = randomNumber();
    local_1c[0] = (uVar1 & 0xff) - 0x7f;
    uVar1 = randomNumber();
    local_1c[1] = (uVar1 & 0xff) - 0x7f;
    uVar1 = randomNumber();
    local_1c[2] = (uVar1 & 0xff) - 0x7f;
    int iVar2 = randomNumber();
    createParticleEffect(
        dramAllocatorEnd[14].mC_fileBundle,
        &g_BTL_GenericData->m_0x60a7848_animatedQuad,
        pThis->m0_positionPtr,
        (sVec3_FP*)local_1c,
        nullptr,
        0x10000, 0,
        (s16)(((u16)iVar2 & 0x1f) + 0xf));
}

// 06054db0
p_workArea createPattergoHealerParticleEffect(s_workAreaCopy* pParent)
{
    sPattergoHealerEffectTask* pNewTask = createSubTaskFromFunction<sPattergoHealerEffectTask>(pParent, &PattergoHealerParticleEffect_update);
    pNewTask->m0_positionPtr = (sVec3_FP*)((u8*)pParent + 8);
    return pNewTask;
}

// ============================================================
// Pattargo multi-projectile spread attack
// ============================================================

struct sPattergoMultiProjectileTask : public s_workAreaTemplate<sPattergoMultiProjectileTask>
{
    std::vector<s_workArea*> m0_taskPtrArray;
    s16 m4_count;
};

// 06056604
static void pattergoMultiProjectile_update(sPattergoMultiProjectileTask* pThis)
{
    bool bVar1 = false;
    for (int i = 0; i < pThis->m4_count; i++)
    {
        if (pThis->m0_taskPtrArray[i] != nullptr)
        {
            if (pThis->m0_taskPtrArray[i]->getTask()->isFinished())
            {
                pThis->m0_taskPtrArray[i] = nullptr;
            }
            else
            {
                bVar1 = true;
            }
        }
    }
    if (!bVar1)
    {
        pThis->getTask()->markFinished();
    }
}

// 0605699c
p_workArea createPattergoMultiParticleEffect(s_workAreaCopy* pParent)
{
    sPattergoMultiProjectileTask* pNewTask = createSubTaskFromFunction<sPattergoMultiProjectileTask>(pParent, &pattergoMultiProjectile_update);
    pNewTask->m4_count = 8;
    pNewTask->m0_taskPtrArray.resize(pNewTask->m4_count, nullptr);

    int iStack_6c = 2;
    for (int i = 0; i < pNewTask->m4_count; i++)
    {
        sVec3_FP sStack_5c;
        sStack_5c.m0_X = ((sVec3_FP*)((u8*)pParent + 8))->m0_X;
        sStack_5c.m4_Y = ((sVec3_FP*)((u8*)pParent + 8))->m4_Y;
        sStack_5c.m8_Z = ((sVec3_FP*)((u8*)pParent + 8))->m8_Z;

        sVec3_FP iStack_68;
        iStack_68.m0_X = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m0_X;
        iStack_68.m4_Y = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m4_Y;
        iStack_68.m8_Z = gBattleManager->m10_battleOverlay->m18_dragon->m8_position.m8_Z;

        iStack_6c--;
        if (iStack_6c < 0)
        {
            u32 uVar3 = randomNumber();
            iStack_68.m0_X += (s32)((uVar3 & 0x3ffff) - 0x1ffff);
            uVar3 = randomNumber();
            iStack_68.m4_Y += (s32)((uVar3 & 0x3ffff) - 0x1ffff);
            uVar3 = randomNumber();
            iStack_68.m8_Z += (s32)((uVar3 & 0x3ffff) - 0x1ffff);
        }

        // TODO: build proper attack params struct (hardcoded params, sprite at 0x060ae668, delay randomized)
        pNewTask->m0_taskPtrArray[i] = BaldorAttack_createAttackModel(&sStack_5c, &iStack_68, g_BTL_A3->getSaturnPtr(0x060ae668));
    }
    playSystemSoundEffect(0x68);
    return pNewTask;
}

// ============================================================
// Single projectile spawn
// ============================================================

// 06054dce
p_workArea urchinAttack_spawnProjectile(s_workAreaCopy* pParent)
{
    playSystemSoundEffect(0x69);
    return BaldorAttack_createAttackModel(
        (sVec3_FP*)((u8*)pParent + 8),
        (sVec3_FP*)((u8*)gBattleManager->m10_battleOverlay->m18_dragon + 8),
        g_BTL_A3->getSaturnPtr(0x060a7b90));
}
