#include "PDS.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/fileBundle.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

s_fieldTaskWorkArea* fieldTaskPtr = NULL;

u8 fieldMainBuffer[0x68000];

void initFieldMemoryArea(u8* buffer, u32 bufferSize)
{
    fieldTaskPtr->m8_pSubFieldData->m20_memoryArea_bottom = buffer;
    fieldTaskPtr->m8_pSubFieldData->m18_memoryArea_edge = buffer;
    fieldTaskPtr->m8_pSubFieldData->m24_memoryArea_top = buffer + bufferSize;
}

void initFieldVdp1Area(u32 buffer, u32 bufferSize)
{
    fieldTaskPtr->m8_pSubFieldData->m28_characterArea_bottom = buffer;
    fieldTaskPtr->m8_pSubFieldData->m1C_characterArea_edge = buffer;
    fieldTaskPtr->m8_pSubFieldData->m2C_characterArea_top = buffer + bufferSize;
}

void loadCommonFieldResources()
{
    initDramAllocator(fieldTaskPtr, playerDataMemoryBuffer, sizeof(playerDataMemoryBuffer), NULL);

    initVdp1Ram(fieldTaskPtr->m4_overlayTaskData, 0x25C18800, 0x4000);

    initFieldMemoryArea(fieldMainBuffer, sizeof(fieldMainBuffer));
    initFieldVdp1Area(0x25C1C800, 0x5F800);
}

bool findMandatoryFileOnDisc(const char* fileName)
{
    FILE* fHandle = fopen(fileName, "rb");

    if (fHandle == NULL)
        return false;

    fclose(fHandle);
    return true;
}

u32 getFileSizeFromFileId(const char* fileName)
{
    FILE* fHandle = fopen(fileName, "rb");

    if (fHandle == NULL)
        return 0;

    fseek(fHandle, 0, SEEK_END);
    u32 fileSize = ftell(fHandle);

    fclose(fHandle);
    return fileSize;
}

void checkFilesExists(const s_MCB_CGB* fileList)
{
    u32* MCBSizes = fieldTaskPtr->m8_pSubFieldData->m34_MCBFilesSizes;
    u32* CGBSizes = fieldTaskPtr->m8_pSubFieldData->m1B4_CGBFilesSizes;

    while (fileList->MCB || fileList->CGB)
    {
        assert(findMandatoryFileOnDisc(fileList->MCB));
        *(MCBSizes++) = getFileSizeFromFileId(fileList->MCB);

        assert(findMandatoryFileOnDisc(fileList->CGB));
        *(CGBSizes++) = getFileSizeFromFileId(fileList->CGB);

        fileList++;
    }
}

void setupFileList(const s_MCB_CGB* fileList)
{
    fieldTaskPtr->m8_pSubFieldData->m30_fileList = fileList;

    checkFilesExists(fileList);

    u8* buffers[3];
    buffers[0] = fieldTaskPtr->m8_pSubFieldData->m20_memoryArea_bottom;
    buffers[1] = buffers[0] + fieldTaskPtr->m8_pSubFieldData->m34_MCBFilesSizes[0]; // TODO: should be aligned
    buffers[2] = buffers[1] + fieldTaskPtr->m8_pSubFieldData->m34_MCBFilesSizes[1]; // TODO: should be aligned

    fieldTaskPtr->m8_pSubFieldData->m0_bundles[0] = new s_fileBundle(buffers[0]);
    fieldTaskPtr->m8_pSubFieldData->m0_bundles[1] = new s_fileBundle(buffers[1]);
    fieldTaskPtr->m8_pSubFieldData->m0_bundles[2] = new s_fileBundle(buffers[2]);

    fieldTaskPtr->m8_pSubFieldData->mC_characterArea[0] = fieldTaskPtr->m8_pSubFieldData->m28_characterArea_bottom;
    fieldTaskPtr->m8_pSubFieldData->mC_characterArea[1] = fieldTaskPtr->m8_pSubFieldData->mC_characterArea[0] + fieldTaskPtr->m8_pSubFieldData->m1B4_CGBFilesSizes[0]; // TODO: should be aligned
    fieldTaskPtr->m8_pSubFieldData->mC_characterArea[2] = fieldTaskPtr->m8_pSubFieldData->mC_characterArea[1] + fieldTaskPtr->m8_pSubFieldData->m1B4_CGBFilesSizes[1]; // TODO: should be aligned

    u32 currentFileIndex = 2;
    const s_MCB_CGB* currentFileList = fileList + currentFileIndex;

    u32 largestMCB = 0;
    u32 largestCGB = 0;

    while (currentFileList->MCB)
    {
        largestMCB = std::max(fieldTaskPtr->m8_pSubFieldData->m34_MCBFilesSizes[currentFileIndex], largestMCB);
        largestCGB = std::max(fieldTaskPtr->m8_pSubFieldData->m1B4_CGBFilesSizes[currentFileIndex], largestCGB);

        currentFileList++;
        currentFileIndex++;
    }

    fieldTaskPtr->m8_pSubFieldData->m18_memoryArea_edge = buffers[2] + largestMCB; // TODO: should be aligned
    fieldTaskPtr->m8_pSubFieldData->m1C_characterArea_edge = fieldTaskPtr->m8_pSubFieldData->mC_characterArea[2] + largestCGB; // TODO: should be aligned
}

