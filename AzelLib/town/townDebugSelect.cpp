#include "PDS.h"
#include "town.h"
#include "townDebugSelect.h"

struct locationTask : public s_workAreaTemplate<locationTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &locationTask::Init, &locationTask::Update, &locationTask::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(locationTask* pThis)
    {
    }

    static void Update(locationTask* pThis)
    {
    }

    static void Draw(locationTask* pThis)
    {
    }

    //size 14
};

p_workArea createLocationTask(p_workArea r4, s32 r5)
{
    townDebugTask2 = createSubTaskFromFunction<townDebugTask2Function>(r4, &townDebugTask2Function::Update);
    createSubTask<locationTask>(townDebugTask2);

    return townDebugTask2;
}
