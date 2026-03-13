#include "PDS.h"
#include "d5_particles.h"
#include "o_fld_d5.h"
#include "field.h"
#include "kernel/vdp1AnimatedQuad.h"

static std::vector<sVdp1Quad> s_particleSpriteData;

// 06054a68
void s_nameEntryParticleTask::Init(s_nameEntryParticleTask* pThis)
{
    s_memoryAreaOutput memArea;
    getMemoryArea(&memArea, 2);
    pThis->m0_bundle = memArea.m0_mainMemoryBundle;
    pThis->m4_characterArea = memArea.m4_characterArea;
    pThis->m28_state = 0;

    // Load sprite data from overlay (0x06078364)
    if (s_particleSpriteData.empty())
    {
        s_particleSpriteData = initVdp1Quad(gFLD_D5->getSaturnPtr(0x06078364));
    }

    u16 vdp1Memory = 0;
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m8_pSubFieldData->mC_characterArea[2])
    {
        vdp1Memory = (u16)(pFieldTask->m8_pSubFieldData->mC_characterArea[2] >> 3);
    }

    // Initialize 64 particles with random positions
    for (s32 i = 0; i < 64; i++)
    {
        s_nameEntryParticle& particle = pThis->m30_particles[i];
        particleInitSub(&particle.m0_quad, vdp1Memory, &s_particleSpriteData);

        // Randomize initial position
        particle.m8_position.m0_X = (s32)(randomNumber() & 0x1FFFFF) - 0x100000;
        particle.m8_position.m4_Y = (s32)(randomNumber() & 0x1FFFFF) - 0x100000;
        particle.m8_position.m8_Z = (s32)(randomNumber() & 0xFFFFF) + 0x10000;

        // Randomize velocity
        particle.m14_velocityX = (s32)(randomNumber() & 0xFFF) - 0x800;
        particle.m18_velocityY = (s32)(randomNumber() & 0xFFF) - 0x800;

        // Randomize starting animation frame
        particle.m0_quad.m7_currentFrame = randomNumber() & 0xF;
    }
}

// 06054ae4
void s_nameEntryParticleTask::Update(s_nameEntryParticleTask* pThis)
{
    for (s32 i = 0; i < 64; i++)
    {
        s_nameEntryParticle& particle = pThis->m30_particles[i];
        sGunShotTask_UpdateSub4(&particle.m0_quad);
        particle.m8_position.m0_X += particle.m14_velocityX;
        particle.m8_position.m4_Y += particle.m18_velocityY;
    }
}

// 06054b16
void s_nameEntryParticleTask::Draw(s_nameEntryParticleTask* pThis)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex != 0)
    {
        // Not on name entry screen, skip to state-based handling
        goto stateSwitch2;
    }

    switch (pThis->m28_state)
    {
    case 0: // normal rendering
    {
        for (s32 i = 0; i < 64; i++)
        {
            drawProjectedParticle(&pThis->m30_particles[i].m0_quad, &pThis->m30_particles[i].m8_position);
        }
        break;
    }
    case 1: // alternate rendering
    {
        for (s32 i = 0; i < 64; i++)
        {
            drawProjectedParticle(&pThis->m30_particles[i].m0_quad, &pThis->m30_particles[i].m8_position);
        }
        break;
    }
    default:
    stateSwitch2:
        switch (pThis->m28_state)
        {
        case 2:
        case 3:
        {
            // Fade-out: render with decreasing count
            s32 count = 64 - (s32)pThis->m28_state * 16;
            if (count < 0) count = 0;
            for (s32 i = 0; i < count; i++)
            {
                drawProjectedParticle(&pThis->m30_particles[i].m0_quad, &pThis->m30_particles[i].m8_position);
            }
            break;
        }
        default:
            break;
        }
        break;
    }
}

// 06054c28
s_nameEntryParticleTask* createParticleSystemTask(p_workArea parent)
{
    return createSubTask<s_nameEntryParticleTask>(parent);
}
