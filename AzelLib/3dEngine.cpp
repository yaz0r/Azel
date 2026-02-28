#include "PDS.h"

std::array<sMatrix4x3,16> matrixStack;
std::array<sMatrix4x3, 16>::iterator matrixStackIter = matrixStack.begin();
sMatrix4x3* pCurrentMatrix = NULL;

s_cameraProperties2 cameraProperties2;
sCameraProperties cameraProperties;

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

    cameraProperties.mC = x0 - centerX;
    cameraProperties.mE = x1 - centerX;
    cameraProperties.m8 = y1 - centerY;
    cameraProperties.mA = y0 - centerY;
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
    resetCamera(0, 0, 352, 224, 176, 112);

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
    matrix->m[0][0] = 0x10000;  matrix->m[0][1] = 0;  matrix->m[0][2] = 0;  matrix->m[0][3] = 0;
    matrix->m[1][0] = 0;  matrix->m[1][1] = 0x10000;  matrix->m[1][2] = 0;  matrix->m[1][3] = 0;
    matrix->m[2][0] = 0;  matrix->m[2][1] = 0;  matrix->m[2][2] = 0x10000;  matrix->m[2][3] = 0;
}

void updateEngineCamera(s_cameraProperties2* r4, const sVec3_FP& r5_position, const sVec3_S16& r6_rotation)
{
    r4->mC_rotation[0] = r6_rotation[0];
    r4->mC_rotation[1] = r6_rotation[1];
    r4->mC_rotation[2] = r6_rotation[2];

    sVec3_FP translation;

    r4->m0_position[0] = r5_position[0];
    translation[0] = -r5_position[0];
    r4->m0_position[1] = r5_position[1];
    translation[1] = -r5_position[1];
    r4->m0_position[2] = r5_position[2];
    translation[2] = -r5_position[2];

    resetMatrixStack();

    rotateCurrentMatrixZ(-r4->mC_rotation[2]);
    rotateCurrentMatrixX(-r4->mC_rotation[0]);
    copyMatrix(pCurrentMatrix, &r4->m88_billboardViewMatrix);
    rotateCurrentMatrixY(-r4->mC_rotation[1]);
    translateCurrentMatrix(&translation);

    rotateMatrixY(0x800, &r4->m88_billboardViewMatrix);
    copyMatrix(&r4->m28[0], &r4->m28[1]);
    initMatrixToIdentity(&r4->m28[0]);
    translateMatrix(&r4->m0_position, &r4->m28[0]);
    rotateMatrixY(r4->mC_rotation[1], &r4->m28[0]);
    rotateMatrixX(r4->mC_rotation[0], &r4->m28[0]);
    rotateMatrixZ(r4->mC_rotation[2], &r4->m28[0]);

    r4->m28[0].m[0][2] = -r4->m28[0].m[0][2];
    r4->m28[0].m[1][2] = -r4->m28[0].m[1][2];
    r4->m28[0].m[2][2] = -r4->m28[0].m[2][2];
}

void copyMatrix(sMatrix4x3* pSrc, sMatrix4x3* pDst)
{
    *pDst = *pSrc;
}

void copyToCurrentMatrix(sMatrix4x3* pSrc)
{
    copyMatrix(pSrc, pCurrentMatrix);
}

void resetMatrixStack()
{
    matrixStackIter = matrixStack.begin();
    pCurrentMatrix = &(*matrixStackIter);

    pCurrentMatrix->m[0][0] = 0x10000;  pCurrentMatrix->m[0][1] = 0;  pCurrentMatrix->m[0][2] = 0;  pCurrentMatrix->m[0][3] = 0;
    pCurrentMatrix->m[1][0] = 0;  pCurrentMatrix->m[1][1] = 0x10000;  pCurrentMatrix->m[1][2] = 0;  pCurrentMatrix->m[1][3] = 0;
    pCurrentMatrix->m[2][0] = 0;  pCurrentMatrix->m[2][1] = 0;  pCurrentMatrix->m[2][2] = -0x10000;  pCurrentMatrix->m[2][3] = 0;
}

