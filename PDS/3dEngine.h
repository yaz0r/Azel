#pragma once

void reset3dEngine();
void resetProjectVector();
void pushProjectionStack();

struct s_cameraProperties2
{
    sVec3_FP m0_position; //0
    s16 m_rotation[3];
    sVec3_FP m14; //14
    s16 m20[3]; // 20
    sMatrix4x3 m28[2]; // 28

    sMatrix4x3 m88; // 88
};
extern s_cameraProperties2 cameraProperties2;

struct sFieldCameraStatus
{
    sVec3_FP m0_position;
    sVec3_FP mC_rotation;
    fixedPoint m18;
    fixedPoint m1C;
    fixedPoint m20;
    s32 m24_distanceToDestination;
    fixedPoint m28;
    s32 m2C;
    s32 m30;
    fixedPoint m34;
    fixedPoint m40;
    sVec3_FP m44;
    sVec3_FP m50;
    sVec3_FP m5C;
    sVec3_FP m68;
    void(*m74)(sFieldCameraStatus* r4);
    void(*m78)(sFieldCameraStatus* r4);
    s32 m7C;
    s32 m80;
    s32 m84;
    s32 m88;
    s8 m89;
    s8 m8A;
    s8 m8C;
    s8 m8D;
    s8 m8E;
    s8 m8F;
    s8 m90;

    //size 94
};

extern sMatrix4x3* pCurrentMatrix;

void addObjectToDrawList(u8* pObjectData, u32 offset);
void addBillBoardtToDrawList(u8* pObjectData, u32 offset);

void updateEngineCamera(s_cameraProperties2* r4, sFieldCameraStatus* r5, s16* r6);

void multiplyMatrix(sMatrix4x3* matrixA, sMatrix4x3* matrixB);

void copyMatrix(sMatrix4x3* pSrc, sMatrix4x3* pDst);
void initMatrixToIdentity(sMatrix4x3* matrix);
void resetMatrixStack();
void pushCurrentMatrix();
void popMatrix();

void rotateMatrixX(s32 rotX, sMatrix4x3* pMatrix);
void rotateMatrixShiftedX(fixedPoint rotX, sMatrix4x3* pMatrix);
void rotateCurrentMatrixX(s32 rotX);
void rotateMatrixY(s32 rotY, sMatrix4x3* pMatrix);
void rotateMatrixShiftedY(fixedPoint rotY, sMatrix4x3* pMatrix);
void rotateCurrentMatrixY(s32 rotY);
void rotateCurrentMatrixShiftedY(fixedPoint rotY);
void rotateMatrixZ(s32 rotZ, sMatrix4x3* pMatrix);
void rotateMatrixShiftedZ(fixedPoint rotZ, sMatrix4x3* pMatrix);
void rotateCurrentMatrixZ(s32 rotZ);
void translateMatrix(sVec3_FP* translation, sMatrix4x3* pMatrix);
void translateCurrentMatrix(sVec3_FP* translation);
void rotateMatrixYXZ(sVec3_FP* rotationVec, sMatrix4x3* pMatrix);
void rotateMatrixZYX(sVec3_FP* rotationVec, sMatrix4x3* pMatrix);
void rotateCurrentMatrixZYX(sVec3_FP* rotationVec);
void rotateMatrixZYX_s16(s16* rotationVec, sMatrix4x3* pMatrix);
void multiplyCurrentMatrix(sMatrix4x3* arg4);
void multiplyCurrentMatrixSaveStack(sMatrix4x3* arg4);

void scaleCurrentMatrixRow0(s32 r4);
void scaleCurrentMatrixRow1(s32 r4);
void scaleCurrentMatrixRow2(s32 r4);

fixedPoint vecDistance(const sVec3_FP& r4, const sVec3_FP& r5);

void transformAndAddVec(sVec3_FP& r4, sVec3_FP& r5, sMatrix4x3& r6);
void transformAndAddVecByCurrentMatrix(sVec3_FP* r4, sVec3_FP* r5);

