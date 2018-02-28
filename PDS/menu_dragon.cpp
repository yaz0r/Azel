#include "PDS.h"

struct s_dragonMenuSubTask1WorkArea : public s_workArea
{
};

void dragonMenuSubTask1Init(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuTaskUpdate");
}

void dragonMenuSubTask1Draw(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuTaskUpdate");
}

void dragonMenuSubTask1Delete(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuTaskDelete");
}

s_taskDefinition dragonMenuSubTask1Definition = { dragonMenuSubTask1Init, NULL, dragonMenuSubTask1Draw, dragonMenuSubTask1Delete, "dragonMenuSubTask1" };

struct s_dragonMenuWorkArea : public s_workArea
{
    u32 field_0;
    u32 field_4;

    p_workArea field_C;
};

void dragonMenuTaskInit(p_workArea pTypelessWorkArea)
{
    s_dragonMenuWorkArea* pWorkArea = static_cast<s_dragonMenuWorkArea*>(pTypelessWorkArea);

    graphicEngineStatus.field_40AC.field_9 = 3;
    pWorkArea->field_4 = vblankData[5];

    createSubTask(pWorkArea, &dragonMenuSubTask1Definition, new s_dragonMenuSubTask1WorkArea);
}

void initVdp2ForDragonMenuSub1(u32 r4, u32 r5, u32 r6)
{
    assert(r4 < 4);

    u32 r7;

    if (VDP2Regs_.TVSTAT & 1)
    {
        r7 = 0;
    }
    else
    {
        r7 = 0x10;
    }

    graphicEngineStatus.field_40BC[r4].field_0 = r5;
    graphicEngineStatus.field_40BC[r4].field_2 = r7 + r6;
}

void initVdp2ForDragonMenuSub2()
{
    for (int i = 0; i < 4; i++)
    {
        graphicEngineStatus.field_40BC[i].field_8 = 0;
    }
}

void initVdp2ForDragonMenuSub3()
{
    unimplemented("initVdp2ForDragonMenuSub3");
}

void initVdp2ForDragonMenu(u32 r4)
{
    if (r4)
    {
        initVdp2ForDragonMenuSub1(0, 0, 0);
        initVdp2ForDragonMenuSub1(1, 0, 0);
        initVdp2ForDragonMenuSub1(3, 0, 0x100);
        
        initVdp2ForDragonMenuSub2();

        setupVDP2StringRendering(0, 0x22, 0x26, 0x1C);

        initVdp2ForDragonMenuSub3();
    }

    unpackGraphicsToVDP2(COMMON_DAT + 0xFE38, getVdp2Vram(0x71000));
}

void dragonMenuTaskUpdate(p_workArea pTypelessWorkArea)
{
    s_dragonMenuWorkArea* pWorkArea = static_cast<s_dragonMenuWorkArea*>(pTypelessWorkArea);

    switch (pWorkArea->field_0)
    {
    case 0:
        if (graphicEngineStatus.field_40AC.field_0 == 1)
        {
            // also init cursor
            assert(0);
        }
        else
        {
            initVdp2ForDragonMenu(1);
            pWorkArea->field_0 = 2;
        }
        break;
    case 2:
        vblankData[5] = 2;
        
        pWorkArea->field_C = createDragonMenuMorhTask(pWorkArea);

        /*
        pWorkArea->field_10 = createSubTask(pWorkArea, &dragonMenuStatsTaskDefinition, NULL);
        pWorkArea->field_14 = createSubTask(pWorkArea, &dragonMenuStatsTask2Definition, NULL);
        pWorkArea->field_1C = createSubTask(pWorkArea, &dragonMenuMorphCursorTaskDefinition, NULL);
        */

        if (graphicEngineStatus.field_40AC.field_0 != 1)
        {
            fadePalette(&menuUnk0.m_field0, 0xC210, 0xC210, 1);
            fadePalette(&menuUnk0.m_field24, 0xC210, 0xC210, 1);
        }
        pWorkArea->field_0++;
        break;
    case 3:
        if (graphicEngineStatus.field_4514.field_8 & 7)
        {
            assert(0);
        }
        break;
    default:
        assert(0);
        break;
    }
}

void dragonMenuTaskDelete(p_workArea pTypelessWorkArea)
{
    unimplemented("dragonMenuTaskDelete");
}

s_taskDefinition dragonMenuTaskDefinition = { dragonMenuTaskInit, dragonMenuTaskUpdate, NULL, dragonMenuTaskDelete, "dragon Menu" };

p_workArea createMainDragonMenuTask(p_workArea workArea)
{
    return createSubTask(workArea, &dragonMenuTaskDefinition, new s_dragonMenuWorkArea);
}
