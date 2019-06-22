#include "PDS.h"
#include "twn_ruin.h"
#include "town/town.h"
#include "town/townScript.h"

void updateEdgePosition(sNPC* r4);

struct TWN_RUIN_data : public sSaturnMemoryFile
{
    void init()
    {

    }
};

TWN_RUIN_data* gTWN_RUIN = NULL;

u8 townVDP1Buffer[0x63800];

s32 townVar0;

const char* listOfFilesToLoad[] = {
    "COMMON3.MCB",
    "COMMON3.CGB",
    "RUINMP.MCB",
    "RUINMP.CGB",
    nullptr
};

void townOverlayDelete(townDebugTask2Function* pThis)
{
    FunctionUnimplemented();
}

struct sRuinBackgroundTask : public s_workAreaTemplate<sRuinBackgroundTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sRuinBackgroundTask::Init, &sRuinBackgroundTask::Update, &sRuinBackgroundTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(sRuinBackgroundTask* pThis)
    {
        reinitVdp2();
        FunctionUnimplemented();
    }

    static void Update(sRuinBackgroundTask* pThis)
    {
        FunctionUnimplemented();
    }

    static void Draw(sRuinBackgroundTask* pThis)
    {
        FunctionUnimplemented();
    }

    // size 0x9C
};

void startRuinBackgroundTask(p_workArea pThis)
{
    createSubTask<sRuinBackgroundTask>(pThis);
}

void registerNpcs(sSaturnPtr r4_objectTable, sSaturnPtr r5_script, s32 r6)
{
    npcData0.m0_numExtraScriptsIterations = 0;
    npcData0.m5E = -1;
    npcData0.m60 = r4_objectTable;
    npcData0.mFC = 0;
    npcData0.m100 = 0;
    npcData0.m11C_currentStackPointer = npcData0.m120_stack.end();
    npcData0.m164 = 0;
    npcData0.m168 = 0;
    npcData0.m16C_displayStringTask = 0;
    npcData0.m170 = 0;

    for (int i = 0; i < npcData0.m70_npcPointerArray.size(); i++)
    {
        npcData0.m70_npcPointerArray[i] = nullptr;
    }

    townVar0 = 0;

    npcData0.mFC |= 0xF;

    npcData0.m104_scriptPtr = r5_script;
    npcData0.m108 = 0;
    npcData0.m10C = 0;

    npcData0.m116 = r6;

    npcData0.mF0 = 0;
    npcData0.mF4 = 0;
}

void applyAnimation(u8* base, u32 offset, std::vector<sPoseData>::iterator& pose)
{
    u8* r13 = base + offset;

    pushCurrentMatrix();
    {
        translateCurrentMatrix(&pose->m0_translation);
        rotateCurrentMatrixZYX(&pose->mC_rotation);
        if (READ_BE_U32(r13))
        {
            addObjectToDrawList(base, READ_BE_U32(r13));
        }
        if (READ_BE_U32(r13 + 4))
        {
            pose++;
            applyAnimation(base, READ_BE_U32(r13 + 4), pose);
        }
    }
    popMatrix();
    if (READ_BE_U32(r13 + 8))
    {
        pose++;
        applyAnimation(base, READ_BE_U32(r13 + 8), pose);
    }

}

