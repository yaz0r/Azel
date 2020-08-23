#include "PDS.h"
#include "fieldItemBoxDefinition.h"
#include "items.h"

s_itemBoxDefinition* readItemBoxDefinition(sSaturnPtr ptr)
{
    s_itemBoxDefinition* pItemBoxDefinition = new s_itemBoxDefinition;

    pItemBoxDefinition->m0_pos = readSaturnVec3(ptr); ptr += 4 * 3;
    pItemBoxDefinition->mC_boundingMin = readSaturnVec3(ptr); ptr += 4 * 3;
    pItemBoxDefinition->m18_boundingMax = readSaturnVec3(ptr); ptr += 4 * 3;
    pItemBoxDefinition->m24_rotation = readSaturnVec3(ptr); ptr += 4 * 3;
    pItemBoxDefinition->m30_scale = readSaturnS32(ptr); ptr += 4;
    pItemBoxDefinition->m34_bitIndex = readSaturnS32(ptr); ptr += 4;
    pItemBoxDefinition->m38 = readSaturnS32(ptr); ptr += 4;
    assert(readSaturnS32(ptr) >= eItems::min);
    assert(readSaturnS32(ptr) <= eItems::max);
    pItemBoxDefinition->m3C_receivedItemId = (eItems)readSaturnS32(ptr); ptr += 4;
    pItemBoxDefinition->m40_receivedItemQuantity = readSaturnS8(ptr); ptr += 1;
    pItemBoxDefinition->m41_LCSType = readSaturnS8(ptr); ptr += 1;
    pItemBoxDefinition->m42 = readSaturnS8(ptr); ptr += 1;
    pItemBoxDefinition->m43 = readSaturnS8(ptr); ptr += 1;
    pItemBoxDefinition->m44 = readSaturnS8(ptr); ptr += 1;
    return pItemBoxDefinition;
}
