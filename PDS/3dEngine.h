#pragma once

void reset3dEngine();
void resetProjectVector();

struct s_cameraProperties2
{
    sVec3 field_0; //0
    s16 field_C;
    s16 field_E;
    s16 field_10;

    sMatrix4x3 field_28[2]; // 28

    sMatrix4x3 field_88; // 88
};
extern s_cameraProperties2 cameraProperties2;

struct sFieldCameraStatus
{
    s32 field_0;
    s32 field_4;
    s32 field_8;
    s32 angle_y; // 0xC
    s32 field_10; // 0x10
    s32 field_14; // 0x14
};

extern sMatrix4x3* pCurrentMatrix;
extern sMatrix4x3 unkMatrix;

void fieldOverlaySubTaskInitSub6Sub1(s_cameraProperties2* r4, sFieldCameraStatus* r5, s16* r6);

void copyMatrix(sMatrix4x3* pSrc, sMatrix4x3* pDst);
void initMatrixToIdentity(sMatrix4x3* matrix);
void resetMatrixStack();
void rotateMatrixX(s32 rotX, sMatrix4x3* pMatrix);
void rotateCurrentMatrixX(s32 rotX);
void rotateMatrixY(s32 rotY, sMatrix4x3* pMatrix);
void rotateCurrentMatrixY(s32 rotY);
void rotateMatrixZ(s32 rotZ, sMatrix4x3* pMatrix);
void rotateCurrentMatrixZ(s32 rotZ);
void translateMatrix(sVec3* translation, sMatrix4x3* pMatrix);
void translateCurrentMatrix(sVec3* translation);