void applyEdgeAnimation(s_3dModel* pModel, sVec2_FP* r5)
{
    std::vector<sPoseData>::iterator r14 = pModel->m2C_poseData.begin();
    u8* r12 = pModel->m4_pModelFile + READ_BE_U32(pModel->m4_pModelFile + pModel->mC_modelIndexOffset);
    r12 = pModel->m4_pModelFile + READ_BE_U32(r12 + 4);

    return applyAnimation(pModel->m4_pModelFile, READ_BE_U32(r12 + 4), r14);

    pushCurrentMatrix();
    {
        translateCurrentMatrix(&r14->m0_translation);
        rotateCurrentMatrixZYX(&r14->mC_rotation);

        pushCurrentMatrix();
        {
            r14++;
            translateCurrentMatrix(&r14->m0_translation);
            rotateCurrentMatrixShiftedZ(r14->mC_rotation[2]);
            rotateCurrentMatrixShiftedY(r14->mC_rotation[1] + MTH_Mul(r5->m_value[1], 0x4CCC));
            rotateCurrentMatrixShiftedX(r14->mC_rotation[0]);

            if (READ_BE_U32(r12))
            {
                addObjectToDrawList(pModel->m4_pModelFile, READ_BE_U32(r12));
            }

            u8* r13 = pModel->m4_pModelFile + READ_BE_U32(r12 + 4);
            pushCurrentMatrix();
            {
                r14++;
                translateCurrentMatrix(&r14->m0_translation);
                rotateCurrentMatrixShiftedZ(r14->mC_rotation[2]);
                rotateCurrentMatrixShiftedY(r14->mC_rotation[1] + MTH_Mul(r5->m_value[1], 0xB333));
                rotateCurrentMatrixShiftedX(r14->mC_rotation[0]);

                if (READ_BE_U32(r13))
                {
                    addObjectToDrawList(pModel->m4_pModelFile, READ_BE_U32(r13));
                }

                if (READ_BE_U32(r13 + 4))
                {
                    r14++;
                    applyAnimation(pModel->m4_pModelFile, READ_BE_U32(r13 + 4), r14);
                }
            }
            popMatrix();

            if (READ_BE_U32(r13 + 8))
            {
                r14++;
                applyAnimation(pModel->m4_pModelFile, READ_BE_U32(r13 + 8), r14);
            }
        }
        popMatrix();

        if (READ_BE_U32(r12 + 8))
        {
            r14++;
            applyAnimation(pModel->m4_pModelFile, READ_BE_U32(r12 + 8), r14);
        }
    }
}


void applyEdgeAnimation2(s_3dModel* pModel, sVec2_FP* r5)
{
    FunctionUnimplemented();
    applyEdgeAnimation(pModel, r5);
}

void EdgeUpdateSub0(sMainLogic_74*)
{
    FunctionUnimplemented();
}

