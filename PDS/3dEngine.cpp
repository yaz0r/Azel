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

u16 projectionStack[8];
u16* projectionStackTop = &projectionStack[8];

void resetProjectVectorMaster()
{
    loc_601FA9E = 0x6103;
}

void pushProjectionStack_Master()
{
    u16* r2 = projectionStackTop;
    *(--r2) = loc_601FA9E;
    projectionStack[0] = *r2;

    resetProjectVectorMaster();
}

void pushProjectionStack()
{
    pushProjectionStack_Master();

    //addSlaveCommand(0, 0, 0, MenuEnTaskInitSub1Sub1_Slave);
}

void initProjectionStack_Master()
{
    projectionStackTop = &projectionStack[8];
    resetProjectVectorMaster();
}

void reset3dEngine()
{
    resetCamera(0, 0, 224, 352, 176, 112);

    initProjectionStack_Master();

    //addSlaveCommand(0, 0, 0, initProjectionStack_Slave);
}

void resetProjectVector()
{
    initProjectionStack_Master();

    //addSlaveCommand(0, 0, 0, initProjectionStack_Slave);
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
    r4->field_C[0] = r6[0];
    r4->field_C[1] = r6[1];
    r4->field_C[2] = r6[2];

    sVec3_FP translation;

    r4->field_0[0] = r5->field_0;
    translation[0] = -r5->field_0;
    r4->field_0[1] = r5->field_4;
    translation[1] = -r5->field_4;
    r4->field_0[2] = r5->field_8;
    translation[2] = -r5->field_8;

    resetMatrixStack();

    rotateCurrentMatrixZ(-r4->field_C[0]);
    rotateCurrentMatrixX(-r4->field_C[2]);
    copyMatrix(pCurrentMatrix, &r4->field_88);
    rotateCurrentMatrixY(-r4->field_C[1]);
    translateCurrentMatrix(&translation);
    rotateMatrixY(0x800, &r4->field_88);
    copyMatrix(&r4->field_28[0], &r4->field_28[1]);
    initMatrixToIdentity(&r4->field_28[0]);
    translateMatrix(&r4->field_0, &r4->field_28[0]);
    rotateMatrixY(r4->field_C[1], &r4->field_28[0]);
    rotateMatrixX(r4->field_C[0], &r4->field_28[0]);
    rotateMatrixZ(r4->field_C[2], &r4->field_28[0]);

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
    u32 angle = rotX & 0xFFF;
    fixedPoint sin = getSin(angle);
    fixedPoint cos = getCos(angle);

    sMatrix4x3 tempMatrix;
    for (int i = 0; i < 3; i++)
    {
        tempMatrix.matrix[i * 4 + 0] = (pMatrix->matrix[i * 4 + 0]);
        tempMatrix.matrix[i * 4 + 1] = (pMatrix->matrix[i * 4 + 0] * cos + pMatrix->matrix[i * 4 + 1] * sin) >> 16;
        tempMatrix.matrix[i * 4 + 2] = (-pMatrix->matrix[i * 4 + 0] * sin + pMatrix->matrix[i * 4 + 2] * cos) >> 16;
        tempMatrix.matrix[i * 4 + 3] = (pMatrix->matrix[i * 4 + 3]);
    }
}

void rotateCurrentMatrixX(s32 rotX)
{
    rotateMatrixX(rotX, pCurrentMatrix);
}

void rotateMatrixY(s32 rotY, sMatrix4x3* pMatrix)
{
    u32 angle = rotY & 0xFFF;
    fixedPoint sin = getSin(angle);
    fixedPoint cos = getCos(angle);

    sMatrix4x3 tempMatrix;
    for (int i = 0; i < 3; i++)
    {
        tempMatrix.matrix[i * 4 + 0] = (pMatrix->matrix[i * 4 + 0] * cos - pMatrix->matrix[i * 4 + 2] * sin) >> 16;
        tempMatrix.matrix[i * 4 + 1] = (pMatrix->matrix[i * 4 + 1]);
        tempMatrix.matrix[i * 4 + 2] = (pMatrix->matrix[i * 4 + 0] * sin + pMatrix->matrix[i * 4 + 2] * cos) >> 16;
        tempMatrix.matrix[i * 4 + 3] = (pMatrix->matrix[i * 4 + 3]);
    }
}

void rotateCurrentMatrixY(s32 rotY)
{
    rotateMatrixY(rotY, pCurrentMatrix);
}

