#include "PDS.h"
#include "a3_background_layer.h"

void drawBackgroundSprite(const sVec2_S32& r4_position, fixedPoint r5_distance, u16 r6_sprite, s32 r7, s16 arg0, s16 arg1)
{
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0, normal sprite + JUMP
    setVdp1VramU16(vdp1WriteEA + 0x04, 0x480 | arg1); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x08, r6_sprite); // CMDSRCA
    setVdp1VramU16(vdp1WriteEA + 0x0A, r7); // CMDSIZE
    setVdp1VramU16(vdp1WriteEA + 0x06, arg0); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x0C, r4_position[0]); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, -r4_position[1]); // CMDYA

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = fixedPoint(r5_distance * graphicEngineStatus.m405C.m38).getInteger();
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
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
        pThis->m18 = (pThis->m0.m4_characterArea - getVdp1Pointer(0x25C00000)) >> 3;
        pThis->m1A = 0x228 + ((pThis->m0.m4_characterArea - getVdp1Pointer(0x25C00000)) >> 3);
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
        pThis->mC = pThis->m10 - MTH_Mul(graphicEngineStatus.m405C.m1C ,FP_Div(getSin(var4), getCos(var4)));
        pThis->m8 = -80 + performModulo(80, MTH_Mul(graphicEngineStatus.m405C.m18, MTH_Mul(cameraProperties2.mC_rotation[2], 0x3243F) << 5));
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