struct sEdgeTask : public s_workAreaTemplateWithArgWithCopy<sEdgeTask, sSaturnPtr>, sNPC
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sEdgeTask::Init, &sEdgeTask::Update, &sEdgeTask::Draw, &sEdgeTask::Delete };
        return &taskDefinition;
    }

    static void initEdgeNPCSub0(sEdgeTask* pThis, s32 r5, sSaturnPtr r6)
    {
        s32 r3 = 0;
        if (r5 & 0x80)
        {
            r3 = 0x80;
        }

        pThis->mF |= r3;
        pThis->mD = 0x3F & r5;
        pThis->m18 = r6;
        switch (pThis->mD)
        {
        case 3:
            assert(r6.m_file == gTWN_RUIN);
            assert(r6.m_offset == 0x605B8D4);
            pThis->m14_updateFunction = &updateEdgePosition;
            break;
        default:
            assert(0);
            break;
        }
    }

    static void initEdgeNPCSub1(sEdgeTask* pThis)
    {
        pThis->m179 = 0;
        pThis->m178 = 0;
        pThis->m17A = 0;
    }

    static void initEdgeNPC(sEdgeTask* pThis, sSaturnPtr arg)
    {
        npcData0.m70_npcPointerArray[readSaturnU8(arg + 0x20)] = pThis;
        pThis->mC = 0;
        pThis->m10_InitPtr = arg;
        pThis->m1C = readSaturnS32(arg + 0x28);
        pThis->m30 = readSaturnEA(arg + 0x2C);
        pThis->mE8.m0_position = readSaturnVec3(arg + 0x8);
        pThis->mE8.mC_rotation = readSaturnVec3(arg + 0x14);

        initEdgeNPCSub0(pThis, readSaturnU8(arg + 0x21), readSaturnEA(arg + 0x30));

        if (pThis->mD == 4)
        {
            pThis->m14E = 1;
        }

        pThis->m84.m30_pPosition = &pThis->mE8.m0_position;
        pThis->m84.m34_pRotation = &pThis->mE8.mC_rotation;
        pThis->m84.m38_pOwner = pThis;
        pThis->m84.m3C = readSaturnS32(arg + 0x38);
        if (u16 offset = readSaturnU16(arg + 0x36))
        {
            pThis->m84.m40 = pThis->m0_dramAllocation + offset;
        }
        else
        {
            pThis->m84.m40 = nullptr;
        }

        mainLogicInitSub0(&pThis->m84, readSaturnU8(arg + 0x34));
        mainLogicInitSub1(&pThis->m84, arg + 0x3C, arg + 0x48);
        initEdgeNPCSub1(pThis);
        pThis->m17B = 0;
    }

    static void Init(sEdgeTask* pThis, sSaturnPtr arg)
    {
        initEdgeNPC(pThis, arg);

        init3DModelRawData(pThis, &pThis->m34_3dModel, 0x100, pThis->m0_dramAllocation, readSaturnU16(arg + 0x22), nullptr, pThis->m0_dramAllocation + READ_BE_U32(pThis->m0_dramAllocation + readSaturnU16(arg + 0x24)), nullptr, nullptr);

        if (readSaturnU8(arg + 0x21) & 0x40)
        {
            assert(0);
        }
    }

    static void Update(sEdgeTask* pThis)
    {
        sNPCE8* r12 = &pThis->mE8;

        pThis->mE8.m54_oldPosition = pThis->mE8.m0_position;

        if (pThis->mC)
        {
            assert(0);
        }
        else
        {
            pThis->m14_updateFunction(pThis);
        }

        //605A01E
        switch (pThis->mE)
        {
        case 0:
            if (pThis->m17A)
            {
                //605A07C
                assert(0);
            }
            break;
        case 1:
        {
            sVec3_FP var0 = r12->m0_position - r12->m54_oldPosition;
            var0 *= var0;
            s32 r4 = sqrt_I(var0[0] + var0[1] + var0[2]) * 0x1E1;
            pThis->m28 += r4;

            s32 r12 = pThis->m28;
            if (r4)
            {
                //0x605A1D0
                if (pThis->m2C != 1)
                {
                    pThis->m2C = 1;
                    sSaturnPtr var0 = pThis->m30 + 4;
                    u8* buffer;
                    if (readSaturnU16(var0))
                    {
                        buffer = dramAllocatorEnd[0].mC_buffer->m0_dramAllocation;
                    }
                    else
                    {
                        buffer = pThis->m0_dramAllocation;
                    }

                    playAnimationGeneric(&pThis->m34_3dModel, buffer + READ_BE_U32(readSaturnU16(var0 + 2) + buffer), 5);
                }
                r12 >>= 16;
            }
            else
            {
                //0x605A206
                if (pThis->m2C)
                {
                    //0x605A20C
                    assert(0);
                }

                r12 = 1;
            }

            if (r12)
            {
                do 
                {
                    stepAnimation(&pThis->m34_3dModel);
                } while (--r12);
            }

            interpolateAnimation(&pThis->m34_3dModel);

            break;
        }
        default:
            assert(0);
            break;
        }

        EdgeUpdateSub0(&pThis->m84);
    }

    static void Draw(sEdgeTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->mE8.m0_position);
        rotateCurrentMatrixShiftedY(pThis->mE8.mC_rotation[1]);
        rotateCurrentMatrixShiftedX(pThis->mE8.mC_rotation[0]);
        rotateCurrentMatrixShiftedY(0x8000000);

        if (pThis->mF & 0x80)
        {
            assert(0);
        }

        if (pThis->m34_3dModel.m48_poseDataInterpolation.size())
        {
            applyEdgeAnimation2(&pThis->m34_3dModel, &pThis->m20);
        }
        else
        {
            applyEdgeAnimation(&pThis->m34_3dModel, &pThis->m20);
        }

        popMatrix();
    }

    static void Delete(sEdgeTask* pThis)
    {
        FunctionUnimplemented();
    }

    s16 m14C;
    s8 m14E;
    s32 m150;
    s32 m154;
    s8 m178;
    s8 m179;
    s8 m17A;
    s8 m17B;
    //size 0x17C
};

sEdgeTask* startEdgeTask(sSaturnPtr r4)
{
    return createSiblingTaskWithArgWithCopy<sEdgeTask>(allocateNPC(currentResTask, readSaturnS32(r4)), r4);
}

s32* twnEdgeVar0;

struct sMainLogic* twnMainLogicTask;

void mainLogicDummy(struct sMainLogic*)
{

}

void mainLogicUpdateSub3()
{
    FunctionUnimplemented();
}

void mainLogicInitSub2()
{
    initVDP1Projection(0x1C71C71, 0);
}

void drawLcs()
{
    if (npcData0.mFC & 0x10)
    {
        assert(0);
    }
    else
    {
        if (enableDebugTask)
        {
            assert(0);
        }
    }
}

