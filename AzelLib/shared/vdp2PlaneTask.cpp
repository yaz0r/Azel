#include "PDS.h"
#include "shared/vdp2PlaneTask.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "trigo.h"

// 060588e0 (FLD_A3), 0605441c (BTL_A3)
void vdp2ApplyWaveDistortion(sVdp2PlaneTask* pThis)
{
    std::vector<fixedPoint>& coefficients = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    s32 phase = pThis->m4C_wavePhase;
    for (int i = 0; i < 0x1A8 && i < (int)coefficients.size(); i++)
    {
        s32 sinVal = getSin((u16)((u32)phase >> 16) & 0xFFF);
        fixedPoint modulated = MTH_Mul(pThis->m48, sinVal);
        coefficients[i] = MTH_Mul(coefficients[i], modulated + 0x10000);
        phase += pThis->m44_waveFreq;
    }
    pThis->m4C_wavePhase += pThis->m40_waveSpeed;
}

// 06014274 - wave distortion on a raw params struct (used by battle envs via pThis+0x7C)
void updateWaveDistortionParams(sVdp2PlaneTask::sLineScrollParams* p)
{
    s32 phase = p->m18_phaseAccum;
    p->m18_phaseAccum = p->m4_phaseSpeed + phase;
    s32 scrollBase = p->m1C_scrollBaseAccum + p->m10_scrollBaseSpeed;
    p->m1C_scrollBaseAccum = scrollBase;

    s32 amplitude = p->mC_zoomAmplitude;
    s32 freq = p->m8_freqPerLine;
    s32 scrollInc = p->m14_scrollIncPerLine;
    s32* buf = (s32*)p->m0_buffer;

    u32 phaseIdx = (u32)phase;

    for (s32 i = 0; i < 0xE0; i++)
    {
        phaseIdx += (u32)freq;

        fixedPoint sinVal = getSin((phaseIdx >> 16) & 0xFFF);
        s32 divisor = amplitude + 0x10000 + MTH_Mul(fixedPoint(amplitude), sinVal).asS32();
        s32 quotient = (s32)(0x100000000LL / (s64)divisor);

        buf[i * 3 + 1] = (buf[i * 3 + 1] + scrollInc) & 0xFFFFFF;
        buf[i * 3 + 2] = quotient;
        buf[i * 3 + 0] = (0x10000 - quotient) * 0xB0 + scrollBase;
    }
}

// 0605b3d0 (FLD_B2) - line scroll update using sVdp2PlaneTask m50 line scroll params
void updateLineScrollTable(sVdp2PlaneTask* pThis)
{
    auto& ls = pThis->m50_lineScrollParams;
    s32 phase = ls.m18_phaseAccum;
    ls.m18_phaseAccum = ls.m4_phaseSpeed + phase;

    s32 amplitude = ls.mC_zoomAmplitude;
    s32 freq = ls.m8_freqPerLine;
    s32 scrollInc = ls.m14_scrollIncPerLine;
    s32* buf = (s32*)ls.m0_buffer;

    u32 phaseIdx = (u32)phase;

    for (s32 i = 0; i < 0xE0; i++)
    {
        phaseIdx += (u32)freq;

        fixedPoint sinVal = getSin((phaseIdx >> 16) & 0xFFF);
        s32 divisor = amplitude + 0x10000 + MTH_Mul(fixedPoint(amplitude), sinVal).asS32();
        s32 quotient = FP_Div(0x10000, fixedPoint(divisor)).asS32();

        buf[i * 3 + 1] = (buf[i * 3 + 1] + scrollInc) & 0xFFFFFF;
        buf[i * 3 + 2] = quotient;
        buf[i * 3 + 0] = (0x10000 - quotient) * 0xB0;
    }
}

// 060590ae
void vdp2SetupRotationPass(sVdp2PlaneTask* pThis)
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    fixedPoint rotX = pThis->m18_cameraRotation.m0_X;
    if ((s32)rotX == 0) rotX = fixedPoint(0xFFF49F4A);
    fixedPoint rotY = pThis->m18_cameraRotation.m4_Y;
    fixedPoint rotZ = pThis->m18_cameraRotation.m8_Z;

    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m30_vdp1ProjectionParam[1];
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
    scaleRotationMatrix(pThis->m3C_scale, pThis->m3C_scale);
    writeRotationParams(-rotZ);

    s32 diffX = (s32)t.m34 - (s32)t.m3C;
    s32 diffY = (s32)t.m36 - (s32)t.m3E;
    s32 diffZ = (s32)t.m38 - (s32)t.m40;

    gVdp2RotationMatrix.Mx = MTH_Mul(pThis->m3C_scale, (s32)pThis->mC_cameraPosition.m0_X << 4)
                    - gVdp2RotationMatrix.m[0][0] * diffX - gVdp2RotationMatrix.m[0][1] * diffY - gVdp2RotationMatrix.m[0][2] * diffZ
                    + (s32)(s16)t.m3C * -0x10000;
    gVdp2RotationMatrix.My = MTH_Mul(pThis->m3C_scale, (s32)pThis->mC_cameraPosition.m8_Z << 4)
                    - gVdp2RotationMatrix.m[1][0] * diffX - gVdp2RotationMatrix.m[1][1] * diffY - gVdp2RotationMatrix.m[1][2] * diffZ
                    + (s32)(s16)t.m3E * -0x10000;
    gVdp2RotationMatrix.Mz = ((pThis->mC_cameraPosition.m4_Y - pThis->m38) * 0x10)
                    - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
                    + (s32)(s16)t.m40 * -0x10000;
}

// 060540ac (BTL_A3), 06054090 (BTL_A3_2), 060540c8/06054a2e (BTL_X0), 060579f6 (BTL_A7_2), 0605d4ec (FLD_A5), 06057e76/06058b02 (FLD_A7),
// 0606dd7c (TWN_CARA), 06054230 (TWN_E006), 06054b80 (TWN_E011), 0605456c (TWN_E014), 06055bac (TWN_EXCA), 0606d584/0606dc8a (TWN_SEEK)
s32 computeRotationScrollOffset()
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 pivotX = (s32)gVdp2RotationMatrix.m[2][0] * (s32)t.m34;
    s32 pivotY = (s32)gVdp2RotationMatrix.m[2][1] * (s32)t.m36;
    s32 depthTerm = MTH_Mul(gVdp2RotationMatrix.m[2][2], (s32)t.m38 * 0x10000 - t.m8_Zst);
    return FP_Div(pivotX + pivotY + depthTerm, gVdp2RotationMatrix.m[2][1]).getInteger();
}
