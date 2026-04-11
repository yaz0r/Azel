#pragma once

#include "task.h"
#include "common.h"

struct sA7SpawnedEntityChild;

// 06054b54
void a7SpawnedEntityChild_Init_06054b54(sA7SpawnedEntityChild* pThis);
// 06054b82
void a7SpawnedEntityChild_Update_06054b82(sA7SpawnedEntityChild* pThis);

struct sA7SpawnedEntityChild : public s_workAreaTemplate<sA7SpawnedEntityChild>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition td = {
            &a7SpawnedEntityChild_Init_06054b54,
            &a7SpawnedEntityChild_Update_06054b82,
            nullptr,
            nullptr,
        };
        return &td;
    }

    u32 m0_state;
    // size 0x4
};