struct sMainLogic : public s_workAreaTemplate<sMainLogic>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sMainLogic::Init, &sMainLogic::Update, &sMainLogic::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(sMainLogic* pThis)
    {
        twnMainLogicTask = pThis;

        if (mainGameState.getBit(0x274, 7))
        {
            pThis->m1 = 1;
        }
        else
        {
            pThis->m1 = 0;
        }
        pThis->m0 = 0;
        pThis->m10 = &mainLogicDummy;

        resetMatrixStack();

        pThis->mA4 = &pThis->m100;
        pThis->mA8 = &pThis->m68;
        pThis->mAC = &pThis->m0;
        pThis->mB0 = 0;
        pThis->mB4 = 0;

        mainLogicInitSub0(&pThis->m74, 0);
        mainLogicInitSub1(&pThis->m74, gTWN_RUIN->getSaturnPtr(0x605EEE4), gTWN_RUIN->getSaturnPtr(0x605EEF0));

        npcData0.mFC &= ~0x10;

        mainLogicInitSub2();
    }

    // read inputs
    static void mainLogicUpdateSub1(sMainLogic* pThis)
    {
        if (!(npcData0.mFC & 8))
        {
            if (npcData0.mFC & 0x10)
            {
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1)
                {
                    pThis->m4_flags |= 0x4000000;
                }
            }
            else
            {
                //605578C
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][2])
                {
                    pThis->m4_flags |= 0x8000000;
                }
            }
        }

        //0x60557A0
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][13])
        {
            pThis->m4_flags |= 0x20000000;
        }

        s32 r6 = 0;
        s32 r7 = 0;
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        {
            //60557BA
            assert(0);
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][4])
            {
                r6 = 0;
                r7 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][5])
            {
                r6 = 0;
                r7 = -0x10000;
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][6])
            {
                r6 = 0x10000;
                r7 = 0;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][7])
            {
                r6 = -0x10000;
                r7 = -0;
            }
        }

        //6055820
        pThis->m8_inputX = r6;
        pThis->mC_inputY = r7;
    }

    static void mainLogicUpdateSub2(sMainLogic* pThis)
    {
        if (pThis->m4_flags & 0x4000000)
        {
            assert(0);
        }
        else if (pThis->m4_flags & 0x8000000)
        {
            assert(0);
        }

        //60558AE
        if (pThis->m4_flags & 0x10000000)
        {
            assert(0);
        }

        if (pThis->m4_flags & 0x40000000)
        {
            pThis->m0 ^= 1;
        }
    }

    static void mainLogicUpdateSub4(sMainLogic* pThis)
    {
        if (pThis->m14_EdgeTask)
        {
            sNPCE8* r5 = &pThis->m14_EdgeTask->mE8;
            if (pThis->m118)
            {
                assert(0);
            }
        }
    }

    static void Update(sMainLogic* pThis)
    {
        if (pThis->m14_EdgeTask)
        {
            sEdgeTask* r13 = pThis->m14_EdgeTask;
            mainLogicUpdateSub0(r13->mE8.m0_position[0], r13->mE8.m0_position[2]);
            pThis->m18_mainRamEdgeData = r13->m0_dramAllocation;
            pThis->m1C_vdp1EdgeData = r13->m4_vd1Allocation->m0 + 0x1800;
            pThis->m20_MCBEdgeData = r13->m8_MCBInDram;
        }

        mainLogicUpdateSub1(pThis);

        mainLogicUpdateSub2(pThis);

        if (!(npcData0.mFC & 1))
        {
            mainLogicUpdateSub3();
        }

        pThis->m10(pThis);

        // Hack
        {
            if (pThis->m14_EdgeTask)
            {
                pThis->m38 = pThis->m14_EdgeTask->mE8.m0_position - sVec3_FP(0,0,0x5000);
                pThis->m44 = pThis->m14_EdgeTask->mE8.m0_position;
            }
        }

        pThis->m50 = pThis->m38;
        pThis->m50[1] += 0x10000;
        pThis->m4_flags = 0;

        mainLogicUpdateSub4(pThis);
    }

    static void Draw(sMainLogic* pThis)
    {
        sVec3_FP var18 = pThis->m38 + ((pThis->m44 - pThis->m38) >> 4);

        generateCameraMatrix(&cameraProperties2, pThis->m38, var18, pThis->m50);

        drawLcs();

        if (enableDebugTask)
        {
            assert(0);
        }
    }

    s8 m0;
    s8 m1;
    s32 m4_flags;
    s32 m8_inputX;
    s32 mC_inputY;
    void (*m10)(sMainLogic*);
    sEdgeTask* m14_EdgeTask;
    u8* m18_mainRamEdgeData;
    u8* m1C_vdp1EdgeData;
    u8* m20_MCBEdgeData;
    sVec3_FP m44;
    sVec3_FP m38;
    sVec3_FP m50;
    s32 m68;
    sMainLogic_74 m74;
    s32* mA4;
    s32* mA8;
    s8* mAC;
    s32 mB0;
    s32 mB4;
    s32 m100;
    s32 m118;
    // size 0x320
};


