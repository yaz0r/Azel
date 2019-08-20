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

    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Delete() = 0;

    template <typename T>
    T* castTo()
    {
        return static_cast<T*>(this);
    }
};

static void UnimplementedImpl(const char* functionName)
{
#ifndef SHIPPING_BUILD
    PDS_Logger[eLogCategories::log_unimlemented].AddLog("Unimplemented: %s\n", functionName);
#endif // !SHIPPING_BUILD
}

#define FunctionUnimplemented() { static bool printed = false; if(!printed) {printed = true; UnimplementedImpl(__FUNCTION__);}}

typedef s_workArea* p_workArea;

template<typename T, typename... argType>
struct s_workAreaTemplateWithArg : public s_workArea
{
    static constexpr const char* getTaskName()
    {
        return typeid(T).name();
    }

    s_workAreaTemplateWithArg()
    {
        m_UpdateMethod = NULL;
        m_DrawMethod = NULL;
        m_DeleteMethod = NULL;
    }

    virtual void Update()
    {
        if(m_UpdateMethod)
            m_UpdateMethod(static_cast<T*>(this));
    }

    virtual void Draw()
    {
        if (m_DrawMethod)
            m_DrawMethod(static_cast<T*>(this));
    }

    virtual void Delete()
    {
        if (m_DeleteMethod)
            m_DeleteMethod(static_cast<T*>(this));
    }

    typedef void (*FunctionType)(T*);
    typedef void (*InitFunctionType)(T*, argType ...);

    FunctionType m_UpdateMethod; //8
    FunctionType m_DrawMethod; //C
    FunctionType m_DeleteMethod;

    struct TypedTaskDefinition
    {
        InitFunctionType m_pInit;
        FunctionType m_pUpdate;
        FunctionType m_pDraw;
        FunctionType m_pDelete;
    };
};

template<typename T>
struct s_workAreaTemplate : public s_workAreaTemplateWithArg<T>
{
};

struct s_workAreaCopy : public s_workArea
{
    struct s_fileBundle* m0_dramAllocation;
    struct s_vdp1AllocationNode* m4_vd1Allocation;
};

template<typename T, typename... argType>
struct s_workAreaTemplateWithArgWithCopy : public s_workAreaCopy
{
    static constexpr const char* getTaskName()
    {
        return typeid(T).name();
    }

    s_workAreaTemplateWithArgWithCopy()
    {
        m_UpdateMethod = NULL;
        m_DrawMethod = NULL;
        m_DeleteMethod = NULL;
    }

    virtual void Update()
    {
        if (m_UpdateMethod)
            m_UpdateMethod(static_cast<T*>(this));
    }

    virtual void Draw()
    {
        if (m_DrawMethod)
            m_DrawMethod(static_cast<T*>(this));
    }

    virtual void Delete()
    {
        if (m_DeleteMethod)
            m_DeleteMethod(static_cast<T*>(this));
    }

    typedef void (*FunctionType)(T*);
    typedef void (*InitFunctionType)(T*, argType ...);

    FunctionType m_UpdateMethod;
    FunctionType m_DrawMethod;
    FunctionType m_DeleteMethod;

    struct TypedTaskDefinition
    {
        InitFunctionType m_pInit;
        FunctionType m_pUpdate;
        FunctionType m_pDraw;
        FunctionType m_pDelete;
    };
};

template<typename T>
struct s_workAreaTemplateWithCopy : public s_workAreaTemplateWithArgWithCopy<T>
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
        m14_flags(0),
        m_taskName(nullptr),
        m_workArea(nullptr),
        m_heapNode(nullptr)
    {

    }
    s_task* m0_pNextTask;
    s_task* m4_pSubTask;
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
    T* pNewTask = static_cast<T*>(createSubTaskWithArg(parentTask, new T));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = T::getTaskName();
    if (pTypeTaskDefinition->m_pInit)
    {
        pTypeTaskDefinition->m_pInit(pNewTask);
        //((pNewTask)->*(pTypeTaskDefinition->m_pInit))();
    }
    PDS_CategorizedLog(eLogCategories::log_task, "Created task %s\n", T::getTaskName());
    return pNewTask;
}

template<typename T>
T* createSubTaskWithCopy(s_workAreaCopy* parentTask, const typename T::TypedTaskDefinition* pTypeTaskDefinition = T::getTypedTaskDefinition())
{
    T* pNewTask = static_cast<T*>(createSubTaskWithArg(parentTask, new T));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = T::getTaskName();

    //copy
    pNewTask->m0_dramAllocation = parentTask->m0_dramAllocation;
    pNewTask->m4_vd1Allocation = parentTask->m4_vd1Allocation;

    if (pTypeTaskDefinition->m_pInit)
    {
        pTypeTaskDefinition->m_pInit(pNewTask);
        //((pNewTask)->*(pTypeTaskDefinition->m_pInit))();
    }
    PDS_CategorizedLog(eLogCategories::log_task, "Created task %s\n", T::getTaskName());
    return pNewTask;
}

