#pragma once

//namespace FLD_A3_OVERLAY {
    p_workArea overlayStart_FLD_A3(p_workArea workArea, u32 arg);
    s32 startFieldScript(s32 r4, s32 r5);

    void initDragonSpeed(u32 arg);
    void dragonFieldTaskInitSub4(s_dragonTaskWorkArea* pTypedWorkArea);
    void fieldOverlaySubTaskInitSub1(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*));
    u32 fieldOverlaySubTaskInitSub3(u32 r4);
//};