p_workArea startMainLogic(p_workArea pParent)
{
    return createSubTask<sMainLogic>(pParent);
}

p_workArea startCameraTask(p_workArea)
{
    return NULL;
}

s32* twnVar1;
s32 twnVar2 = 0x7FFFFFFF;

p_workArea overlayStart_TWN_RUIN(p_workArea pUntypedThis, u32 arg)
{
    // load data
    if (gTWN_RUIN == NULL)
    {
        FILE* fHandle = fopen("TWN_RUIN.PRG", "rb");
        assert(fHandle);

        fseek(fHandle, 0, SEEK_END);
        u32 fileSize = ftell(fHandle);

        fseek(fHandle, 0, SEEK_SET);
        u8* fileData = new u8[fileSize];
        fread(fileData, fileSize, 1, fHandle);
        fclose(fHandle);

        gTWN_RUIN = new TWN_RUIN_data();
        gTWN_RUIN->m_name = "TWN_RUIN.PRG";
        gTWN_RUIN->m_data = fileData;
        gTWN_RUIN->m_dataSize = fileSize;
        gTWN_RUIN->m_base = 0x6054000;

        gTWN_RUIN->init();
    }
    townDebugTask2Function* pThis = static_cast<townDebugTask2Function*>(pUntypedThis);

    pThis->m_DeleteMethod = &townOverlayDelete;

    playMusic(-1, 0);
    playMusic(59, 0);

    playPCM(pThis, 100);

    loadFnt("EVTRUIN.FNT");

    graphicEngineStatus.m405C.m10 = 0x800;
    graphicEngineStatus.m405C.m30 = FP_Div(0x10000, graphicEngineStatus.m405C.m10);

    graphicEngineStatus.m405C.m14_farClipDistance = 0xF000;
    graphicEngineStatus.m405C.m38 = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34 = graphicEngineStatus.m405C.m38 << 8;

    initDramAllocator(pThis, townBuffer, sizeof(townBuffer), listOfFilesToLoad);

    allocateVramList(pThis, townVDP1Buffer, sizeof(townVDP1Buffer));

    registerNpcs(gTWN_RUIN->getSaturnPtr(0x605E984), gTWN_RUIN->getSaturnPtr(0x06054398), arg);

    startScriptTask(pThis);

    startRuinBackgroundTask(pThis);

    sEdgeTask* pEdgeTask = startEdgeTask(gTWN_RUIN->getSaturnPtr(0x605E990));

    twnEdgeVar0 = &pEdgeTask->m84.m8;

    startMainLogic(pThis);

    twnMainLogicTask->m14_EdgeTask = pEdgeTask;

    startCameraTask(pThis);

    twnVar1 = &twnVar2;

    return pThis;
}

s32 TwnFadeOut(s32 arg0)
{
    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32(g_fadeControls.m0_fade0.m0_color), 0x8000, arg0);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32(g_fadeControls.m24_fade1.m0_color), 0x8000, arg0);
    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
    return 0;
}

s32 TwnFadeIn(s32 arg0)
{
    FunctionUnimplemented();
    return 0;
}

s32 hasLoadingCompleted()
{
    FunctionUnimplemented();
    return 1;
}

s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr)
{
    assert(ptr.m_file == gTWN_RUIN);

    switch (ptr.m_offset)
    {
    case 0x6057570: //hasLoadingCompleted
        return hasLoadingCompleted();
    case 0x6057058:
        PDS_Logger.AddLog("Unimplemented TWN_RUIN native function: 0x%08X\n", ptr.m_offset);
        break;
    default:
        assert(0);
    }
    return 0;
}

s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0)
{
    assert(ptr.m_file == gTWN_RUIN);

    switch (ptr.m_offset)
    {
    case 0x605C83C:
        return TwnFadeOut(arg0);
    case 0x0605c7c4:
        return TwnFadeIn(arg0);
    case 0x6014DF2:
        PDS_Logger.AddLog("Unimplemented TWN_RUIN native function: 0x%08X\n", ptr.m_offset);
        break;
    default:
        assert(0);
    }
    return 0;
}

