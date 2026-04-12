#include "PDS.h"
#include "o_fld_a5.h"
#include "audio/systemSounds.h"

// sA5ExitEntity is declared in o_fld_a5.h — this file owns its implementation.
// The entity is created by createA5_exitEntityTask (shared) for every A5 subfield
// and drives the wind-state machine + per-mode particle spawning near the exit.

// 06058514 — exit entity init: sets up orbit params and stores self in field data
static void a5ExitEntity_Init(sA5ExitEntity* p)
{
    p->m30_radius = 0xAAA;
    p->m35 = 0;
    p->m36 = 0;
    p->m18 = 0;
    p->m0 = 0;
    p->m4_angle = 0x1C71C71;
    p->m8 = 0;

    u32 angleIdx = ((u32)p->m4_angle >> 16) & 0xFFF;
    p->mC_direction.m0_X = MTH_Mul(fixedPoint(p->m18), getSin(angleIdx));
    p->mC_direction.m4_Y = 0;
    p->mC_direction.m8_Z = MTH_Mul(fixedPoint(p->m18), getCos(angleIdx));

    p->m34 = 0;

    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    pFieldData->m4_pExitEntity = p;
}

// 060586a0 — wind state machine: manages wind speed transitions
static void a5ExitEntity_WindStateMachine(sA5ExitEntity* p)
{
    switch (p->m35)
    {
    case 0: // calm
        p->m30_radius = 0x800;
        p->m36--;
        if (p->m36 < 1)
        {
            p->m36 = 0;
            if ((randomNumber() & 0xFFFF) < 0x3334)
            {
                p->m36 = 0x78;
                p->m35 = 1;
            }
        }
        break;
    case 1: // moderate
        p->m30_radius = 0x1000;
        p->m36--;
        if (p->m36 < 1)
        {
            p->m36 = 0;
            if ((randomNumber() & 0xFFFF) < 0x7AF)
            {
                p->m35 = 2;
            }
            else if ((randomNumber() & 0xFFFF) < 0x199A)
            {
                p->m36 = 0x78;
                p->m35 = 0;
            }
        }
        break;
    case 2: // ramp up
        p->m30_radius = 0x2AAA;
        p->m35 = 3;
        break;
    case 3: // strong — wait for speed to reach threshold
        if (p->m18 > 0x27FF)
        {
            playSystemSoundEffect(0x67);
            p->m36 = 0x5A;
            p->m35 = 4;
        }
        break;
    case 4: // cooldown
        p->m36--;
        if (p->m36 < 1)
        {
            p->m36 = 0x96;
            p->m35 = 1;
        }
        break;
    default:
        p->m35 = 0;
        break;
    }
}

// 060587A8 — exit mode 0: wind particles near camera
static void a5ExitEntity_Mode0(sA5ExitEntity* p)
{
    a5ExitEntity_WindStateMachine(p);
    Unimplemented(); // particle spawning near camera position based on wind speed
}

// 0605887C — exit mode 1: directional particles near exit position
static void a5ExitEntity_Mode1(sA5ExitEntity* p)
{
    a5ExitEntity_WindStateMachine(p);
    Unimplemented(); // particle spawning near exit target position
}

// 06058944 — exit mode 3: periodic particle spawning
static void a5ExitEntity_Mode3(sA5ExitEntity* p)
{
    p->m34++;
    if (p->m34 > 8)
    {
        p->m34 = 0;
        Unimplemented(); // spawn particle near camera with height from m24_exitTargetX_mode3
    }
}

// 0605857E
static void a5ExitEntity_Update(sA5ExitEntity* p)
{
    // Debug display (only when debug flags active)
    if ((getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 4) != 0
        && getFieldTaskPtr()->m8_pSubFieldData->m37E_debugMenuStatus2_a == 0
        && getFieldTaskPtr()->m8_pSubFieldData->m369 == 0)
    {
        // Debug wind info display — skipped in non-debug builds
    }

    // Dispatch to exit mode handler
    switch (p->m2C_exitMode)
    {
    case 0: a5ExitEntity_Mode0(p); break;
    case 1: a5ExitEntity_Mode1(p); break;
    case 2: break; // 06058940 — empty
    case 3: a5ExitEntity_Mode3(p); break;
    default: break;
    }

    // Interpolate approach speed toward target radius
    p->m18 = p->m18 + ((p->m30_radius - p->m18) >> 4);

    // Recompute direction vector from angle
    u32 angleIdx = ((u32)p->m4_angle >> 16) & 0xFFF;
    p->mC_direction.m0_X = MTH_Mul(fixedPoint(p->m18), getSin(angleIdx));
    p->mC_direction.m8_Z = MTH_Mul(fixedPoint(p->m18), getCos(angleIdx));
}

// 060589be
void createA5_exitEntityTask(p_workArea parent)
{
    static sA5ExitEntity::TypedTaskDefinition td = { &a5ExitEntity_Init, &a5ExitEntity_Update, nullptr, nullptr };
    createSubTask<sA5ExitEntity>(parent, &td);
}

// 060584ca — set field exit configuration
void setFieldExitConfig_A5(s32 mode, s32* pParams)
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    sA5ExitEntity* pExit = pFieldData->m4_pExitEntity;
    if (mode < 4)
    {
        pExit->m2C_exitMode = mode;
        if (mode == 1)
        {
            pExit->m1C_exitTargetX_mode1 = pParams[0];
            pExit->m20_exitTargetZ_mode1 = pParams[2];
        }
        else if (mode == 3)
        {
            pExit->m24_exitTargetX_mode3 = pParams[0];
            pExit->m28_exitTargetZ_mode3 = pParams[1];
        }
    }
}
