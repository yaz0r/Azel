#include "PDS.h"

struct sCameraProperties
{
    s16 field_8; //8
    s16 field_A; //A
    s16 field_C; //C
    s16 field_E; //E

    s16 x0; // 3C
    s16 y0; // 3E
    s16 x1; // 40
    s16 y1; // 42
    s16 centerX; // 44
    s16 centerY; // 46
} cameraProperties;

void resetCamera(u32 x0, u32 y0, u32 x1, u32 y1, u32 centerX, u32 centerY)
{
    if (x0 > x1)
    {
        u32 temp = x1;
        x1 = x0;
        x0 = temp;
    }

    if (y0 > y1)
    {
        y0 = y1;
    }

    cameraProperties.x0 = x0;
    cameraProperties.y0 = y0;
    cameraProperties.x1 = x1;
    cameraProperties.y1 = y1;
    cameraProperties.centerX = centerX;
    cameraProperties.centerY = centerY;

    cameraProperties.field_C = x0 - centerX;
    cameraProperties.field_E = x1 - centerX;
    cameraProperties.field_8 = y1 - centerY;
    cameraProperties.field_A = y0 - centerY;
}

u16 loc_601FA9E;

u32* unk_601FE64[5];

void resetProjectVectorMaster()
{
    loc_601FA9E = 0x6103;
}

void j_resetProjectVectorMaster()
{
    unk_601FE64[0] = (u32*)&unk_601FE64[5];
    resetProjectVectorMaster();
}

void reset3dEngine()
{
    resetCamera(0, 0, 224, 352, 176, 112);

    j_resetProjectVectorMaster();

    //addSlaveCommand(0, 0, 0, j_resetProjectVectorSlave);
}

void resetProjectVector()
{
    j_resetProjectVectorMaster();

    //addSlaveCommand(0, 0, 0, j_resetProjectVectorSlave);
}
