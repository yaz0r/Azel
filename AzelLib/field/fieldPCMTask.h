#pragma once

// 06093960 (A3): voice played by field script opcode 34. The Saturn streams it from CD; SoLoud plays it here.
struct sFieldPCMTask : public s_workAreaTemplateWithArg<sFieldPCMTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &Init, nullptr, &Draw, &Delete };
        return &taskDefinition;
    }

    static void Init(sFieldPCMTask* pThis, sSaturnPtr pPcmNameEA);
    static void Draw(sFieldPCMTask* pThis);
    static void Delete(sFieldPCMTask* pThis);

    SoLoud::handle m_soloudHandle; // C++ only: replaces the Saturn PCM stream
    // size 0x1DC
};

s32 fieldPlayPCM(sSaturnPtr pPcmNameEA);
