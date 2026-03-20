#include "PDS.h"
#include "fieldDragonMovement.h"

// 0608904c — dragon type 8 (floater) movement
void floaterFlightUpdate(s_dragonTaskWorkArea* r14)
{
    r14->m24A_runningCameraScript = 0;
    if (r14->m104_dragonScriptStatus == 0)
    {
        s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
        pLCS->m8 &= ~1;
        r14->mF8_Flags |= 0x400;
        r14->mF8_Flags &= ~0x20000;
        r14->m104_dragonScriptStatus++;
    }

    r14->m160_deltaTranslation = {};
    r14->m238 &= 0xFC;

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 1)
        updateDragonMovementFromControllerType1(r14);
    else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        updateDragonMovementFromAnalogController(r14);
    else
        assert(0); // keyboard not supported

    integrateDragonMovement(r14);
}

// Activate dragon flight mode — selects normal or floater update function
void activateDragonFlight()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (gDragonState->mC_dragonType == DR_LEVEL_8_FLOATER)
        pDragon->mF0 = floaterFlightUpdate;
    else
        pDragon->mF0 = dragonFlightUpdate;
    pDragon->m104_dragonScriptStatus = 0;
    pDragon->mF8_Flags &= ~0x10000;
}
