#include "PDS.h"
#include "ArachnothTentacle.h"

void arachnothTentacle_updateMode1(sArachnothTentacle* pThis) {
    fixedPoint cosValue = getCos(pThis->m178_rotation->m_value[0].toInteger());
    fixedPoint iVar1 = MTH_Mul_5_6(cosValue, getSin(pThis->m178_rotation->m_value[1].toInteger()), 0xF000);
    fixedPoint iVar2 = MTH_Mul(-getSin(pThis->m178_rotation->m_value[0].toInteger()), 0xF000);
    fixedPoint rot3 = MTH_Mul_5_6(cosValue, getCos(pThis->m178_rotation->m_value[1].toInteger()), 0xF000);

    pThis->mC_segments[0].m0[0] += MTH_Mul((iVar1 + pThis->m174_position->m_value[0]) - pThis->mC_segments[0].m18[0], 0x28f);
    pThis->mC_segments[0].m0[1] += MTH_Mul((iVar2 + pThis->m174_position->m_value[1]) - pThis->mC_segments[0].m18[1], 0x28f);
    pThis->mC_segments[0].m0[2] += MTH_Mul((iVar2 + pThis->m174_position->m_value[2]) - pThis->mC_segments[0].m18[2], 0x28f);

    int segmentIndex = MTH_Mul(randomNumber() >> 0x10, 9);
    pThis->mC_segments[segmentIndex].m0[0] += MTH_Mul(randomNumber() >> 0x10, 0x666) - 0x333;
    pThis->mC_segments[segmentIndex].m0[1] += MTH_Mul(randomNumber() >> 0x10, 0x666) - 0x333;
    pThis->mC_segments[segmentIndex].m0[2] += MTH_Mul(randomNumber() >> 0x10, 0x666) - 0x333;
}

void arachnothTentacle_updateMode2(sArachnothTentacle* pThis) {
    Unimplemented();
}

void arachnothTentacle_update(sArachnothTentacle* pThis) {
    switch (pThis->m188_currentMode) {
    case 0:
        break;
    case 1:
        arachnothTentacle_updateMode1(pThis);
        break;
    case 2:
        arachnothTentacle_updateMode2(pThis);
        break;
    default:
        assert(0);
    }

    if (pThis->m188_currentMode != 4) {
        sMatrix4x3 matrix;
        initMatrixToIdentity(&matrix);
        rotateMatrixShiftedY(pThis->m178_rotation->m_value[1], &matrix);
        rotateMatrixShiftedX(pThis->m178_rotation->m_value[0], &matrix);
        rotateMatrixShiftedZ(pThis->m178_rotation->m_value[2], &matrix);

        sVec3_FP transformedPoint;
        transformAndAddVec(pThis->m17C_offset, transformedPoint, matrix);

        pThis->mC_segments[9].m0.zeroize();
        pThis->mC_segments[9].mC.zeroize();
        pThis->mC_segments[9].m18 = transformedPoint + *pThis->m174_position;
    }

    for (int i = 0; i < 9; i++) {
        sVec3_FP temp = pThis->mC_segments[i + 1].m18 - pThis->mC_segments[i].m18;
        temp[0] = MTH_Mul(temp[0], pThis->m190);
        temp[1] = MTH_Mul(temp[1], pThis->m190);
        temp[2] = MTH_Mul(temp[2], pThis->m190);

        pThis->mC_segments[i].m0 += temp;
    }

    for (int i = 9; i > 0; i--) {
        sVec3_FP temp = pThis->mC_segments[i - 1].m18 - pThis->mC_segments[i].m18;
        temp[0] = MTH_Mul(temp[0], pThis->m190);
        temp[1] = MTH_Mul(temp[1], pThis->m190);
        temp[2] = MTH_Mul(temp[2], pThis->m190);

        pThis->mC_segments[i - 1].m0 += temp;
    }

    if (pThis->m188_currentMode == 3) {
        Unimplemented();
    }
    else {
        Unimplemented();
    }
}

void arachnothTentacle_draw(sArachnothTentacle* pThis) {
    Unimplemented();
}

sArachnothTentacle* createArachnothTentacle(s_workAreaCopy* pParent, sVec3_FP* position, sVec3_FP* rotation, sVec3_FP* offset) {
    static const sArachnothTentacle::TypedTaskDefinition definition = {
        nullptr,
        arachnothTentacle_update,
        arachnothTentacle_draw,
        nullptr,
    };

    sArachnothTentacle* pNewTask = createSubTaskWithCopy<sArachnothTentacle>(pParent, &definition);

    Unimplemented();

    pNewTask->m174_position = position;
    pNewTask->m178_rotation = rotation;
    pNewTask->m17C_offset = *offset;

    sMatrix4x3 matrix;
    initMatrixToIdentity(&matrix);
    rotateMatrixShiftedY(pNewTask->m178_rotation->m_value[1], &matrix);
    rotateMatrixShiftedX(pNewTask->m178_rotation->m_value[0], &matrix);
    rotateMatrixShiftedZ(pNewTask->m178_rotation->m_value[2], &matrix);

    sVec3_FP transformedPoint;
    transformAndAddVec(pNewTask->m17C_offset, transformedPoint, matrix);

    transformedPoint += *pNewTask->m174_position;

    for (int i = 0; i < 10; i++) {
        pNewTask->mC_segments[i].m0.zeroize();
        pNewTask->mC_segments[i].mC.zeroize();
        pNewTask->mC_segments[i].m18 = transformedPoint;
    }

    pNewTask->m188_currentMode = 0;
    pNewTask->m190 = MTH_Mul(randomNumber() >> 0x10, 0x51F);
    pNewTask->m194 = MTH_Mul(randomNumber() >> 0x10, 0x51F);

    return pNewTask;
}

void arachnothTentacle_setMode1(sArachnothTentacle* pThis) {
    pThis->m188_currentMode = 1;
}

void arachnothTentacle_setMode2(sArachnothTentacle* pThis) {
    pThis->m188_currentMode = 2;
}
