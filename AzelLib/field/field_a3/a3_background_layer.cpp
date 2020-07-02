#include "PDS.h"
#include "a3_background_layer.h"

void drawBackgroundSprite(const sVec2_S32& r4_position, fixedPoint r5_distance, u16 r6_sprite, s32 r7, s16 arg0, s16 arg1)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0, normal sprite + JUMP
    vdp1WriteEA.m4_CMDPMOD = 0x480 | arg1; // CMDPMOD
    vdp1WriteEA.m8_CMDSRCA = r6_sprite; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = r7; // CMDSIZE
    vdp1WriteEA.m6_CMDCOLR = arg0; // CMDCOLR
    vdp1WriteEA.mC_CMDXA = r4_position[0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -r4_position[1]; // CMDYA

    s_vd1ExtendedCommand* pExtendedCommand = createVdp1ExtendedCommand(vdp1WriteEA);
    pExtendedCommand->depth = (float)r5_distance.asS32() / (float)graphicEngineStatus.m405C.m14_farClipDistance.asS32();

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = fixedPoint(r5_distance * graphicEngineStatus.m405C.m38).getInteger();
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;

}

struct s_A3_BackgroundLayer : public s_workAreaTemplate<s_A3_BackgroundLayer>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_A3_BackgroundLayer::Init, &s_A3_BackgroundLayer::Update, &s_A3_BackgroundLayer::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(s_A3_BackgroundLayer* pThis)
    {
        getMemoryArea(&pThis->m0, 1);
        pThis->m18 = (pThis->m0.m4_characterArea - (0x25C00000)) >> 3;
        pThis->m1A = 0x228 + ((pThis->m0.m4_characterArea - (0x25C00000)) >> 3);
        pThis->m10 = -4;
        pThis->m14 = 5;
    }

    static void Update(s_A3_BackgroundLayer* pThis)
    {
        sVec3_FP var8 = cameraProperties2.m0_position;
        fixedPoint r4;
        if (var8[1] <= 0)
        {
            r4 = 0;
        }
        else
        {
            r4 = var8[1] >> 2;
        }

        s32 r2 = atan2(r4, 0x80000);
        s16 var0 = cameraProperties2.mC_rotation[0] + r2;
        s32 var4 = var0 & 0xFFF;
        pThis->mC = pThis->m10 - MTH_Mul(graphicEngineStatus.m405C.m1C_heightScale ,FP_Div(getSin(var4), getCos(var4)));
        pThis->m8 = -80 + performModulo(80, MTH_Mul(graphicEngineStatus.m405C.m18_widthScale, MTH_Mul((s32)cameraProperties2.mC_rotation[1], 0x3243F) << 5));
    }

    static void Draw(s_A3_BackgroundLayer* pThis)
    {
        s32 r12 = graphicEngineStatus.m405C.VDP1_X2 / 2;
        sVec2_S32 current;
        current[0] = pThis->m8 - r12;
        current[1] = pThis->mC + 55;

        while (current[0] < r12)
        {
            drawBackgroundSprite(current, graphicEngineStatus.m405C.m14_farClipDistance - 1, pThis->m18, 0xA37, 0x200, 0xA0);

            current[0] += 80;
        }
    }

    static void Draw2(s_A3_BackgroundLayer* pThis)
    {
        s32 r12 = graphicEngineStatus.m405C.VDP1_X2 / 2;
        sVec2_S32 var8_current;
        var8_current[0] = pThis->m8 - r12;
        var8_current[1] = pThis->mC + 55;

        while (var8_current[0] < r12)
        {
            drawBackgroundSprite(var8_current, graphicEngineStatus.m405C.m14_farClipDistance - 1, pThis->m18, 0xA37, 0x200, 0xA0);

            var8_current[0] += 80;
        }

        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos[2] < -0x1068000)
        {
            sVec2_S32 var4_current;
            var4_current[0] = pThis->m8 - r12;
            var4_current[1] = pThis->mC + pThis->m14;

            while (var4_current[0] < r12)
            {
                drawBackgroundSprite(var4_current, graphicEngineStatus.m405C.m14_farClipDistance - 1, pThis->m1A, 0xA28, 0x2070, 0x80);

                var4_current[0] += 80;
            }
        }
    }

    s_memoryAreaOutput m0;
    s32 m8;
    s32 mC;
    s32 m10;
    s32 m14;
    u16 m18;
    u16 m1A;
    //size 1C
};

void create_fieldA3_backgroundLayer(p_workArea workArea)
{
    createSubTask<s_A3_BackgroundLayer>(workArea);
}

void create_fieldA3_backgroundLayer2(p_workArea workArea)
{
    static const s_A3_BackgroundLayer::TypedTaskDefinition taskDefinition = { &s_A3_BackgroundLayer::Init, &s_A3_BackgroundLayer::Update, &s_A3_BackgroundLayer::Draw2, NULL };
    createSubTask<s_A3_BackgroundLayer>(workArea, &taskDefinition);
}
