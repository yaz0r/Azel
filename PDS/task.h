#pragma once

#include <vector>
#include "heap.h"
struct s_workArea
{
    struct s_task* m_pTask;

    s_task* getTask()
    {
        return m_pTask;
    }

    void* operator new(size_t size)
    {
        void* ptr = malloc(size);
        memset(ptr, 0, size);

        return ptr;
    }
    /*
    virtual void Init(void* pArgument = NULL)
    {
        PDS_unimplemented("Init");
    }

    virtual void Update()
    {
        PDS_unimplemented("Update");
    }

    virtual void Draw()
    {
        PDS_unimplemented("Draw");
    }

    virtual void Delete()
    {
        PDS_unimplemented("Delete");
    }
    */

    void UnimplementedImpl(const char* functionName)
    {
        PDS_Logger.AddLog("Unimplemented: %s\n", functionName);
    }
};

#define TaskUnimplemented() { static bool printed = false; if(!printed) {printed = true; UnimplementedImpl(__FUNCTION__);}}

struct s_taskDefinition
{
    void(*m_pInit)(s_workArea*);
    void(*m_pUpdate)(s_workArea*);
    void(*m_pLateUpdate)(s_workArea*);
    void(*m_pDelete)(s_workArea*);
    const char* m_taskName;
};

struct s_taskDefinitionWithArg
{
    void(*m_pInit)(s_workArea*, void* argument);
    void(*m_pUpdate)(s_workArea*);
    void(*m_pLateUpdate)(s_workArea*);
    void(*m_pDelete)(s_workArea*);
    const char* m_taskName;
};