void rotateMatrixZ(s32 rotZ, sMatrix4x3* pMatrix)
{
    u32 angle = rotZ & 0xFFF;
    fixedPoint sin = getSin(angle);
    fixedPoint cos = getCos(angle);

    sMatrix4x3 tempMatrix;
    for(int i=0; i<3; i++)
    {
        tempMatrix.matrix[i * 4 + 0] = ( pMatrix->matrix[i * 4 + 0] * cos + pMatrix->matrix[i * 4 + 1] * sin) >> 16;
        tempMatrix.matrix[i * 4 + 1] = (-pMatrix->matrix[i * 4 + 0] * cos + pMatrix->matrix[i * 4 + 1] * sin) >> 16;
        tempMatrix.matrix[i * 4 + 2] = ( pMatrix->matrix[i * 4 + 2]);
        tempMatrix.matrix[i * 4 + 3] = ( pMatrix->matrix[i * 4 + 3]);
    }
}

void rotateCurrentMatrixZ(s32 rotZ)
{
    rotateMatrixZ(rotZ, pCurrentMatrix);
}

void translateMatrix(sVec3_FP* translation, sMatrix4x3* pMatrix)
{
    fixedPoint* r4 = &(*translation)[0];
    fixedPoint* r5 = &pMatrix->matrix[0];

    s64 mac = 0;
    mac += *(r4++) * *(r5++);
    mac += *(r4++) * *(r5++);
    mac += *(r4++) * *(r5++);
    r4 -= 3;
    *(r5) = *(r5) + (mac >> 16);
    r5++;

    mac = 0;
    mac += *(r4++) * *(r5++);
    mac += *(r4++) * *(r5++);
    mac += *(r4++) * *(r5++);
    r4 -= 3;
    *(r5) = *(r5)+(mac >> 16);
    r5++;

    mac = 0;
    mac += *(r4++) * *(r5++);
    mac += *(r4++) * *(r5++);
    mac += *(r4++) * *(r5++);
    r4 -= 3;
    *(r5) = *(r5)+(mac >> 16);
    r5++;
}

void translateCurrentMatrix(sVec3_FP* translation)
{
    translateMatrix(translation, pCurrentMatrix);
}

void pushCurrentMatrix()
{
    sMatrix4x3* pNextMatrix = pCurrentMatrix+1;
    copyMatrix(pCurrentMatrix, pNextMatrix);
    pCurrentMatrix = pNextMatrix;
}

void popMatrix()
{
    pCurrentMatrix--;
}

void multiplyMatrix(sMatrix4x3* matrixA, sMatrix4x3* matrixB)
{
    fixedPoint* r4 = &matrixA->matrix[0];
    fixedPoint* r5 = &matrixB->matrix[0];

    for(int i=0; i<3; i++)
    {
        s64 mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 += 3;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 += 3;
        mac += MUL_FP(*(r4++), *(r5++));
        r5 -= 3;
        r4 -= 8;
        s64 r0 = mac >> 16;

        mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 += 3;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 += 3;
        mac += MUL_FP(*(r4++), *(r5++));
        r5 -= 3;
        r4 -= 8;
        s64 r1 = mac >> 16;

        mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 += 3;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 += 3;
        mac += MUL_FP(*(r4++), *(r5++));
        r5 -= 3;
        r4 -= 8;
        s64 r2 = mac >> 16;

        mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 += 3;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 += 3;
        mac += MUL_FP(*(r4++), *(r5++));
        r4 -= 12;
        s64 r3 = (mac >> 16) + *(r5++);

        *(--r5) = r3;
        *(--r5) = r2;
        *(--r5) = r1;
        *(--r5) = r0;
        r5 += 4;
    }
}

void multiplyCurrentMatrix(sMatrix4x3* arg4)
{
    multiplyMatrix(arg4, pCurrentMatrix);
}

void rotateMatrixYXZ(sVec3_FP* rotationVec, sMatrix4x3* pMatrix)
{
    rotateMatrixY((*rotationVec)[1].getInteger(), pMatrix);
    rotateMatrixX((*rotationVec)[0].getInteger(), pMatrix);
    rotateMatrixZ((*rotationVec)[2].getInteger(), pMatrix);
}

void rotateMatrixZYX(sVec3_FP* rotationVec, sMatrix4x3* pMatrix)
{
    rotateMatrixZ((*rotationVec)[2].getInteger(), pMatrix);
    rotateMatrixY((*rotationVec)[1].getInteger(), pMatrix);
    rotateMatrixX((*rotationVec)[0].getInteger(), pMatrix);
}

