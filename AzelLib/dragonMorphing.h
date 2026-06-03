#pragma once

struct sDragonBuffer {
    s_fileBundle* m0_dramData;
    std::array<u8, 0x6800> m18E00_MCBInDram;
    // size 0x1F600;
};


struct sDragonMorphData : public s_workAreaTemplate<sDragonMorphData>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL };
        return &taskDefinition;
    }

    sDragonBuffer* m0_dramAllocation; //0
    u8* m4_vramAllocation;//4
    u8* m8_MCBInDram;//8
};

sDragonMorphData* createDragonMorphData(s_workArea* pWorkArea, e_dragonLevel dragonLevel);
void morphDragon(sDragonMorphData* pLoadDragonWorkArea, s_3dModel* pDragonStateSubData1, u8* pMCB, const sDragonMorphDataPerLevel* pDragonData3, s16 cursorX, s16 cursorY);
void deleteDragonMorphData(sDragonMorphData* pLoadDragonWorkArea);
