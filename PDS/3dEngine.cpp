#include "PDS.h"

sMatrix4x3 matrixStack[16];
sMatrix4x3* pCurrentMatrix = NULL;
sMatrix4x3 unkMatrix;

s_cameraProperties2 cameraProperties2;

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

void initMatrixToIdentity(sMatrix4x3* matrix)
{
    matrix->matrix[0] = 0x10000;
    matrix->matrix[1] = 0;
    matrix->matrix[2] = 0;
    matrix->matrix[3] = 0;

    matrix->matrix[4] = 0;
    matrix->matrix[5] = 0x10000;
    matrix->matrix[6] = 0;
    matrix->matrix[7] = 0;

    matrix->matrix[8] = 0;
    matrix->matrix[9] = 0;
    matrix->matrix[10] = 0x10000;
    matrix->matrix[11] = 0;
}

void fieldOverlaySubTaskInitSub6Sub1(s_cameraProperties2* r4, sFieldCameraStatus* r5, s16* r6)
{
    r4->field_C = r6[0];
    r4->field_E = r6[1];
    r4->field_10 = r6[2];

    sVec3 translation;

    r4->field_0[0] = r5->field_0;
    translation[0] = -r5->field_0;
    r4->field_0[1] = r5->field_4;
    translation[1] = -r5->field_4;
    r4->field_0[2] = r5->field_8;
    translation[2] = -r5->field_8;

    resetMatrixStack();

    rotateCurrentMatrixZ(-r4->field_10);
    rotateCurrentMatrixX(-r4->field_C);
    copyMatrix(pCurrentMatrix, &r4->field_88);
    rotateCurrentMatrixY(-r4->field_E);
    translateCurrentMatrix(&translation);
    rotateMatrixY(0x800, &r4->field_88);
    copyMatrix(&r4->field_28[0], &r4->field_28[1]);
    initMatrixToIdentity(&r4->field_28[0]);
    translateMatrix(&r4->field_0, &r4->field_28[0]);
    rotateMatrixY(r4->field_E, &r4->field_28[0]);
    rotateMatrixX(r4->field_C, &r4->field_28[0]);
    rotateMatrixZ(r4->field_10, &r4->field_28[0]);

    r4->field_28[0].matrix[2] = -r4->field_28[0].matrix[2];
    r4->field_28[0].matrix[6] = -r4->field_28[0].matrix[6];
    r4->field_28[0].matrix[10] = -r4->field_28[0].matrix[10];
}

void copyMatrix(sMatrix4x3* pSrc, sMatrix4x3* pDst)
{
    for(int i=0; i<12; i++)
        pDst->matrix[i] = pSrc->matrix[i];
}

void resetMatrixStack()
{
    pCurrentMatrix = &matrixStack[0];

    pCurrentMatrix->matrix[0] = 0x10000;
    pCurrentMatrix->matrix[1] = 0;
    pCurrentMatrix->matrix[2] = 0;
    pCurrentMatrix->matrix[3] = 0;

    pCurrentMatrix->matrix[4] = 0;
    pCurrentMatrix->matrix[5] = 0x10000;
    pCurrentMatrix->matrix[6] = 0;
    pCurrentMatrix->matrix[7] = 0;

    pCurrentMatrix->matrix[8] = 0;
    pCurrentMatrix->matrix[9] = 0;
    pCurrentMatrix->matrix[10] = -0x10000;
    pCurrentMatrix->matrix[11] = 0;
}

void rotateMatrixX(s32 rotX, sMatrix4x3* pMatrix)
{
    unimplemented("rotateMatrixX");
}

void rotateCurrentMatrixX(s32 rotX)
{
    rotateMatrixX(rotX, pCurrentMatrix);
}

void rotateMatrixY(s32 rorY, sMatrix4x3* pMatrix)
{
    unimplemented("rotateMatrixY");
}

void rotateCurrentMatrixY(s32 rotY)
{
    rotateMatrixY(rotY, pCurrentMatrix);
}

void rotateMatrixZ(s32 rotZ, sMatrix4x3* pMatrix)
{
    unimplemented("rotateMatrixZ");
}

void rotateCurrentMatrixZ(s32 rotZ)
{
    rotateMatrixZ(rotZ, pCurrentMatrix);
}

void translateMatrix(sVec3* translation, sMatrix4x3* pMatrix)
{
    unimplemented("translateMatrix");
}

void translateCurrentMatrix(sVec3* translation)
{
    translateMatrix(translation, pCurrentMatrix);
}