void rotateMatrixX(s32 rotX, sMatrix4x3* pMatrix)
{
    u32 angle = rotX & 0xFFF;
    fixedPoint sin = getSin(angle);
    fixedPoint cos = getCos(angle);

    sMatrix4x3 tempMatrix;
    for (int i = 0; i < 3; i++)
    {
        tempMatrix.m[i][0] = pMatrix->m[i][0];
        tempMatrix.m[i][1] = ((s64)pMatrix->m[i][1] * cos + (s64)pMatrix->m[i][2] * sin) >> 16;
        tempMatrix.m[i][2] = ((s64)-pMatrix->m[i][1] * sin + (s64)pMatrix->m[i][2] * cos) >> 16;
        tempMatrix.m[i][3] = pMatrix->m[i][3];
    }

    *pMatrix = tempMatrix;
}

void rotateMatrixShiftedX(fixedPoint rotX, sMatrix4x3* pMatrix)
{
    rotateMatrixX(rotX.getInteger(), pMatrix);
}

void rotateCurrentMatrixShiftedX(fixedPoint rotX)
{
    rotateMatrixShiftedX(rotX, pCurrentMatrix);
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
        tempMatrix.m[i][0] = ((s64)pMatrix->m[i][0] * cos - (s64)pMatrix->m[i][2] * sin) >> 16;
        tempMatrix.m[i][1] = pMatrix->m[i][1];
        tempMatrix.m[i][2] = ((s64)pMatrix->m[i][0] * sin + (s64)pMatrix->m[i][2] * cos) >> 16;
        tempMatrix.m[i][3] = pMatrix->m[i][3];
    }

    *pMatrix = tempMatrix;
}

void rotateMatrixShiftedY(fixedPoint rotY, sMatrix4x3* pMatrix)
{
    rotateMatrixY(rotY.getInteger(), pMatrix);
}

void rotateCurrentMatrixY(s32 rotY)
{
    rotateMatrixY(rotY, pCurrentMatrix);
}

void rotateCurrentMatrixShiftedY(fixedPoint rotY)
{
    rotateMatrixY(rotY.getInteger(), pCurrentMatrix);
}

void rotateMatrixZ(s32 rotZ, sMatrix4x3* pMatrix)
{
    u32 angle = rotZ & 0xFFF;
    fixedPoint sin = getSin(angle);
    fixedPoint cos = getCos(angle);

    sMatrix4x3 tempMatrix;
    for (int i = 0; i < 3; i++)
    {
        tempMatrix.m[i][0] = ((s64)pMatrix->m[i][0] * cos + (s64)pMatrix->m[i][1] * sin) >> 16;
        tempMatrix.m[i][1] = ((s64)-pMatrix->m[i][0] * sin + (s64)pMatrix->m[i][1] * cos) >> 16;
        tempMatrix.m[i][2] = pMatrix->m[i][2];
        tempMatrix.m[i][3] = pMatrix->m[i][3];
    }

    *pMatrix = tempMatrix;
}

void rotateMatrixShiftedZ(fixedPoint rotZ, sMatrix4x3* pMatrix)
{
    rotateMatrixZ(rotZ.getInteger(), pMatrix);
}

void rotateCurrentMatrixZ(s32 rotZ)
{
    rotateMatrixZ(rotZ, pCurrentMatrix);
}

void rotateCurrentMatrixShiftedZ(fixedPoint rotZ)
{
    rotateMatrixShiftedZ(rotZ, pCurrentMatrix);
}

void translateMatrix(const sVec3_FP* translation, sMatrix4x3* pMatrix)
{
    for(int i=0; i<3; i++)
    {
        s64 mac = 0;
        mac += MUL_FP(translation->m0_X, pMatrix->m[i][0]);
        mac += MUL_FP(translation->m4_Y, pMatrix->m[i][1]);
        mac += MUL_FP(translation->m8_Z, pMatrix->m[i][2]);
        pMatrix->m[i][3] += mac >> 16;
    }
}

void translateMatrix(const sVec3_FP& translation, sMatrix4x3* pMatrix)
{
    translateMatrix(&translation, pMatrix);
}

void translateCurrentMatrix(const sVec3_FP* translation)
{
    translateMatrix(translation, pCurrentMatrix);
}

