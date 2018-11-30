#pragma once

extern sSaturnMemoryFile* gFLD_A3;

//namespace FLD_A3_OVERLAY {
    p_workArea overlayStart_FLD_A3(p_workArea workArea, u32 arg);
    s32 startFieldScript(s32 r4, s32 r5);

    void initDragonSpeed(u32 arg);
    void dragonFieldTaskInitSub4(s_dragonTaskWorkArea* pTypedWorkArea);
    void fieldOverlaySubTaskInitSub1(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*));
    u32 fieldOverlaySubTaskInitSub3(u32 r4);
    p_workArea createLCSShootTask(s_LCSTask* r4, sLCSTarget* r5);
    void startScript_cantDestroy();
    void LCSUpdateCursorFromInputSub0(s32 r4, sVec3_FP* r5);
    s8 LCSTaskDrawSub1Sub6();
    void LCSTaskDrawSub1Sub3();
    s32 fieldScriptTaskUpdateSub4();
    void LCSTaskDrawSub1Sub3();
    void LCSTaskDrawSub1Sub4();
//};