s32 getFieldMemoryAreaRemain()
{
    return fieldTaskPtr->m8_pSubFieldData->m24_memoryArea_top - fieldTaskPtr->m8_pSubFieldData->m18_memoryArea_edge;
}

s32 getFieldCharacterAreaRemain()
{
    return fieldTaskPtr->m8_pSubFieldData->m2C_characterArea_top - fieldTaskPtr->m8_pSubFieldData->m1C_characterArea_edge;
}

void loadFileFromFileList(u32 index)
{
    const s_MCB_CGB* pFileData = &fieldTaskPtr->m8_pSubFieldData->m30_fileList[index];

    int slot = std::min(2, (int)index);

    if (pFileData->MCB)
    {
        loadFile(pFileData->MCB, &fieldTaskPtr->m8_pSubFieldData->m0_bundles[slot], fieldTaskPtr->m8_pSubFieldData->mC_characterArea[slot] >> 3);
    }

    if (pFileData->CGB)
    {
        loadFile(pFileData->CGB, getVdp1Pointer(fieldTaskPtr->m8_pSubFieldData->mC_characterArea[slot]), NULL);
    }
}

void getMemoryArea(s_memoryAreaOutput* pOutput, u32 areaIndex)
{
    if (areaIndex > 2)
    {
        areaIndex = 2;
    }

    pOutput->m0_mainMemoryBundle = fieldTaskPtr->m8_pSubFieldData->m0_bundles[areaIndex];
    pOutput->m4_characterArea = fieldTaskPtr->m8_pSubFieldData->mC_characterArea[areaIndex];
}

void DragonUpdateCutscene(s_dragonTaskWorkArea* r4)
{
    r4->m24A_runningCameraScript = 3;
    getFieldTaskPtr()->m28_status |= 0x10000;
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m8 |= 1;
    switch (r4->m104_dragonScriptStatus)
    {
    case 0:
        updateCameraScriptSub0(r4->mB8_lightWingEffect);
        r4->mF8_Flags &= ~0x400;
        r4->mF8_Flags |= 0x20000;
        dragonFieldTaskInitSub4Sub6(r4);
        updateCameraScriptSub0Sub2(r4);
        r4->m104_dragonScriptStatus++;
        //fall through
    case 1:
        dragonFieldTaskInitSub4Sub5(&r4->m48, &r4->m20_angle);
        copyMatrix(&r4->m48.m0_matrix, &r4->m88_matrix);
        r4->m8_pos += r4->m160_deltaTranslation;
        break;
    default:
        assert(0);
        break;
    }

    dragonFieldTaskInitSub4Sub6(r4);
}

void exitCutsceneTaskUpdateSub0Sub0()
{
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m64 = 1;
}

//kernel
void exitCutsceneTaskUpdateSub0Sub1Sub0Sub0(s32 fieldIndex)
{
    if (fieldTaskPtr->m2C_currentFieldIndex == fieldIndex)
        return;

    switch (fieldIndex)
    {
    case 3:
        mainGameState.setPackedBits(135, 6, 86);
        break;
    case 6:
        mainGameState.setPackedBits(135, 6, 99);
        break;
    case 17:
        mainGameState.setPackedBits(135, 6, 109);
        break;
    default:
        break;
    }
}

