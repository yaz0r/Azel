#include "PDS.h"
#include "campDragon.h"
#include "town/town.h"

struct sCampDragon : public s_workAreaTemplateWithArg<sCampDragon, sSaturnPtr>, sTownObject
{
    s16 m16;
    sSaturnPtr m48;
    sVec3_FP m4C;
    sVec3_FP m58;
    sVec3_FP m64;
    //size: 0x624
};

void sCampDragon_Init(sCampDragon* pThis, sSaturnPtr arg) {
    pThis->m48 = arg;
    pThis->m4C = pThis->m58 = readSaturnVec3(arg + 0x8);
    pThis->m64 = readSaturnVec3(arg + 0x14);
    pThis->m16 = 0x3C;

    Unimplemented();
}

void sCampDragon_Update(sCampDragon* pThis) {
    Unimplemented();
}

void sCampDragon_Draw(sCampDragon* pThis) {
    Unimplemented();
}

void sCampDragon_Delete(sCampDragon* pThis) {
    Unimplemented();
}

struct sTownObject* createCampDragon(p_workArea parent, sSaturnPtr arg) {
    static const sCampDragon::TypedTaskDefinition definition = {
        &sCampDragon_Init,
        &sCampDragon_Update,
        &sCampDragon_Draw,
        &sCampDragon_Delete,
    };

    return createSubTaskWithArg<sCampDragon, sSaturnPtr>(parent, arg, &definition);
}
