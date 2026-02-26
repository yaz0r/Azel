#include "PDS.h"
#include "town.h"
#include "townDebugSelect.h"
#include "audio/soundDriver.h"
#include "commonOverlay.h"

struct townDebugSelect : public s_workAreaTemplate<townDebugSelect>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &townDebugSelect::Init, &townDebugSelect::Update, &townDebugSelect::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(townDebugSelect* pThis)
    {
        if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
        {
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
        }

        fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);

        fadeOutAllSequences();

        pThis->CountNumberOfSubTown();

        pThis->PrepareTownList();
    }

    static void Update(townDebugSelect* pThis)
    {
        pauseEngine[2] = 0;
        if ((graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0xE) && (pThis->m10[pThis->m0_currentSelectedTown]))
        {
            loadTownPrg(pThis->m0_currentSelectedTown, pThis->m4_columnIndex);
            pThis->getTask()->markFinished();
            return;
        }

        switch (pThis->mC_isSelectingSubTown)
        {
        case 0:
            // up
            if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.mC_newButtonDown2 & 0x10)
            {
                if (--pThis->m0_currentSelectedTown < 0)
                {
                    pThis->m0_currentSelectedTown = 16;
                }
                pThis->CountNumberOfSubTown();
            }
            // down
            else if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.mC_newButtonDown2 & 0x20)
            {
                if (++pThis->m0_currentSelectedTown > 16)
                {
                    pThis->m0_currentSelectedTown = 0;
                }
                pThis->CountNumberOfSubTown();
            }
            break;
        case 1:
            if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.mC_newButtonDown2 & 0x10) {
                if (--pThis->m4_columnIndex < 0) {
                    pThis->m4_columnIndex = pThis->m8 - 1;
                }
            }
            else if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x20) {
                if (++pThis->m4_columnIndex > pThis->m8 - 1) {
                    pThis->m4_columnIndex = 0;
                }
            }
            break;
        default:
            break;
        }

        // switch to subTown selection
        if (graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & 0xC0)
        {
            if (readSaturnEA(gCommonFile->getSaturnPtr(0x2165DC + pThis->m0_currentSelectedTown * 16)).m_offset)
            {
                if (pThis->m10[pThis->m0_currentSelectedTown])
                {
                    pThis->mC_isSelectingSubTown ^= 1;
                }
            }
        }
    }

    static void Draw(townDebugSelect* pThis)
    {
        clearVdp2TextMemory();
        for (int i = 0; i < 17; i++)
        {
            if (!pThis->m10[i])
            {
                vdp2PrintStatus.m10_palette = 0xB000;
            }
            else
            {
                if (pThis->m0_currentSelectedTown == i)
                {
                    pThis->DrawSubTowns(pThis);
                    vdp2PrintStatus.m10_palette = 0x8000;
                }
                else
                {
                    vdp2PrintStatus.m10_palette = 0xC000;
                }
            }

            vdp2DebugPrintSetPosition(2, i + 4);
            vdp2DebugPrintNewLine(readSaturnString(readSaturnEA(gCommonFile->getSaturnPtr(0x2165D4 + i * 16))));
        }

        vdp2PrintStatus.m10_palette = 0x8000;

        switch (pThis->mC_isSelectingSubTown)
        {
        case 0:
            vdp2DebugPrintSetPosition(1, pThis->m0_currentSelectedTown + 4);
            vdp2DebugPrintNewLine("\x7F");
            break;
        case 1:
        {
            fixedPoint div = performDivision(20, pThis->m4_columnIndex);
            fixedPoint x = 15 + 14 * div;
            fixedPoint y = pThis->m4_columnIndex + div * -0x14 + 4;
            vdp2DebugPrintSetPosition(x, y);
            vdp2DebugPrintNewLine("\x7F");
            break;
        }
        default:
            vdp2PrintStatus.m10_palette = 0x8000;
            break;
        }
    }

    void DrawSubTowns(townDebugSelect* pThis)
    {
        sSaturnPtr subTownData = readSaturnEA(gCommonFile->getSaturnPtr(0x2165dc + pThis->m0_currentSelectedTown * 4 * 4));
        if(!subTownData.isNull())
        {
            int i = 0;
            while (!readSaturnEA(subTownData).isNull()) {
                auto div = performDivision(0x14, i);
                vdp2DebugPrintSetPosition(div * 0xe + 0x10, i + div * -0x14 + 4);
                if (pThis->m4_columnIndex == i) {
                    vdp2PrintStatus.m10_palette = 0x8000;
                }
                else {
                    vdp2PrintStatus.m10_palette = 0xc000;
                }
                vdp2DebugPrintNewLine(readSaturnString(readSaturnEA(subTownData)));
                i++;
                subTownData += 4;
            }
        }
    }

    void CountNumberOfSubTown()
    {
        sSaturnPtr r6 = readSaturnEA(gCommonFile->getSaturnPtr(0x2165D8 + 4 + m0_currentSelectedTown * 16));
        
        if (r6.m_offset == 0)
        {
            m8 = 0;
            m4_columnIndex = 0;
            return;
        }

        s32 count = 0;
        while(readSaturnEA(r6).m_offset)
        {
            count++;
            r6 += 4;
        }

        m8 = count;
        m4_columnIndex = 0;
    }

    void PrepareTownList()
    {
        for (int i = 0; i < 17; i++)
        {
            m10.push_back(checkIfTownFileExists(i));
        }
    }

    s8 checkIfTownFileExists(s32 townIndex)
    {
        sSaturnPtr fileListEA = readSaturnEA(gCommonFile->getSaturnPtr(0x2165E0 + townIndex * 16));
        if (fileListEA.m_offset == 0)
            return 1;

        while (readSaturnEA(fileListEA).m_offset)
        {
            if (!findFileOnDisc(readSaturnString(readSaturnEA(fileListEA))))
                return 0;

            fileListEA += 4;
        }

        return 1;
    }

    //size 14
    s32 m0_currentSelectedTown;
    s32 m4_columnIndex;
    s32 m8;
    s32 mC_isSelectingSubTown;
    std::vector<s8> m10;
};

p_workArea createLocationTask(p_workArea r4, s32 r5)
{
    townDebugTask2 = createSubTaskFromFunction<townDebugTask2Function>(r4, &townDebugTask2Function::Update);
    createSubTask<townDebugSelect>(townDebugTask2);

    return townDebugTask2;
}