template<typename T, typename argType>
T* createSubTaskWithArg(p_workArea parentTask, argType arg, const typename T::TypedTaskDefinition* pTypeTaskDefinition = T::getTypedTaskDefinition())
{
    T* pNewTask = static_cast<T*>(createSubTaskWithArg(parentTask, new T));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = T::getTaskName();
    if (pTypeTaskDefinition->m_pInit)
    {
        pTypeTaskDefinition->m_pInit(pNewTask, arg);
        //((pNewTask)->*(pTypeTaskDefinition->m_pInit))(arg);
    }
    PDS_CategorizedLog(eLogCategories::log_task, "Created task %s\n", T::getTaskName());
    return pNewTask;
}

template<typename T, typename argType>
T* createSiblingTaskWithArg(p_workArea parentTask, argType arg, const typename T::TypedTaskDefinition* pTypeTaskDefinition = T::getTypedTaskDefinition())
{
    T* pNewTask = static_cast<T*>(createSiblingTaskWithArg(parentTask, new T));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = T::getTaskName();
    if (pTypeTaskDefinition->m_pInit)
    {
        pTypeTaskDefinition->m_pInit(pNewTask, arg);
        //((pNewTask)->*(pTypeTaskDefinition->m_pInit))(arg);
    }
    PDS_CategorizedLog(eLogCategories::log_task, "Created task %s\n", T::getTaskName());
    return pNewTask;
}

template<typename T, typename argType>
T* createSiblingTaskWithArgWithCopy(s_workAreaCopy* parentTask, argType arg, const typename T::TypedTaskDefinition* pTypeTaskDefinition = T::getTypedTaskDefinition())
{
    T* pNewTask = static_cast<T*>(createSiblingTaskWithArg(parentTask, new T));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = T::getTaskName();

    //copy
    pNewTask->m0_dramAllocation = parentTask->m0_dramAllocation;
    pNewTask->m4_vd1Allocation = parentTask->m4_vd1Allocation;

    if (pTypeTaskDefinition->m_pInit)
    {
        pTypeTaskDefinition->m_pInit(pNewTask, arg);
        //((pNewTask)->*(pTypeTaskDefinition->m_pInit))(arg);
    }
    PDS_CategorizedLog(eLogCategories::log_task, "Created task %s\n", T::getTaskName());
    return pNewTask;
}

template<typename T>
T* createSiblingTask(p_workArea parentTask, const typename T::TypedTaskDefinition* pTypeTaskDefinition = T::getTypedTaskDefinition())
{
    T* pNewTask = static_cast<T*>(createSiblingTaskWithArg(parentTask, new T));
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = T::getTaskName();
    if (pTypeTaskDefinition->m_pInit)
    {
        pTypeTaskDefinition->m_pInit(pNewTask);
        //((pNewTask)->*(pTypeTaskDefinition->m_pInit))(arg);
    }
    PDS_CategorizedLog(eLogCategories::log_task, "Created task %s\n", T::getTaskName());
    return pNewTask;
}

template<typename T>
T* createSubTaskFromFunction(p_workArea parentTask, typename T::FunctionType UpdateFunction)
{
    T* pNewTask = static_cast<T*>(createSubTaskWithArg(parentTask, new T));
    pNewTask->m_UpdateMethod = UpdateFunction;
    pNewTask->getTask()->m_taskName = T::getTaskName();
    PDS_CategorizedLog(eLogCategories::log_task, "Created task (from function) %s\n", T::getTaskName());
    return pNewTask;
}

p_workArea createSubTask(p_workArea workArea, p_workArea pNewWorkArea);
p_workArea createSubTaskWithArg(p_workArea workArea, p_workArea pNewWorkArea);
p_workArea createSiblingTaskWithArg(p_workArea workArea, p_workArea pNewWorkArea);

template<typename T>
T* createRootTask()
{
    T* pNewTask = static_cast<T*>(createRootTask(new T));
    auto pTypeTaskDefinition = T::getTypedTaskDefinition();
    pNewTask->m_UpdateMethod = pTypeTaskDefinition->m_pUpdate;
    pNewTask->m_DrawMethod = pTypeTaskDefinition->m_pDraw;
    pNewTask->m_DeleteMethod = pTypeTaskDefinition->m_pDelete;
    pNewTask->getTask()->m_taskName = T::getTaskName();
    if (pTypeTaskDefinition->m_pInit)
    {
        pTypeTaskDefinition->m_pInit(pNewTask);
        //((pNewTask)->*(pTypeTaskDefinition->m_pInit))();
    }
    return pNewTask;
}
s_workArea* createRootTask(p_workArea pNewWorkArea);


void dummyTaskInit(s_workArea* pWorkArea);
void dummyTaskInitWithArg(s_workArea* pWorkArea, void* arg);
void dummyTaskUpdate(s_workArea* pWorkArea);
void dummyTaskDraw(s_workArea* pWorkArea);
void dummyTaskDelete(s_workArea* pWorkArea);

struct s_dummyWorkArea : s_workArea
{

};

void DebugTasks();