s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0, s32 arg1)
{
    assert(ptr.m_file == gTWN_RUIN);

    switch (ptr.m_offset)
    {
    case 0x605B320:
        getNpcDataByIndex(arg0)->mE = arg1;
        return 0;
    case 0x605C55C:
        PDS_Logger.AddLog("Unimplemented TWN_RUIN native function: 0x%08X\n", ptr.m_offset);
        break;
    default:
        assert(0);
    }
    return 0;
}

void updateEdgeControls(sEdgeTask* r4)
{
    s32 r5 = 0;
    s32 r6 = 0;
    if (npcData0.mFC & 2)
    {
        r4->m14C |= 0xC0;
    }
    else
    {
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
        {
            //0605BC5E
            r5 = 512 * graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m2_analogX;
            r6 = 512 * graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m3_analogY;
        }
        else
        {
            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x10)
            {
                r6 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x20)
            {
                r6 = -0x10000;
            }

            if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x40)
            {
                r5 = 0x10000;
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x80)
            {
                r5 = -0x10000;
            }
        }
    }

    r4->m150 = r5;
    r4->m154 = r6;

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][0])
    {
        r4->m14C |= 2;
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & graphicEngineStatus.m4514.mD8_buttonConfig[0][1])
    {
        r4->m14C |= 1;
    }
}

void updateEdgePositionSub1(sEdgeTask* r4)
{
    if (!(r4->m14C & 0x40))
    {
        FunctionUnimplemented();
    }
    else
    {
        r4->mE8.m24_stepRotation[1] = 0;
    }

    //605BDAE
    if (r4->m14C & 0x80)
    {
        r4->mE8.m30_stepTranslation[0] = 0;
        r4->mE8.m30_stepTranslation[2] = 0;
    }
    else
    {
        FunctionUnimplemented();
    }

    //605BE7E
    r4->m14C &= 1;
    if (r4->m14C)
    {
        assert(0);
    }
}

void updateEdgePositionSub2(sNPCE8* r4)
{
    r4->mC_rotation[1] += r4->m24_stepRotation[1];
}

void updateEdgePositionSub3(sEdgeTask* r4)
{
    if (currentResTask->m8)
    {
        //605BEEA
        assert(0);
    }
    else
    {
        //0605C018
        if (npcData0.mFC & 0x10)
        {
            //605C026
            assert(0);
        }
    }

    //605C174
    fixedPoint r13 = r4->mE8.m24_stepRotation[1];
    if (r13 > 0x1C71C71)
    {
        r13 = 0x1C71C71;
    }
    if (r13 < -0x1C71C71)
    {
        r13 = -0x1C71C71;
    }

    if (r13)
    {
        r4->m20[1] += MTH_Mul(r13 - r4->m20[1], 0xB333);
    }
    else
    {
        r4->m20[1] += MTH_Mul(r13 - r4->m20[1], 0x8000);
    }

    r4->m20[0] = MTH_Mul(r4->m20[0], 0xB333);
}

void updateEdgePosition(sNPC* r4)
{
    sEdgeTask* r12 = static_cast<sEdgeTask*>(r4);
    sNPCE8* r13 = &r12->mE8;
    sMainLogic_74* r14 = &r12->m84;

    updateEdgeControls(r12);
    updateEdgePositionSub1(r12);

    *r4->m84.m30_pPosition += r4->mDC;
    if (r4->m84.m44 & 4)
    {
        assert(0);
    }
    else
    {
        //605B954
        if ((r13->m30_stepTranslation[1] < -0x199) || (r13->m30_stepTranslation[1] > 0))
        {
            FunctionUnimplemented();
        }
    }
    //605B9AA
    // gravity?
    r13->m30_stepTranslation[1] += -0x56;
    if (r13->m30_stepTranslation[1] < -0x800)
    {
        r13->m30_stepTranslation[1] = -0x800;
    }

    updateEdgePositionSub2(r13);

    sMatrix4x3 var10;
    initMatrixToIdentity(&var10);
    rotateMatrixShiftedY(r13->mC_rotation[1], &var10);
    rotateMatrixShiftedX(r13->mC_rotation[0], &var10);

    if (r14->m44 & 4)
    {
        //0605B9F0
        assert(0);
    }

    //0605BB48
    if (r14->m44 & 4)
    {
        //605BB50
        assert(0);
    }

    //605BBD6
    transformVec(r13->m30_stepTranslation, r13->m18, var10);

    PDS_warningOnce("Disable Edge position update!");
    //r13->m0_position += r13->m18;

    updateEdgePositionSub3(r12);

    r12->m14C = 0;
}