#define DECLARE_DEFAULT_TASK(pInit, pUpdate, pDraw, pDelete, name)\
static const TypedTaskDefinition* getTypedTaskDefinition() \
{\
    static const TypedTaskDefinition taskDefinition = { pInit, pUpdate, pDraw, pDelete, ##name };\
    return &taskDefinition;\
}


typedef s_workArea* p_workArea;

template<typename T, typename... argType>
struct s_workAreaTemplateWithArg : public s_workArea
{
    static s_taskDefinitionWithArg* getTaskDefinition()
    {
        static s_taskDefinitionWithArg taskDefinition = { T::StaticInit, T::StaticUpdate, T::StaticDraw, T::StaticDelete, "" };
        return &taskDefinition;
    }

    s_workAreaTemplateWithArg()
    {
        m_UpdateMethod = NULL;
        m_DrawMethod = NULL;
        m_DeleteMethod = NULL;
    }

    static T* ConvertType(p_workArea pWorkArea)
    {
        return static_cast<T*>(pWorkArea);
    }
    static void StaticInit(p_workArea pWorkArea, void* arg)
    {
        //T* pTask = ConvertType(pWorkArea);
    }
    static void StaticUpdate(p_workArea pWorkArea)
    {
        T* pTask = ConvertType(pWorkArea);
        if(pTask->m_UpdateMethod)
        {
            //printf("calling update for task %s\n", pTask->m_pTask->m_taskName);
            ((pTask)->*(pTask->m_UpdateMethod))();
        }
    }
    static void StaticDraw(p_workArea pWorkArea)
    {
        T* pTask = ConvertType(pWorkArea);
        if (pTask->m_DrawMethod)
        {
            //printf("calling draw for task %s\n", pTask->m_pTask->m_taskName);
            ((pTask)->*(pTask->m_DrawMethod))();
        }
    }
    static void StaticDelete(p_workArea pWorkArea)
    {
        T* pTask = ConvertType(pWorkArea);
        if (pTask->m_DeleteMethod)
        {
            //printf("calling delete for task %s\n", pTask->m_pTask->m_taskName);
            ((pTask)->*(pTask->m_DeleteMethod))();
        }
    }

    void dummyTaskDelete()
    {
        assert(0);
    }

    typedef void (T::*FunctionType)();
    typedef void (T::*InitFunctionType)(argType ...);
    //InitFunctionType m_InitMethod;
    FunctionType m_UpdateMethod;
    FunctionType m_DrawMethod;
    FunctionType m_DeleteMethod;

    struct TypedTaskDefinition
    {
        InitFunctionType m_pInit;
        FunctionType m_pUpdate;
        FunctionType m_pDraw;
        FunctionType m_pDelete;
        const char* m_taskName;
    };

    static TypedTaskDefinition setupTypeDefinition(InitFunctionType fInit, FunctionType fUpdate, FunctionType fDraw, FunctionType fDelete, const char* name)
    {
        TypedTaskDefinition temp;
        temp.m_pInit = fInit;
        temp.m_pUpdate = fUpdate;
        temp.m_pDraw = fDraw;
        temp.m_pDelete = fDelete;
        temp.m_taskName = name;

        return temp;
    }
};

template<typename T>
struct s_workAreaTemplate : public s_workAreaTemplateWithArg<T>
{

};

#define TASK_FLAGS_FINISHED 1
#define TASK_FLAGS_PAUSED 2
#define TASK_FLAGS_DELETING 4

struct s_task
{
    s_task() :
        m0_pNextTask(nullptr),
        m4_pSubTask(nullptr),
        m8_pUpdate(nullptr),
        mC_pDraw(nullptr),
        m10_pDelete(nullptr),
        m14_flags(0),
        m_taskName(nullptr),
        m_workArea(nullptr),
        m_heapNode(nullptr)
    {

    }
    s_task* m0_pNextTask;
    s_task* m4_pSubTask;
    void(*m8_pUpdate)(p_workArea);
    void(*mC_pDraw)(p_workArea);
    void(*m10_pDelete)(p_workArea);
    u32 m14_flags;
    const char* m_taskName;

    s_workArea* m_workArea;
    s_heapNode* m_heapNode;

    p_workArea getWorkArea()
    {
        return m_workArea;
    }
    s_heapNode* getHeapNode()
    {
        return m_heapNode;
    }

    bool isFinished()
    {
        return (m14_flags & TASK_FLAGS_FINISHED) ? true : false;
    }
    void markFinished()
    {
        m14_flags |= TASK_FLAGS_FINISHED;
    }
    bool isPaused()
    {
        return (m14_flags & TASK_FLAGS_PAUSED) ? true : false;
    }
    void clearPaused()
    {
        m14_flags &= ~TASK_FLAGS_PAUSED;
    }
    void markPaused()
    {
        m14_flags |= TASK_FLAGS_PAUSED;
    }
    bool isDeleting()
    {
        return (m14_flags & TASK_FLAGS_DELETING) ? true : false;
    }
private:
    void markDeleting()
    {
        m14_flags |= TASK_FLAGS_DELETING;
    }
};

void resetTasks();
void runTasks();

template<typename T>
T* createSubTask(p_workArea parentTask, const typename T::TypedTaskDefinition* pTypeTaskDefinition = T::getTypedTaskDefinition())
{
    T* pNewTask = static_cast<T*>(createSubTaskWithArg(parentTask, T::getTaskDefinition(), new T, NULL));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = pTypeTaskDefinition->m_taskName;
    if (pTypeTaskDefinition->m_pInit)
    {
        ((pNewTask)->*(pTypeTaskDefinition->m_pInit))();
    }
    return pNewTask;
}

template<typename T, typename argType>
T* createSubTaskWithArg(p_workArea parentTask, argType arg, const typename T::TypedTaskDefinition* pTypeTaskDefinition = T::getTypedTaskDefinition())
{
    T* pNewTask = static_cast<T*>(createSubTaskWithArg(parentTask, T::getTaskDefinition(), new T, nullptr));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = pTypeTaskDefinition->m_taskName;
    if (pTypeTaskDefinition->m_pInit)
    {
        ((pNewTask)->*(pTypeTaskDefinition->m_pInit))(arg);
    }
    return pNewTask;
}

template<typename T, typename argType>
T* createSiblingTaskWithArg(p_workArea parentTask, argType arg, const typename T::TypedTaskDefinition* pTypeTaskDefinition = T::getTypedTaskDefinition())
{
    T* pNewTask = static_cast<T*>(createSiblingTaskWithArg(parentTask, T::getTaskDefinition(), new T, nullptr));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = pTypeTaskDefinition->m_taskName;
    if (pTypeTaskDefinition->m_pInit)
    {
        ((pNewTask)->*(pTypeTaskDefinition->m_pInit))(arg);
    }
    return pNewTask;
}

template<typename T>
T* createSubTaskFromFunction(p_workArea parentTask, typename T::FunctionType UpdateFunction)
{
    T* pNewTask = static_cast<T*>(createSubTaskWithArg(parentTask, T::getTaskDefinition(), new T, NULL));
    pNewTask->m_UpdateMethod = UpdateFunction;
    return pNewTask;
}

p_workArea createSubTask(p_workArea workArea, const s_taskDefinition* pDefinition, p_workArea pNewWorkArea);
p_workArea createSubTaskWithArg(p_workArea workArea, const s_taskDefinitionWithArg* pDefinition, p_workArea pNewWorkArea, void* argument);
p_workArea createSiblingTaskWithArg(p_workArea workArea, const s_taskDefinitionWithArg* pDefinition, p_workArea pNewWorkArea, void* argument);

p_workArea createSubTaskFromFunction(p_workArea workArea, void(*pFunction)(p_workArea), p_workArea pNewWorkArea, const char* name);

template<typename T>
T* createRootTask()
{
    T* pNewTask = static_cast<T*>(createRootTask(T::getTaskDefinition(), new T));
    auto pTypeTaskDefinition = T::getTypedTaskDefinition();
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = pTypeTaskDefinition->m_taskName;
    if (pTypeTaskDefinition->m_pInit)
    {
        ((pNewTask)->*(pTypeTaskDefinition->m_pInit))();
    }
    return pNewTask;
}
s_workArea* createRootTask(s_taskDefinitionWithArg* pDefinition, p_workArea pNewWorkArea);


void dummyTaskInit(s_workArea* pWorkArea);
void dummyTaskInitWithArg(s_workArea* pWorkArea, void* arg);
void dummyTaskUpdate(s_workArea* pWorkArea);
void dummyTaskDraw(s_workArea* pWorkArea);
void dummyTaskDelete(s_workArea* pWorkArea);

struct s_dummyWorkArea : s_workArea
{

};

void DebugTasks();