//kernel
void exitCutsceneTaskUpdateSub0Sub1Sub0Sub1(s32 fieldIndex, s32 r5, s32 r6)
{
    if (fieldTaskPtr->m2C_currentFieldIndex != fieldIndex)
    {
        fieldTaskPtr->m2E_currentSubFieldIndex = -1;
        if (fieldTaskPtr->m0)
        {
            fieldTaskPtr->m0->getTask()->markFinished();
        }
    }
    else
    {
        if (fieldTaskPtr->m8_pSubFieldData)
        {
            fieldTaskPtr->m8_pSubFieldData->getTask()->markFinished();
        }
    }

    fieldTaskPtr->m36_fieldIndexMenuSelection = fieldIndex;
    fieldTaskPtr->m38_subFieldIndexMenuSelection = r5;
    fieldTaskPtr->m3A = r6;
    fieldTaskPtr->m3C_fieldTaskState = 5;
}

// kernel
struct exitCutsceneTaskUpdateSub0Sub1Sub0 : public s_workAreaTemplate<exitCutsceneTaskUpdateSub0Sub1Sub0>
{
    static void update(exitCutsceneTaskUpdateSub0Sub1Sub0* pThis)
    {
        switch (pThis->m14_status)
        {
        case 0:
            fieldTaskPtr->m28_status |= 0x100;
            if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
            {
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0x10;
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0x10;
            }

            fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), pThis->m0, pThis->m10);
            fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), pThis->m0, pThis->m10);
            pThis->m14_status++;
            //fall through
        case 1:
            if (!g_fadeControls.m0_fade0.m20_stopped)
                return;
            exitCutsceneTaskUpdateSub0Sub1Sub0Sub0(pThis->m4_fieldIndex);
            exitCutsceneTaskUpdateSub0Sub1Sub0Sub1(pThis->m4_fieldIndex, pThis->m8, pThis->mC_exitNumber);
            pThis->m14_status++;
            break;
        case 2:
            if (fieldTaskPtr->m8_pSubFieldData)
            {
                pThis->m14_status++;
            }
            break;
        case 3:
            fieldTaskPtr->m28_status &= ~0x100;
            if (pThis)
            {
                pThis->getTask()->markFinished();
            }
            break;
        default:
            assert(0);
            break;
        }
    }

    s32 m0;
    s32 m4_fieldIndex;
    s32 m8;
    s32 mC_exitNumber;
    s32 m10;
    s32 m14_status;
    //size 18
};

// kernel
void exitCutsceneTaskUpdateSub0Sub1(s32 fieldIndex, s32 param, s32 exitNumber, s32 r7)
{
    if (param < 0)
    {
        dispatchTutorialMultiChoiceSub2();
    }
    else
    {
        exitCutsceneTaskUpdateSub0Sub1Sub0* pNewTask = createSubTaskFromFunction<exitCutsceneTaskUpdateSub0Sub1Sub0>(fieldTaskPtr, &exitCutsceneTaskUpdateSub0Sub1Sub0::update);
        pNewTask->m0 = r7;
        pNewTask->m4_fieldIndex = fieldIndex;
        pNewTask->m8 = param;
        pNewTask->mC_exitNumber = exitNumber;
        pNewTask->m10 = 20;
    }
}

void exitCutsceneTaskUpdateSub0(s32 param, s32 exitNumber, s16 r6)
{
    exitCutsceneTaskUpdateSub0Sub0();
    exitCutsceneTaskUpdateSub0Sub1(getFieldTaskPtr()->m2C_currentFieldIndex, param, exitNumber, r6);
}

struct s_exitCutsceneTask : public s_workAreaTemplate<s_exitCutsceneTask>
{
    static void Update(s_exitCutsceneTask* pThis)
    {
        s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

        switch (pThis->m16_state)
        {
        case 0:
            getFieldCameraStatus()->m0_position = pThis->m0_pScript->m0_position;
            dragonFieldTaskInitSub4Sub3(0);
            pThis->m10_length = pThis->m0_pScript->m20_length;
            r14->m8_pos[1] = pThis->m0_pScript->m18;
            r14->m20_angle = pThis->m0_pScript->mC_rotation;

            r14->m160_deltaTranslation[0] = MTH_Mul(-pThis->m0_pScript->m1C, getSin(r14->m20_angle[1].getInteger() & 0xFFF));
            r14->m160_deltaTranslation[1] = performDivision(pThis->m10_length, pThis->m0_pScript->m0_position[1] - r14->m8_pos[1]);
            r14->m160_deltaTranslation[2] = MTH_Mul(-pThis->m0_pScript->m1C, getCos(r14->m20_angle[1].getInteger() & 0xFFF));

            r14->m8_pos[0] = pThis->m0_pScript->m0_position[0] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[0]);
            r14->m8_pos[1] = pThis->m0_pScript->m0_position[1] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[1]);
            r14->m8_pos[2] = pThis->m0_pScript->m0_position[2] - ((pThis->m10_length + 5) * r14->m160_deltaTranslation[2]);

