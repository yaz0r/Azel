#pragma once

struct s_fieldCameraConfig;

enum eCameraFollowMode : u8
{
    eCameraFollowMode_scriptTarget = 0,
    eCameraFollowMode_default = 1,
    eCameraFollowMode_dragonYaw = 2,
    eCameraFollowMode_banked = 3,
    eCameraFollowMode_dragonYawBanked = 4,
    eCameraFollowMode_watchDragon = 5,
    eCameraFollowMode_hold = 6,
    eCameraFollowMode_debugFreeCamera = 7,
    eCameraFollowMode_idle = 8,
};

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
    sVec3_FP m44_positionSpring;
    sVec3_FP m50_positionImpulse;
    sVec3_FP m5C_rotationSpring;
    sVec3_FP m68_rotationImpulse;
    void(*m74_updateFunc)(sFieldCameraStatus* r4);
    void(*m78_drawFunc)(sFieldCameraStatus* r4);
    s32 m7C;
    s32 m80_frameCounter;
    s32 m84;
    s32 m88;
    s8 m89;
    s8 m8A;
    s8 m8C_isActive;
    s8 m8D_reinitMode;
    s8 m8E_followSubState;
    s8 m8F_followType;
    s8 m90_followMode;

    //size 94
};

void cameraFollowMode_default(sFieldCameraStatus* r4);

s8 isFieldCameraSlotActive(s32 slotIndex);
sFieldCameraStatus* getFieldCameraStatus();
void initCameraSlotWithFunctions(u32 slotIndex, void(*updateFunc)(sFieldCameraStatus*), void(*drawFunc)(sFieldCameraStatus*));
u32 selectCameraSlot(u32 slotIndex);
void deactivateCameraSlot(u32 slotIndex);
void updateFieldCameraSlots();
void drawFieldCameraSlots();

void setCameraFollowMode_cut(eCameraFollowMode followMode);
void setCameraFollowMode_blend(eCameraFollowMode followMode);

s_fieldCameraConfig* readCameraConfig(sSaturnPtr EA);
void setupFieldCameraConfigs(s_fieldCameraConfig* pConfigs, u32 count);

