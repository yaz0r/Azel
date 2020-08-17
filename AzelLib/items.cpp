#include "PDS.h"
#include "items.h"
#include "commonOverlay.h"

const sObjectListEntry* getObjectListEntry(eItems entry)
{
    static std::unordered_map<eItems, sObjectListEntry*> cache;

    std::unordered_map<eItems, sObjectListEntry*>::iterator cacheEntry = cache.find(entry);
    if (cacheEntry != cache.end())
    {
        return cacheEntry->second;
    }

    sSaturnPtr EA = gCommonFile->getSaturnPtr(0x20C3F4);
    EA += 3 * 4 * entry;

    sObjectListEntry* pNewObjectEntry = new sObjectListEntry;

    pNewObjectEntry->m0_flags = readSaturnS8(EA + 0);
    pNewObjectEntry->m1_type = readSaturnS8(EA + 1);
    pNewObjectEntry->m2 = readSaturnS8(EA + 2);
    pNewObjectEntry->m3 = readSaturnS8(EA + 3);
    pNewObjectEntry->m4_name = readSaturnString(readSaturnEA(EA + 4));
    pNewObjectEntry->m8_description = readSaturnString(readSaturnEA(EA + 8));

    cache.insert_or_assign(entry, pNewObjectEntry);

    return pNewObjectEntry;
}

s32 getObjectIcon(eItems objectID)
{
    const sObjectListEntry* pObject = getObjectListEntry(objectID);

    static const std::array<s32, 10> objectTypeLookupTable =
    { 0,1,2,3,4,5,6,7,8,9 };

    return objectTypeLookupTable[pObject->m1_type];
}