            r14->mF4 = DragonUpdateCutscene;
            r14->m108 = 0;
            r14->mF4(r14);

            pThis->m16_state++;
            break;
        case 1:
            if (--pThis->m10_length <= 0)
            {
                exitCutsceneTaskUpdateSub0(pThis->m8_param, pThis->mC_exitNumber, pThis->m14);
                pThis->m16_state++;
            }
        case 2:
            break;
        default:
            assert(0);
            break;
        }
    }

    // size 0x18
    s_cameraScript* m0_pScript;
    s32 m8_param;
    s32 mC_exitNumber;
    s32 m10_length;
    s32 m14;
    s8 m16_state;
};

void startExitFieldCutscene(p_workArea parent, s_cameraScript* pScript, s32 param, s32 exitIndex, s32 arg0)
{
    s_exitCutsceneTask* pNewTask = createSubTaskFromFunction<s_exitCutsceneTask>(parent, &s_exitCutsceneTask::Update);

    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    pNewTask->m0_pScript = pScript;
    pNewTask->m8_param = param;
    pNewTask->mC_exitNumber = exitIndex;
    pNewTask->m14 = arg0;

    
    pDragonTask->m1D0_cameraScript = pScript;
    pDragonTask->mF8_Flags &= ~0x400;
}

void startExitFieldCutscene2Sub0(p_workArea parent, s_cutsceneData* pScript, s32 fieldIndex, s32 param, s32 exitIndex, s32 arg0)
{
    s_cutsceneTask* pCutsceneTask = createSubTaskWithArg<s_cutsceneTask>(getFieldTaskPtr()->m8_pSubFieldData, pScript);
    pCutsceneTask->m4_changeField = 1;
    pCutsceneTask->m8_fieldIndex = fieldIndex;
    pCutsceneTask->mC_fieldParam = param;
    pCutsceneTask->m10_fieldExitIndex = exitIndex;
    pCutsceneTask->m14 = arg0;
}

void startExitFieldCutscene2(p_workArea parent, s_cutsceneData* pScript, s32 param, s32 exitIndex, s32 arg0)
{
    startExitFieldCutscene2Sub0(parent, pScript, getFieldTaskPtr()->m2C_currentFieldIndex, param, exitIndex, arg0);
}

void initFieldDragonLight()
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragonTask->mC8_normalLightColor.m0 = 0x10;
    pDragonTask->mC8_normalLightColor.m1 = 0x10;
    pDragonTask->mC8_normalLightColor.m2 = 0x10;

    pDragonTask->mCB_falloffColor0.m0 = 9;
    pDragonTask->mCB_falloffColor0.m1 = 9;
    pDragonTask->mCB_falloffColor0.m2 = 9;

    pDragonTask->mCE_falloffColor1.m0 = -5;
    pDragonTask->mCE_falloffColor1.m1 = -5;
    pDragonTask->mCE_falloffColor1.m2 = -5;

    pDragonTask->mD1_falloffColor2.m0 = 0x19;
    pDragonTask->mD1_falloffColor2.m1 = 0x19;
    pDragonTask->mD1_falloffColor2.m2 = 0x19;

    pDragonTask->mC0_lightRotationAroundDragon = 0xC000000;
}

void gridCellDraw_normalSub2(p_workArea r4, s32 r5, s32 r6)
{
    FunctionUnimplemented();
}

void callGridCellDraw_normalSub2(p_workArea r4, s32 r5)
{
    gridCellDraw_normalSub2(r4, r5, 0x10000);
}

s_RGB8 readSaturnRGB8(const sSaturnPtr& ptr)
{
    s_RGB8 newValue;
    newValue.m0 = readSaturnU8(ptr + 0);
    newValue.m1 = readSaturnU8(ptr + 1);
    newValue.m2 = readSaturnU8(ptr + 2);

    return newValue;
}