void translateCurrentMatrix(const sVec3_FP& translation)
{
    translateMatrix(&translation, pCurrentMatrix);
}

void pushCurrentMatrix()
{
    matrixStackIter++;
    sMatrix4x3* pNextMatrix = &(*matrixStackIter);
    copyMatrix(pCurrentMatrix, pNextMatrix);
    pCurrentMatrix = pNextMatrix;
}

void popMatrix()
{
    matrixStackIter--;
    pCurrentMatrix = &(*matrixStackIter);;
}

void multiplyMatrix(sMatrix4x3* matrixA, sMatrix4x3* matrixB)
{
    sMatrix4x3 result;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            s64 mac = 0;
            mac += MUL_FP(matrixB->m[i][0], matrixA->m[0][j]);
            mac += MUL_FP(matrixB->m[i][1], matrixA->m[1][j]);
            mac += MUL_FP(matrixB->m[i][2], matrixA->m[2][j]);
            result.m[i][j] = mac >> 16;
        }
        {
            s64 mac = 0;
            mac += MUL_FP(matrixB->m[i][0], matrixA->m[0][3]);
            mac += MUL_FP(matrixB->m[i][1], matrixA->m[1][3]);
            mac += MUL_FP(matrixB->m[i][2], matrixA->m[2][3]);
            result.m[i][3] = (mac >> 16) + matrixB->m[i][3];
        }
    }
    *matrixB = result;
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

void rotateMatrixZYX(const sVec3_FP* rotationVec, sMatrix4x3* pMatrix)
{
    rotateMatrixZ((*rotationVec)[2].getInteger(), pMatrix);
    rotateMatrixY((*rotationVec)[1].getInteger(), pMatrix);
    rotateMatrixX((*rotationVec)[0].getInteger(), pMatrix);
}

void rotateMatrixYXZ(const sVec3_FP* rotationVec, sMatrix4x3* pMatrix)
{
    rotateMatrixY((*rotationVec)[1].getInteger(), pMatrix);
    rotateMatrixX((*rotationVec)[0].getInteger(), pMatrix);
    rotateMatrixZ((*rotationVec)[2].getInteger(), pMatrix);
}

void rotateCurrentMatrixZYX(const sVec3_FP* rotationVec)
{
    rotateMatrixZYX(rotationVec, pCurrentMatrix);
}

void rotateCurrentMatrixZYX(const sVec3_FP& rotationVec)
{
    rotateMatrixZYX(&rotationVec, pCurrentMatrix);
}

void rotateCurrentMatrixYXZ(const sVec3_FP* rotationVec)
{
    rotateMatrixYXZ(rotationVec, pCurrentMatrix);
}

void rotateCurrentMatrixYXZ(const sVec3_FP& rotationVec)
{
    rotateMatrixYXZ(&rotationVec, pCurrentMatrix);
}

void rotateMatrixZYX_s16(const sVec3_S16_12_4& rotationVec, sMatrix4x3* pMatrix)
{
    rotateMatrixZ(rotationVec[2], pMatrix);
    rotateMatrixY(rotationVec[1], pMatrix);
    rotateMatrixX(rotationVec[0], pMatrix);
}

void rotateCurrentMatrixZYX_s16(const sVec3_S16_12_4& rotationVec)
{
    rotateMatrixZYX_s16(rotationVec, pCurrentMatrix);
}

void scaleMatrixRow0(fixedPoint r4, sMatrix4x3* pMatrix)
{
    pMatrix->m[0][0] = MUL_FP(pMatrix->m[0][0], r4) >> 16;
    pMatrix->m[1][0] = MUL_FP(pMatrix->m[1][0], r4) >> 16;
    pMatrix->m[2][0] = MUL_FP(pMatrix->m[2][0], r4) >> 16;
}

void scaleMatrixRow1(fixedPoint r4, sMatrix4x3* pMatrix)
{
    pMatrix->m[0][1] = MUL_FP(pMatrix->m[0][1], r4) >> 16;
    pMatrix->m[1][1] = MUL_FP(pMatrix->m[1][1], r4) >> 16;
    pMatrix->m[2][1] = MUL_FP(pMatrix->m[2][1], r4) >> 16;
}

