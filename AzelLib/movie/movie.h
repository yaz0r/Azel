#pragma once

p_workArea overlayStart_MOVIE(p_workArea pWorkArea, s32 movieIndex);
p_workArea startTitleScreenVideo(p_workArea pWorkArea);
p_workArea loadMovieOverlay(p_workArea pWorkArea, s32 movieIndex);
void setupMovieVdp2();
void closeMovieStream();
u32 lastUpdateFunction();

struct s_movieMainWorkArea : public s_workAreaTemplateWithArg<s_movieMainWorkArea, s32>
{
    static TypedTaskDefinition* getTypedTaskDefinition();

    u8         m0_state;
    u8         m1_debugMode;
    u8         m2_cpkIndex;
    u8         m3_countdown;
    u8         m4_savedVblank;
    u8         m5;
    s16        m6_lastPitch;
    u16        m8_fadePalTarget;
    s16        mA_timer;
    p_workArea mC_subtask;
    void* m10_movieDatBuffer;
    u32        m14_movieDatSize;
    void* m18_vdp1Buffer;

    static void Init(s_movieMainWorkArea* pThis, s32 movieIndex);
    static void Draw(s_movieMainWorkArea* pThis);
    static void Delete(s_movieMainWorkArea* pThis);
};
