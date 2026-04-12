#include "PDS.h"
#include "o_fld_a5.h"
#include "field/fieldModelRender.h"
#include "field/fieldVisibilityGrid.h"
#include "kernel/fileBundle.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "3dEngine.h"

// Weather/fog task (size 0xD0)
struct sA5WeatherTask : public s_workAreaTemplate<sA5WeatherTask>
{
    s_memoryAreaOutput m0_memoryArea;
    sAnimatedQuad m8_quad;
    s32 m10_velocities[0x2F]; // 47 random velocity offsets (0x10..0xCC)
    s32 mCC_state;
    // Saturn size 0xD0
};

// Saturn quad definition source at FLD_A5::0609A25C — loaded once into this
// static vector so particleInitSub can reference it.
static std::vector<sVdp1Quad> s_weatherQuadData;

// Saturn vec3 table at FLD_A5::0608B5FC — null-terminated list of pointers
// to 47 sVec3_FP-equivalent records each 20 bytes apart starting at 0608B250.
static constexpr u32 kWeatherPositionTableAddr = 0x0608B5FC;
static constexpr u32 kWeatherCenterPositionAddr = 0x0608B250;

// 0605BD08
static void a5WeatherTask_Init(sA5WeatherTask* p)
{
    getMemoryArea(&p->m0_memoryArea, 3);
    u16 vdp1Mem = (u16)((p->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);

    if (s_weatherQuadData.empty())
    {
        s_weatherQuadData = initVdp1Quad(gFLD_A5->getSaturnPtr(0x0609A25C));
    }
    particleInitSub(&p->m8_quad, vdp1Mem, &s_weatherQuadData);

    p->mCC_state = 0;
    for (s32 i = 0; i < 0x2F; i++)
    {
        p->m10_velocities[i] = (s32)(randomNumber() & 0x1F) - 0x10;
    }
}

// 0605BDF4
static void a5WeatherTask_Update(sA5WeatherTask* p)
{
    if ((mainGameState.bitField[0x95] & 2) != 0)
        return;

    for (s32 i = 0; i < 0x2F; i++)
    {
        p->mCC_state++;
        if (p->mCC_state > 1)
        {
            p->mCC_state = 0;
            p->m10_velocities[i]++;
            if (p->m10_velocities[i] > 0xF)
            {
                p->m10_velocities[i] = -(s32)(randomNumber() & 0x3F);
            }
        }
    }
}

// 0605BD70
static void a5WeatherTask_Draw(sA5WeatherTask* p)
{
    if ((mainGameState.bitField[0x95] & 2) != 0)
        return;

    // LOD check: transform the entity anchor and read the resulting Z depth.
    pushCurrentMatrix();
    sVec3_FP anchor = readSaturnVec3(gFLD_A5->getSaturnPtr(kWeatherCenterPositionAddr));
    translateCurrentMatrix(&anchor);
    // FLD_A5::06078fa6 is an overlay-local copy of gridCellDraw_GetDepthRange.
    u32 lod = gridCellDraw_GetDepthRange(pCurrentMatrix->m[2][3]);
    popMatrix();

    if (lod >= 2)
        return;

    // Walk the null-terminated Saturn pointer table at 0608B5FC and submit one
    // projected particle per non-empty slot whose matching velocity is non-negative.
    for (s32 i = 0; ; i++)
    {
        u32 ea = readSaturnU32(gFLD_A5->getSaturnPtr(kWeatherPositionTableAddr + i * 4));
        if (ea == 0)
            break;

        if (p->m10_velocities[i] >= 0)
        {
            // Saturn reads the low byte of the velocity as the current frame.
            p->m8_quad.m7_currentFrame = (u8)p->m10_velocities[i];

            sVec3_FP pos = readSaturnVec3(gFLD_A5->getSaturnPtr(ea));
            drawProjectedParticle(&p->m8_quad, &pos);
        }
    }
}

// 0605c006
void createA5_envObjects_sub2_weatherTask(p_workArea parent)
{
    static sA5WeatherTask::TypedTaskDefinition td = { &a5WeatherTask_Init, &a5WeatherTask_Update, &a5WeatherTask_Draw, nullptr };
    createSubTask<sA5WeatherTask>(parent, &td);
}
