#include "PDS.h"
#include "shared/vdp2PlaneTask.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "trigo.h"

// 060588e0
void vdp2ApplyWaveDistortion(sVdp2PlaneTask* pThis)
{
    std::vector<fixedPoint>& coefficients = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    s32 phase = pThis->m4C_wavePhase;
    for (int i = 0; i < 0x1A8 && i < (int)coefficients.size(); i++)
    {
        s32 sinVal = getSin((u16)((u32)phase >> 16) & 0xFFF);
        fixedPoint modulated = MTH_Mul(pThis->m48_waveAmplitude, sinVal);
        coefficients[i] = MTH_Mul(coefficients[i], modulated + 0x10000);
        phase += pThis->m44_waveFreq;
    }
    pThis->m4C_wavePhase += pThis->m40_waveSpeed;
}

// 06014274 — update line scroll table with wave distortion (shared across A7, B2)
void updateLineScrollTable(sVdp2PlaneTask* pThis)
{
    s32 phase = pThis->m68_lsPhaseAccum;
    pThis->m68_lsPhaseAccum = pThis->m54_lsPhaseSpeed + phase;
    s32 scrollBase = pThis->m6C_lsScrollBaseAccum + pThis->m60_lsScrollBaseSpeed;
    pThis->m6C_lsScrollBaseAccum = scrollBase;

    s32 amplitude = pThis->m5C_lsZoomAmplitude;
    s32 freq = pThis->m58_lsFreqPerLine;
    s32 scrollInc = pThis->m64_lsScrollIncPerLine;
    s32* buf = (s32*)pThis->m50_lineScrollBuffer;

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
    t.m38 = pThis->m32_projParam1;
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
    scaleRotationMatrix(pThis->m3C_scale);
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
    gVdp2RotationMatrix.Mz = ((pThis->mC_cameraPosition.m4_Y - pThis->m38_groundY) * 0x10)
                    - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
                    + (s32)(s16)t.m40 * -0x10000;
}
