#pragma once

void resetCamera(u32 x0, u32 y0, u32 x1, u32 y1, u32 centerX, u32 centerY);

void reset3dEngine();
void resetProjectVector();
void pushProjectionStack();

void setupLight(fixedPoint r4, fixedPoint r5, fixedPoint r6, u32 r7);
void generateLightFalloffMap(u32 r4, u32 r5, u32 r6);

void dragonFieldTaskDrawSub1Sub0();
void dragonFieldTaskDrawSub1Sub1(fixedPoint r4, fixedPoint r5, fixedPoint r6);

struct s_cameraProperties2
{
    sVec3_FP m0_position; //0
    sVec3_S16 mC_rotation;
    sVec3_FP m14_previousPosition; //14
    s16 m20_previousRotation[3]; // 20
    sMatrix4x3 m28[2]; // 28

    sMatrix4x3 m88_billboardViewMatrix; // 88
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

void addObjectToDrawList(struct sProcessed3dModel* pObjectData);
void addBillBoardToDrawList(struct sProcessed3dModel* pObjectData);

void updateEngineCamera(s_cameraProperties2* r4, const sVec3_FP& r5_position, const sVec3_S16& r6_rotation);

void multiplyMatrix(sMatrix4x3* matrixA, sMatrix4x3* matrixB);

void copyMatrix(sMatrix4x3* pSrc, sMatrix4x3* pDst);
void copyToCurrentMatrix(sMatrix4x3* pSrc);
void initMatrixToIdentity(sMatrix4x3* matrix);
void resetMatrixStack();
void pushCurrentMatrix();
void popMatrix();

void rotateMatrixX(s32 rotX, sMatrix4x3* pMatrix);
void rotateMatrixShiftedX(fixedPoint rotX, sMatrix4x3* pMatrix);
void rotateCurrentMatrixShiftedX(fixedPoint rotX);
void rotateCurrentMatrixX(s32 rotX);
void rotateMatrixY(s32 rotY, sMatrix4x3* pMatrix);
void rotateMatrixShiftedY(fixedPoint rotY, sMatrix4x3* pMatrix);
void rotateCurrentMatrixY(s32 rotY);
void rotateCurrentMatrixShiftedY(fixedPoint rotY);
void rotateMatrixZ(s32 rotZ, sMatrix4x3* pMatrix);
void rotateMatrixShiftedZ(fixedPoint rotZ, sMatrix4x3* pMatrix);
void rotateCurrentMatrixZ(s32 rotZ);
void rotateCurrentMatrixShiftedZ(fixedPoint rotZ);
void translateMatrix(const sVec3_FP& translation, sMatrix4x3* pMatrix);
void translateMatrix(const sVec3_FP* translation, sMatrix4x3* pMatrix);
void translateCurrentMatrix(const sVec3_FP* translation);
void translateCurrentMatrix(const sVec3_FP& translation);
void rotateMatrixYXZ(sVec3_FP* rotationVec, sMatrix4x3* pMatrix);
void rotateMatrixZYX(const sVec3_FP* rotationVec, sMatrix4x3* pMatrix);
void rotateCurrentMatrixZYX(const sVec3_FP* rotationVec);
void rotateCurrentMatrixZYX(const sVec3_FP& rotationVec);
void rotateCurrentMatrixYXZ(const sVec3_FP* rotationVec);
void rotateCurrentMatrixYXZ(const sVec3_FP& rotationVec);
void rotateMatrixZYX_s16(const sVec3_S16_12_4& rotationVec, sMatrix4x3* pMatrix);
void rotateCurrentMatrixZYX_s16(const sVec3_S16_12_4& rotationVec);
void multiplyCurrentMatrix(sMatrix4x3* arg4);
void multiplyCurrentMatrixSaveStack(sMatrix4x3* arg4);

void scaleMatrixRow2(fixedPoint r4, sMatrix4x3* pMatrix);

void scaleCurrentMatrixRow0(s32 r4);
void scaleCurrentMatrixRow1(s32 r4);
void scaleCurrentMatrixRow2(s32 r4);

fixedPoint vecDistance(const sVec3_FP& r4, const sVec3_FP& r5);

void transformAndAddVec(const sVec3_FP& r4, sVec3_FP& r5, const sMatrix4x3& r6);
void transformAndAddVecByCurrentMatrix(const sVec3_FP* r4, sVec3_FP* r5);
void transformVecByCurrentMatrix(const sVec3_FP& r4, sVec3_FP& r5);

fixedPoint transformByMatrixRow0(const sVec3_FP& r4);
fixedPoint transformByMatrixRow1(const sVec3_FP& r4);
fixedPoint transformByMatrixRow2(const sVec3_FP& r4);

fixedPoint MulVec2(const sVec2_FP& r4, const sVec2_FP& r5);
