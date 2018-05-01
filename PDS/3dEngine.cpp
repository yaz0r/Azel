#include "PDS.h"

sMatrix4x3 matrixStack[16];
sMatrix4x3* pCurrentMatrix = NULL;

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

void updateEngineCamera(s_cameraProperties2* r4, sFieldCameraStatus* r5, s16* r6)
{
    r4->m_rotation[0] = r6[0];
    r4->m_rotation[1] = r6[1];
    r4->m_rotation[2] = r6[2];

    sVec3_FP translation;

    r4->m0_position[0] = r5->m0_position[0];
    translation[0] = -r5->m0_position[0];
    r4->m0_position[1] = r5->m0_position[1];
    translation[1] = -r5->m0_position[1];
    r4->m0_position[2] = r5->m0_position[2];
    translation[2] = -r5->m0_position[2];

    resetMatrixStack();

    rotateCurrentMatrixZ(-r4->m_rotation[0]);
    rotateCurrentMatrixX(-r4->m_rotation[2]);
    copyMatrix(pCurrentMatrix, &r4->field_88);
    rotateCurrentMatrixY(-r4->m_rotation[1]);
    translateCurrentMatrix(&translation);
    rotateMatrixY(0x800, &r4->field_88);
    copyMatrix(&r4->m28[0], &r4->m28[1]);
    initMatrixToIdentity(&r4->m28[0]);
    translateMatrix(&r4->m0_position, &r4->m28[0]);
    rotateMatrixY(r4->m_rotation[1], &r4->m28[0]);
    rotateMatrixX(r4->m_rotation[0], &r4->m28[0]);
    rotateMatrixZ(r4->m_rotation[2], &r4->m28[0]);

    r4->m28[0].matrix[2] = -r4->m28[0].matrix[2];
    r4->m28[0].matrix[6] = -r4->m28[0].matrix[6];
    r4->m28[0].matrix[10] = -r4->m28[0].matrix[10];
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

    if(0)
    {
        fixedPoint stack[4];
        stack[0] = sin;
        stack[1] = cos;
        stack[2] = cos;
        stack[3] = -sin;

        fixedPoint* r4 = stack;
        fixedPoint* r5 = pMatrix->matrix;

        s64 mac = 0;
        r5++;
        mac += MUL_FP(*(r4++), *(r5++));
        mac += MUL_FP(*(r4++), *(r5++));
        r5 -= 2;
        s64 r1 = mac >> 16;
        mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        mac += MUL_FP(*(r4++), *(r5++));
        *(--r5) = r1;
        *(--r5) = mac >> 16;

        r4 = stack;
        r5 += 0x10 / 4;
        mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        mac += MUL_FP(*(r4++), *(r5++));
        r5 -= 2;
        r1 = mac >> 16;
        mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        mac += MUL_FP(*(r4++), *(r5++));
        *(--r5) = r1;
        *(--r5) = mac >> 16;

        r4 = stack;
        r5 += 0x10 / 4;
        mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        mac += MUL_FP(*(r4++), *(r5++));
        r5 -= 2;
        r1 = mac >> 16;
        mac = 0;
        mac += MUL_FP(*(r4++), *(r5++));
        mac += MUL_FP(*(r4++), *(r5++));
        *(--r5) = r1;
        *(--r5) = mac >> 16;

    }
    else
    {
        sMatrix4x3 tempMatrix;
        for (int i = 0; i < 3; i++)
        {
            tempMatrix.matrix[i * 4 + 0] = (pMatrix->matrix[i * 4 + 0]);
            tempMatrix.matrix[i * 4 + 1] = ((s64)pMatrix->matrix[i * 4 + 1] * cos + (s64)pMatrix->matrix[i * 4 + 2] * sin) >> 16;
            tempMatrix.matrix[i * 4 + 2] = ((s64)-pMatrix->matrix[i * 4 + 1] * sin + (s64)pMatrix->matrix[i * 4 + 2] * cos) >> 16;
            tempMatrix.matrix[i * 4 + 3] = (pMatrix->matrix[i * 4 + 3]);
        }

        *pMatrix = tempMatrix;
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

    if (0)
    {
        fixedPoint stack[4];
        stack[0] = -sin;
        stack[1] = cos;
        stack[2] = cos;
        stack[3] = sin;

        s64 mac = 0;
        s64 r1 = 0;
        fixedPoint* r4 = stack;
        fixedPoint* r5 = pMatrix->matrix;

        for (int i = 0; i < 3; i++)
        {
            mac = 0;
            mac += MUL_FP(*(r4++), *(r5++));
            r5++;
            mac += MUL_FP(*(r4++), *(r5++));
            r5 -= 3;
            r1 = mac >> 16;
            mac = 0;
            mac += MUL_FP(*(r4++), *(r5++));
            r5++;
            mac += MUL_FP(*(r4++), *(r5++));
            *(--r5) = r1;
            r5 -= 1;
            *(--r5) = mac >> 16;
            r4 = stack;
            r5 += 0x10 / 4;
        }
    }
    else
    {
        sMatrix4x3 tempMatrix;
        for (int i = 0; i < 3; i++)
        {
            tempMatrix.matrix[i * 4 + 0] = ((s64)pMatrix->matrix[i * 4 + 0] * cos - (s64)pMatrix->matrix[i * 4 + 2] * sin) >> 16;
            tempMatrix.matrix[i * 4 + 1] = (pMatrix->matrix[i * 4 + 1]);
            tempMatrix.matrix[i * 4 + 2] = ((s64)pMatrix->matrix[i * 4 + 0] * sin + (s64)pMatrix->matrix[i * 4 + 2] * cos) >> 16;
            tempMatrix.matrix[i * 4 + 3] = (pMatrix->matrix[i * 4 + 3]);
        }

        *pMatrix = tempMatrix;
    }
}

void rotateCurrentMatrixY(s32 rotY)
{
    rotateMatrixY(rotY, pCurrentMatrix);
}

void rotateCurrentMatrixShiftedY(s32 rotY)
{
    rotateMatrixY(rotY >> 16, pCurrentMatrix);
}

void rotateMatrixZ(s32 rotZ, sMatrix4x3* pMatrix)
{
    u32 angle = rotZ & 0xFFF;
    fixedPoint sin = getSin(angle);
    fixedPoint cos = getCos(angle);

    if (0)
    {
        fixedPoint stack[4];
        stack[0] = sin;
        stack[1] = cos;
        stack[2] = cos;
        stack[3] = -sin;

        s64 mac = 0;
        s64 r1 = 0;
        fixedPoint* r4 = stack;
        fixedPoint* r5 = pMatrix->matrix;

        for (int i = 0; i < 3; i++)
        {
            mac = MUL_FP(*(r4++), *(r5++));
            mac += MUL_FP(*(r4++), *(r5++));
            r5 -= 2;
            r1 = mac >> 16;

            mac = MUL_FP(*(r4++), *(r5++));
            mac += MUL_FP(*(r4++), *(r5++));

            *(--r5) = r1;
            *(--r5) = mac >> 16;
            r4 = stack;
            r5 += 0x10 / 4;
        }
    }
    else
    {
        sMatrix4x3 tempMatrix;
        for (int i = 0; i < 3; i++)
        {
            tempMatrix.matrix[i * 4 + 0] = ((s64)pMatrix->matrix[i * 4 + 0] * cos + (s64)pMatrix->matrix[i * 4 + 1] * sin) >> 16;
            tempMatrix.matrix[i * 4 + 1] = ((s64)-pMatrix->matrix[i * 4 + 0] * sin + (s64)pMatrix->matrix[i * 4 + 1] * cos) >> 16;
            tempMatrix.matrix[i * 4 + 2] = (pMatrix->matrix[i * 4 + 2]);
            tempMatrix.matrix[i * 4 + 3] = (pMatrix->matrix[i * 4 + 3]);
        }

        *pMatrix = tempMatrix;
    }
}

void rotateCurrentMatrixZ(s32 rotZ)
{
    rotateMatrixZ(rotZ, pCurrentMatrix);
}

void translateMatrix(sVec3_FP* translation, sMatrix4x3* pMatrix)
{
    fixedPoint* r5 = pMatrix->matrix;

    for(int i=0; i<3; i++)
    {
        s64 mac = 0;
        mac += MUL_FP(translation->m_value[0], r5[0]);
        mac += MUL_FP(translation->m_value[1], r5[1]);
        mac += MUL_FP(translation->m_value[2], r5[2]);
        r5[3] += mac >> 16;
        r5 += 4;
    }
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

void multiplyCurrentMatrixSaveStack(sMatrix4x3* arg4)
{
    multiplyCurrentMatrix(arg4);
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

void scaleCurrentMatrixRow0(s32 r4)
{
    unimplemented("scaleCurrentMatrixRow0");
}

void scaleCurrentMatrixRow1(s32 r4)
{
    unimplemented("scaleCurrentMatrixRow1");
}

void scaleCurrentMatrixRow2(s32 r4)
{
    unimplemented("scaleCurrentMatrixRow2");
}