void scaleMatrixRow2(fixedPoint r4, sMatrix4x3* pMatrix)
{
    pMatrix->m[0][2] = MUL_FP(pMatrix->m[0][2], r4) >> 16;
    pMatrix->m[1][2] = MUL_FP(pMatrix->m[1][2], r4) >> 16;
    pMatrix->m[2][2] = MUL_FP(pMatrix->m[2][2], r4) >> 16;
}

void scaleCurrentMatrixRow0(s32 r4)
{
    scaleMatrixRow0(r4, pCurrentMatrix);
}

void scaleCurrentMatrixRow1(s32 r4)
{
    scaleMatrixRow1(r4, pCurrentMatrix);
}

void scaleCurrentMatrixRow2(s32 r4)
{
    scaleMatrixRow2(r4, pCurrentMatrix);
}

fixedPoint vecDistance(const sVec3_FP& r4, const sVec3_FP& r5)
{
    s32 x = ((s64)(r5[0] - r4[0]) * (r5[0] - r4[0])) >> 16;
    s32 y = ((s64)(r5[1] - r4[1]) * (r5[1] - r4[1])) >> 16;
    s32 z = ((s64)(r5[2] - r4[2]) * (r5[2] - r4[2])) >> 16;

    return sqrt_F(x + y + z);
}

void transformAndAddVec(const sVec3_FP& r4, sVec3_FP& r5, const sMatrix4x3& r6)
{
    s64 mac = 0;
    mac += (s64)r6.m[0][0] * (s64)r4[0].asS32();
    mac += (s64)r6.m[0][1] * (s64)r4[1].asS32();
    mac += (s64)r6.m[0][2] * (s64)r4[2].asS32();
    r5[0] = (mac >> 16) + r6.m[0][3];

    mac = 0;
    mac += (s64)r6.m[1][0] * (s64)r4[0].asS32();
    mac += (s64)r6.m[1][1] * (s64)r4[1].asS32();
    mac += (s64)r6.m[1][2] * (s64)r4[2].asS32();
    r5[1] = (mac >> 16) + r6.m[1][3];

    mac = 0;
    mac += (s64)r6.m[2][0] * (s64)r4[0].asS32();
    mac += (s64)r6.m[2][1] * (s64)r4[1].asS32();
    mac += (s64)r6.m[2][2] * (s64)r4[2].asS32();
    r5[2] = (mac >> 16) + r6.m[2][3];
}

void transformAndAddVecByCurrentMatrix(const sVec3_FP* r4, sVec3_FP* r5)
{
    transformAndAddVec(*r4, *r5, *pCurrentMatrix);
}

void dragonFieldTaskDrawSub1Sub0()
{
    PDS_unimplemented("dragonFieldTaskDrawSub1Sub0");
}

void dragonFieldTaskDrawSub1Sub1(fixedPoint r4, fixedPoint r5, fixedPoint r6)
{
    PDS_unimplemented("dragonFieldTaskDrawSub1Sub1");
}

fixedPoint transformByMatrixRow0(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->m[0][0] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->m[0][1] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->m[0][2] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->m[0][3];
}

fixedPoint transformByMatrixRow1(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->m[1][0] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->m[1][1] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->m[1][2] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->m[1][3];
}

fixedPoint transformByMatrixRow2(const sVec3_FP& r4)
{
    s64 mac = 0;
    mac += (s64)pCurrentMatrix->m[2][0] * (s64)r4[0].asS32();
    mac += (s64)pCurrentMatrix->m[2][1] * (s64)r4[1].asS32();
    mac += (s64)pCurrentMatrix->m[2][2] * (s64)r4[2].asS32();
    return (mac >> 16) + pCurrentMatrix->m[2][3];
}

fixedPoint MulVec2(const sVec2_FP& r4, const sVec2_FP& r5)
{
    s64 mac = 0;
    mac += (s64)r4.m_value[0] * (s64)r5.m_value[0];
    mac += (s64)r4.m_value[1] * (s64)r5.m_value[1];

    return fixedPoint(mac >> 16);
}


