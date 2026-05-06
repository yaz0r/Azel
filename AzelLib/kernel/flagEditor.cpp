#include "PDS.h"
#include "flagEditor.h"
#include "menu_dragon.h"

#ifndef SHIPPING_BUILD
#include "imgui.h"
#include "debugWindows.h"

struct sFlagEntry
{
    const char* name;
    const char* category;
};

static const sFlagEntry s_flagTranslations[] = {
    // Page 01: Plot Advance Flags 000-040
    {"Edge wounded", "Plot"},
    {"Edge regains consciousness", "Plot"},
    {"Dragon appears", "Plot"},
    {"Go to the Excavation Site", "Plot"},
    {"The captain's last will", "Plot"},
    {"Gash found on the run", "Plot"},
    {"Meeting up with Gash", "Plot"},
    {"Gash tells the legend of the dragon", "Plot"},
    {"Farewell to Gash", "Plot"},
    {"Rumors of the Craymen fleet", "Plot"},
    {"Arwen defeated ~ Atolm Dragon appears", "Plot"},
    {"A request from Paet", "Plot"},
    {"Seized by the Empire", "Plot"},
    // Page 02: Plot Advance Flags 041-072
    {"Interrogation by the Imperial Army", "Plot"},
    {"Seeker attack", "Plot"},
    {"Reunion with Gash", "Plot"},
    {"Negotiations with Paet", "Plot"},
    {"X-ray footage", "Plot"},
    {"Lieutenant Zastava appears", "Plot"},
    {"Atolm Dragon appears 2", "Plot"},
    {"To the underground of Ur", "Plot"},
    {"Meeting Azel", "Plot"},
    {"Getting on the elevator", "Plot"},
    {"Azel's monologue", "Plot"},
    {"Premonition of a reunion", "Plot"},
    {"To the sky with Paet", "Plot"},
    // Page 03: Plot Advance Flags 074-128
    {"Advance party from the Empire", "Plot"},
    {"A request from Baiman", "Plot"},
    {"Baiman's conspiracy", "Plot"},
    {"Late-night attack by Craymen's subordinates", "Plot"},
    {"Mel-Kava is seen", "Plot"},
    {"What the legacy showed", "Plot"},
    {"Edge saves Azel / Imperial Army appears", "Plot"},
    {"The city is destroyed", "Plot"},
    {"Craymen's ship appears (first half)", "Plot"},
    {"Craymen's ship appears (second half)", "Plot"},
    {"Craymen's confession", "Plot"},
    {"Azel's awakening / Craymen's death", "Plot"},
    {"Aggressive creatures vs. Imperial Army", "Plot"},
    // Page 04: Plot Advance Flags 134-155
    {"The Seeker's true identity", "Plot"},
    {"Azel awakens", "Plot"},
    {"Imperial Army attacks", "Plot"},
    {"Azel persuades", "Plot"},
    {"Azel makes a decision", "Plot"},
    {"Re-entering the tower", "Plot"},
    {"Edge and Azel part ways", "Plot"},
    {"To Sestren, the tower explodes", "Plot"},
    {"Battle movie 1", "Plot"},
    {"Battle movie 2", "Plot"},
    {"Battle movie 3", "Plot"},
    {"Battle movie 4", "Plot"},
    {"Battle movie 5", "Plot"},
    // Page 05: Plot + Areas cleared
    {"Farewell to the Dragon", "Plot"},
    {"Gash waits for Edge in the wilderness", "Plot"},
    {"Cleared skies above excavation site", "Area Clear"},
    {"Cleared valley area", "Area Clear"},
    {"Cleared Galil Desert", "Area Clear"},
    {"Cleared restricted area", "Area Clear"},
    {"Cleared Georgios 2", "Area Clear"},
    {"Cleared Georgios 3", "Area Clear"},
    {"Cleared Ur", "Area Clear"},
    {"Cleared Ur Underground Ruins", "Area Clear"},
    {"Cleared Imperial Air Fleet Garrison", "Area Clear"},
    {"Cleared Mel-Kava", "Area Clear"},
    {"Cleared Imperial Air Fleet", "Area Clear"},
    // Page 06: Areas cleared + Map targets
    {"Cleared Tower 1", "Area Clear"},
    {"Cleared Zoa Forest", "Area Clear"},
    {"Cleared Underground of Ur 2", "Area Clear"},
    {"Cleared Tower 2", "Area Clear"},
    {"Cleared Sestren", "Area Clear"},
    {"Can enter air above excavation site", "Map"},
    {"Can enter excavation site", "Map"},
    {"Can enter camp (excavation site)", "Map"},
    {"Can enter valley area", "Map"},
    {"Can enter Kainas Village", "Map"},
    {"Can enter camp (Kainas Village)", "Map"},
    {"Can enter Galil Desert", "Map"},
    {"Can enter Caravan (Desert A)", "Map"},
    // Page 07: Map targets
    {"Camp (Desert A) allowed", "Map"},
    {"Caravan (Desert B) allowed", "Map"},
    {"Camp (Desert B) allowed", "Map"},
    {"Caravan (Plains A) allowed", "Map"},
    {"Camp (Plains) allowed", "Map"},
    {"Caravan (Plains B) allowed", "Map"},
    {"Caravan (Zoa Town) allowed", "Map"},
    {"Camp (Zia Forest) allowed", "Map"},
    {"Caravan (Zia Forest) allowed", "Map"},
    {"Restricted area allowed", "Map"},
    {"Georgios 2 allowed", "Map"},
    {"Georgios 2.5 allowed", "Map"},
    {"Georgios 3 allowed", "Map"},
    // Page 08: Map targets
    {"Can enter Zoa City", "Map"},
    {"Imperial Air Fleet", "Map"},
    {"Can enter Imperial Air Fleet Garrison", "Map"},
    {"Can enter Mel-Kava", "Map"},
    {"Can enter Tower 1", "Map"},
    {"Can enter Tower 2", "Map"},
    {"Can enter Ur", "Map"},
    {"Can enter Ur Underground Ruins", "Map"},
    {"Can enter Zoa Forest", "Map"},
    {"Can enter Seeker Village", "Map"},
    {"Can enter Camp (Seeker Village)", "Map"},
    {"Can enter Sestren", "Map"},
    {"Player's location on the map", "Map"},
    // Page 09: Bosses cleared
    {"Valley mid-boss (Snake Bug Nest A)", "Boss"},
    {"Valley mid-boss (Snake Bug Nest B)", "Boss"},
    {"Valley boss (Gakeokori)", "Boss"},
    {"Galil Desert mid-boss A (Zusuu)", "Boss"},
    {"Galil Desert boss (Haish & Haiga)", "Boss"},
    {"Restricted Zone mid-boss (Craymen's Ship)", "Boss"},
    {"Restricted Zone boss (AD#1)", "Boss"},
    {"Georgios 2 mid-boss (Varuna)", "Boss"},
    {"Georgios 2 boss (Guardian Dragon)", "Boss"},
    {"Ur Consecutive Battle 1", "Boss"},
    {"Ur Consecutive Battle 2", "Boss"},
    {"Ur Consecutive Battle 3", "Boss"},
    {"Ur mid-boss (Nurisinha)", "Boss"},
    // Page 10: Bosses cleared
    {"Ur boss (AD#2)", "Boss"},
    {"Ur Ruins mid-boss (Miyabiray)", "Boss"},
    {"Ur Ruins boss (Daioubiray)", "Boss"},
    {"Air Fleet Garrison mid-boss (Vritra)", "Boss"},
    {"Air Fleet Garrison boss (Rocket Missile)", "Boss"},
    {"Mel-Kava mid-boss", "Boss"},
    {"Mel-Kava boss (AD#3)", "Boss"},
    {"Air Fleet mid-boss (Griguorigu)", "Boss"},
    {"Imperial Army boss (Soma)", "Boss"},
    {"Tower 1 boss (Bariou)", "Boss"},
    {"Zoa Forest mid-boss (Masan A)", "Boss"},
    {"Zoa Forest mid-boss (Masan B)", "Boss"},
    {"Zoa Forest mid-boss (Masan C)", "Boss"},
    // Page 11: Bosses cleared
    {"Zoa Forest mid-boss (Masan D)", "Boss"},
    {"Zoa Forest mid-boss (Masan E)", "Boss"},
    {"Zoa Forest mid-boss (Masan F)", "Boss"},
    {"Zoa Forest mid-boss (Masan G)", "Boss"},
    {"Zoa Forest mid-boss (Masan H)", "Boss"},
    {"Zoa Forest boss (Parasitic Griguorigu)", "Boss"},
    {"Ur Underground weak enemy (Offensive creature)", "Boss"},
    {"Tower 2 mid-boss A", "Boss"},
    {"Tower 2 mid-boss B", "Boss"},
    {"Sestren mid-boss (Type R)", "Boss"},
    {"Sestren mid-boss (Type 1)", "Boss"},
    {"Sestren mid-boss (Type 2)", "Boss"},
    {"Sestren mid-boss (Type 3)", "Boss"},
    // Page 12: Bosses + Items
    {"Sestren mid-boss (Type 4)", "Boss"},
    {"Sestren boss (Anti-Dragon)", "Boss"},
    {"Sestren boss (Anti-Dragon final form)", "Boss"},
    {"Zoa Forest mid-boss A weakened", "Boss"},
    {"Zoa Forest mid-boss B weakened", "Boss"},
    {"Zoa Forest mid-boss C weakened", "Boss"},
    {"Zoa Forest mid-boss D weakened", "Boss"},
    {"Zoa Forest mid-boss E weakened", "Boss"},
    {"Zoa Forest mid-boss F weakened", "Boss"},
    {"Zoa Forest mid-boss G weakened", "Boss"},
    {"Zoa Forest mid-boss H weakened", "Boss"},
    {"Obtain gun", "Item"},
    {"Obtain key to Dragon Ruins", "Item"},
    // Page 13: Items
    {"Obtain map", "Item"},
    {"Obtain tower painting", "Item"},
    {"Obtain ship parts", "Item"},
    {"Obtain engine unit", "Item"},
    {"Obtain Protect Key 1", "Item"},
    {"Obtain Protect Key 2", "Item"},
    {"Obtain Protect Key 3", "Item"},
    {"Obtain key to great hall", "Item"},
    {"Obtain permit to sanctuary", "Item"},
    {"Church key", "Item"},
    {"Obtain letter from Craymen", "Item"},
    {"Searchlight count flag", "Misc"},
    {"For disc change", "Misc"},
    // Page 14: Misc
    {"Game cleared (Dermot's Ranch)", "Misc"},
    {"Forest mini-boss explanation", "Misc"},
    {"Core barrier explanation", "Misc"},
    {"Garrison book dialogue change", "Misc"},
    {"Secret flag (M Takaha C)", "Misc"},
    {"Named the dragon", "Misc"},
    {"Player map position (saved)", "Misc"},
};

static const int FLAG_TRANSLATION_COUNT = sizeof(s_flagTranslations) / sizeof(s_flagTranslations[0]);

struct sFlagEditorTask : public s_workAreaTemplate<sFlagEditorTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { nullptr, nullptr, &sFlagEditorTask::Draw, nullptr };
        return &taskDefinition;
    }

    static void Draw(sFlagEditorTask* pThis)
    {
        if (!pThis->m0_visible)
            return;

        ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Flag Editor", &pThis->m0_visible))
        {
            ImGui::End();
            return;
        }

        ImGui::Text("Game State Flags (%d bytes = %d bits)", (int)sizeof(mainGameState.bitField), (int)sizeof(mainGameState.bitField) * 8);
        ImGui::Separator();

        static char searchBuf[64] = "";
        ImGui::InputText("Search byte index", searchBuf, sizeof(searchBuf));
        int searchByte = -1;
        if (searchBuf[0])
            searchByte = strtol(searchBuf, nullptr, 0);

        ImGui::Separator();

        if (ImGui::BeginTabBar("FlagTabs"))
        {
            if (ImGui::BeginTabItem("Named Flags"))
            {
                drawNamedFlags();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Byte View"))
            {
                drawByteView(searchByte);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Bit View"))
            {
                drawBitView(searchByte);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Game Stats"))
            {
                drawGameStats();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    static void drawNamedFlags()
    {
        static int categoryFilter = 0;
        const char* categories[] = { "All", "Plot", "Area Clear", "Map", "Boss", "Item", "Misc" };
        ImGui::Combo("Category", &categoryFilter, categories, IM_ARRAYSIZE(categories));
        ImGui::Separator();

        ImGui::BeginChild("NamedScroll", ImVec2(0, 0), true);
        for (int i = 0; i < FLAG_TRANSLATION_COUNT; i++)
        {
            if (categoryFilter > 0 && strcmp(s_flagTranslations[i].category, categories[categoryFilter]) != 0)
                continue;

            int bitOffset = 0x20 + i;
            bool val = mainGameState.getBit(bitOffset);

            ImGui::PushID(i);
            char label[128];
            snprintf(label, sizeof(label), "[%3d] %s", i, s_flagTranslations[i].name);
            if (ImGui::Checkbox(label, &val))
            {
                if (val)
                    mainGameState.setBit(bitOffset);
                else
                    mainGameState.clearBit(bitOffset);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Bit %d (byte 0x%03X bit %d) | Category: %s",
                    bitOffset, bitOffset / 8, 7 - (bitOffset % 8), s_flagTranslations[i].category);
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    static void drawByteView(int searchByte)
    {
        ImGui::BeginChild("ByteScroll", ImVec2(0, 0), true);
        for (int i = 0; i < (int)sizeof(mainGameState.bitField); i++)
        {
            if (searchByte >= 0 && i != searchByte)
                continue;

            u8 val = mainGameState.bitField[i];
            if (val == 0 && searchByte < 0)
                continue;

            ImGui::PushID(i);
            ImGui::Text("0x%03X [%3d]:", i, i);
            ImGui::SameLine();

            for (int bit = 7; bit >= 0; bit--)
            {
                bool b = (val >> bit) & 1;
                ImGui::SameLine();
                char label[8];
                snprintf(label, sizeof(label), "%d##%d", bit, bit);
                if (ImGui::Checkbox(label, &b))
                {
                    if (b)
                        mainGameState.bitField[i] |= (1 << bit);
                    else
                        mainGameState.bitField[i] &= ~(1 << bit);
                }
            }

            ImGui::SameLine();
            ImGui::Text(" = 0x%02X", mainGameState.bitField[i]);
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    static void drawBitView(int searchByte)
    {
        ImGui::BeginChild("BitScroll", ImVec2(0, 0), true);
        int totalBits = (int)sizeof(mainGameState.bitField) * 8;
        int startBit = searchByte >= 0 ? searchByte * 8 : 0;
        int endBit = searchByte >= 0 ? startBit + 8 : totalBits;

        for (int bitIdx = startBit; bitIdx < endBit; bitIdx++)
        {
            bool val = mainGameState.getBit(bitIdx);
            if (!val && searchByte < 0)
                continue;

            ImGui::PushID(bitIdx);
            char label[32];
            snprintf(label, sizeof(label), "Bit %4d (byte 0x%03X bit %d)", bitIdx, bitIdx / 8, 7 - (bitIdx % 8));
            if (ImGui::Checkbox(label, &val))
            {
                if (val)
                    mainGameState.setBit(bitIdx);
                else
                    mainGameState.clearBit(bitIdx);
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    static void drawGameStats()
    {
        auto& gs = mainGameState.gameStats;

        ImGui::Text("Dragon Level: %d", gs.m1_dragonLevel);
        ImGui::Text("Rider 1: %d  Rider 2: %d", gs.m2_rider1, gs.m3_rider2);
        ImGui::Separator();

        int hp = gs.m10_currentHP;
        if (ImGui::InputInt("Current HP", &hp)) gs.m10_currentHP = (s16)hp;
        int maxHp = gs.mB8_maxHP;
        if (ImGui::InputInt("Max HP", &maxHp)) gs.mB8_maxHP = (s16)maxHp;

        int bp = gs.m14_currentBP;
        if (ImGui::InputInt("Current BP", &bp)) gs.m14_currentBP = (s16)bp;
        int maxBp = gs.mBA_maxBP;
        if (ImGui::InputInt("Max BP", &maxBp)) gs.mBA_maxBP = (s16)maxBp;

        ImGui::Separator();
        int xp = (int)gs.m20_XP;
        if (ImGui::InputInt("XP", &xp)) gs.m20_XP = (u32)xp;
        int dyne = (int)gs.m38_dyne;
        if (ImGui::InputInt("Dyne", &dyne)) gs.m38_dyne = (u32)dyne;

        ImGui::Separator();
        int att = gs.mBE_dragonAtt;
        if (ImGui::InputInt("Attack", &att)) gs.mBE_dragonAtt = (s16)att;
        int def = gs.mBC_dragonDef;
        if (ImGui::InputInt("Defense", &def)) gs.mBC_dragonDef = (s16)def;
        int agl = gs.mC0_dragonAgl;
        if (ImGui::InputInt("Agility", &agl)) gs.mC0_dragonAgl = (s16)agl;
        int spr = gs.mC2_dragonSpr;
        if (ImGui::InputInt("Spiritual", &spr)) gs.mC2_dragonSpr = (s16)spr;
    }

    bool m0_visible = true;
};

void createFlagEditorTask(p_workArea parent)
{
    sFlagEditorTask* pTask = createSubTask<sFlagEditorTask>(parent);
    pTask->m0_visible = true;
}

void drawFlagEditorWindow()
{
    if (!gDebugWindows.flagEditor)
        return;
    // Reuse the static draw methods from sFlagEditorTask
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Flag Editor", &gDebugWindows.flagEditor))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("Game State Flags (%d bytes = %d bits)", (int)sizeof(mainGameState.bitField), (int)sizeof(mainGameState.bitField) * 8);
    ImGui::Separator();
    static char searchBuf[64] = "";
    ImGui::InputText("Search byte index", searchBuf, sizeof(searchBuf));
    int searchByte = -1;
    if (searchBuf[0])
        searchByte = strtol(searchBuf, nullptr, 0);
    ImGui::Separator();
    if (ImGui::BeginTabBar("FlagTabs"))
    {
        if (ImGui::BeginTabItem("Named Flags"))
        {
            sFlagEditorTask::drawNamedFlags();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Byte View"))
        {
            sFlagEditorTask::drawByteView(searchByte);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Game Stats"))
        {
            sFlagEditorTask::drawGameStats();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

#else

void createFlagEditorTask(p_workArea parent) {}
void drawFlagEditorWindow() {}

#endif

// Native Saturn-style flag editor using VDP2 text
// Loads FLAGEDIT.PRG as a data file and reads the flag table from it

static const u32 FLAG_TABLE_OFFSET = 0x1370; // 0x06055370 - 0x06054000
static const u32 FLAG_ENTRY_SIZE = 8;
static const u32 FLAG_COUNT = 176; // 0xB0 entries (0 to 0xAF)
static const u32 VISIBLE_ROWS = 13;

struct sNativeFlagEditor : public s_workAreaTemplate<sNativeFlagEditor>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {
            &sNativeFlagEditor::Init,
            nullptr,
            &sNativeFlagEditor::Draw,
            &sNativeFlagEditor::Delete
        };
        return &taskDefinition;
    }

    // 06054008
    static void Init(sNativeFlagEditor* pThis)
    {
        pThis->m_dirty = 2;
        pThis->m_overlayFile = new sSaturnMemoryFile("FLAGEDIT.PRG");
        loadFnt("FLAGEDIT.FNT");
    }

    // 0605403e
    static void Draw(sNativeFlagEditor* pThis)
    {
        // Up arrow (0x109)
        if (readKeyboardTable1(0x109) && pThis->m_cursor > 0)
        {
            pThis->m_dirty = 1;
            pThis->m_cursor--;
            if (pThis->m_cursor < pThis->m_scrollTop)
            {
                pThis->m_dirty = 2;
                pThis->m_scrollTop = pThis->m_cursor;
            }
        }

        // Down arrow (0x10a)
        if (readKeyboardTable1(0x10a) && pThis->m_cursor < (s32)(FLAG_COUNT - 1))
        {
            pThis->m_dirty = 1;
            pThis->m_cursor++;
            if (pThis->m_cursor >= pThis->m_scrollTop + (s32)VISIBLE_ROWS)
            {
                pThis->m_dirty = 2;
                pThis->m_scrollTop = pThis->m_cursor - (s32)VISIBLE_ROWS + 1;
            }
        }

        // Page up (0x10b)
        if (readKeyboardTable1(0x10b) && pThis->m_scrollTop > 0)
        {
            pThis->m_dirty = 2;
            s32 oldTop = pThis->m_scrollTop;
            pThis->m_scrollTop -= (s32)VISIBLE_ROWS;
            if (pThis->m_scrollTop < 0) pThis->m_scrollTop = 0;
            pThis->m_cursor += pThis->m_scrollTop - oldTop;
        }

        // Page down (0x10c)
        if (readKeyboardTable1(0x10c) && pThis->m_scrollTop < (s32)(FLAG_COUNT - VISIBLE_ROWS))
        {
            pThis->m_dirty = 2;
            s32 oldTop = pThis->m_scrollTop;
            pThis->m_scrollTop += (s32)VISIBLE_ROWS;
            if (pThis->m_scrollTop > (s32)(FLAG_COUNT - VISIBLE_ROWS))
                pThis->m_scrollTop = (s32)(FLAG_COUNT - VISIBLE_ROWS);
            pThis->m_cursor += pThis->m_scrollTop - oldTop;
        }

        // Right arrow (0x10d) — increment value
        if (readKeyboardTable1(0x10d))
        {
            pThis->m_dirty = 1;
            incrementFlag(pThis, 1);
        }

        // Left arrow (0x106) — decrement value
        if (readKeyboardTable1(0x106))
        {
            pThis->m_dirty = 1;
            incrementFlag(pThis, -1);
        }

        if (pThis->m_dirty)
        {
            if (pThis->m_dirty == 2)
                clearVdp2TextArea();

            drawFlags(pThis);
            pThis->m_dirty = 0;
        }
    }

    static void Delete(sNativeFlagEditor* pThis)
    {
        delete pThis->m_overlayFile;
        pThis->m_overlayFile = nullptr;
    }

    static void incrementFlag(sNativeFlagEditor* pThis, s32 delta)
    {
        sSaturnPtr entry = pThis->m_overlayFile->getSaturnPtr(0x06054000 + FLAG_TABLE_OFFSET + pThis->m_cursor * FLAG_ENTRY_SIZE);
        s16 bitOffset = readSaturnS16(entry);
        s16 bitCount = readSaturnS16(entry + 2);

        s32 value = mainGameState.readPackedBits(bitOffset, bitCount);
        u32 mask = (1 << bitCount) - 1;
        value = (value + delta) & mask;
        mainGameState.setPackedBits(bitOffset, bitCount, value);
    }

    static void drawFlags(sNativeFlagEditor* pThis)
    {
        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        vdp2PrintStatus.m10_palette = 0xC000;

        for (s32 row = 0; row < (s32)VISIBLE_ROWS; row++)
        {
            s32 flagIdx = pThis->m_scrollTop + row;
            if (flagIdx >= (s32)FLAG_COUNT) break;

            bool isSelected = (flagIdx == pThis->m_cursor);
            if (isSelected)
                vdp2PrintStatus.m10_palette = 0xB000;

            sSaturnPtr entry = pThis->m_overlayFile->getSaturnPtr(0x06054000 + FLAG_TABLE_OFFSET + flagIdx * FLAG_ENTRY_SIZE);
            s16 bitOffset = readSaturnS16(entry);
            s16 bitCount = readSaturnS16(entry + 2);
            sSaturnPtr namePtr = readSaturnEA(entry + 4);

            s32 value = mainGameState.readPackedBits(bitOffset, bitCount);

            vdp2DebugPrintSetPosition(2, row * 2 + 0x21);
            vdp2PrintfLargeFont("%3d", value);
            vdp2DebugPrintSetPosition(8, row * 2 + 0x21);
            drawObjectName(readSaturnString(namePtr).c_str());

            if (isSelected)
                vdp2PrintStatus.m10_palette = 0xC000;
        }

        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
    }

    sSaturnMemoryFile* m_overlayFile = nullptr;
    s32 m_cursor = 0;
    s32 m_scrollTop = 0;
    s32 m_dirty = 0;
};

void createNativeFlagEditorTask(p_workArea parent)
{
    createSubTask<sNativeFlagEditor>(parent);
}

#ifndef SHIPPING_BUILD
#include "kernel/moduleManager.h"
#include "debugWindows.h"

struct sGameStatusEntry
{
    s32 id;
    const char* name;
};

static const sGameStatusEntry s_gameStatusEntries[] = {
    { 0x00, "00: Boot" },
    { 0x01, "01: Intro Movie (Edge wounded)" },
    { 0x02, "02: Opening Movie" },
    { 0x03, "03: Name Entry" },
    { 0x04, "04: Resurrection Movie" },
    { 0x05, "05: Excavation Site (Ruins)" },
    { 0x06, "06: Excavation Site Battle 1" },
    { 0x07, "07: Excavation Site Boss (Baldor Queen)" },
    { 0x08, "08: Post-Excavation Movie" },
    { 0x09, "09: Valley (Field A3)" },
    { 0x0A, "0A: Valley Battle" },
    { 0x0B, "0B: Valley Boss" },
    { 0x0C, "0C: Camp (Excavation)" },
    { 0x0D, "0D: Craymen Fleet Movie" },
    { 0x0E, "0E: Craymen Fleet Battle" },
    { 0x0F, "0F: Craymen Fleet Boss (AD#1)" },
    { 0x10, "10: Georgius 2" },
    { 0x11, "11: Georgius 2 Battle" },
    { 0x12, "12: Georgius 2 Boss (Guardian Dragon)" },
    { 0x13, "13: Camp (Kainas)" },
    { 0x14, "14: Kainas Village (Zoah)" },
    { 0x15, "15: Galil Desert" },
    { 0x16, "16: Galil Desert Battle" },
    { 0x17, "17: Galil Desert Boss" },
    { 0x18, "18: Caravan" },
    { 0x19, "19: Movie (Paet)" },
    { 0x1A, "1A: Ur" },
    { 0x1B, "1B: Ur Battle" },
    { 0x1C, "1C: Ur Boss (AD#2)" },
    { 0x1D, "1D: Ur Underground Ruins" },
    { 0x1E, "1E: Ur Underground Battle" },
    { 0x1F, "1F: Ur Underground Boss" },
    { 0x20, "20: Imperial Air Fleet Garrison" },
    { 0x21, "21: Garrison Battle" },
    { 0x22, "22: Garrison Boss" },
    { 0x23, "23: Mel-Kava" },
    { 0x24, "24: Mel-Kava Battle" },
    { 0x25, "25: Mel-Kava Boss (AD#3)" },
    { 0x26, "26: Imperial Air Fleet" },
    { 0x27, "27: Air Fleet Battle" },
    { 0x28, "28: Air Fleet Boss (Soma)" },
    { 0x29, "29: Camp (Seeker Village)" },
    { 0x2A, "2A: Seeker Village" },
    { 0x2B, "2B: Movie (Azel awakens)" },
    { 0x2C, "2C: Tower 1" },
    { 0x2D, "2D: Tower 1 Battle" },
    { 0x2E, "2E: Tower 1 Boss (Bariou)" },
    { 0x2F, "2F: Zoa Forest" },
    { 0x30, "30: Zoa Forest Battle (Masan)" },
    { 0x31, "31: Zoa Forest Boss (Parasitic Griguorigu)" },
    { 0x32, "32: Ur Underground 2" },
    { 0x33, "33: Ur Underground 2 Battle" },
    { 0x34, "34: Tower 2" },
    { 0x35, "35: Tower 2 Battle" },
    { 0x36, "36: Movie (Craymen)" },
    { 0x37, "37: Movie (Azel awakening / Craymen death)" },
    { 0x38, "38: Sestren" },
    { 0x39, "39: Sestren Battle (Type R)" },
    { 0x3A, "3A: Sestren Battle (Type 1)" },
    { 0x3B, "3B: Sestren Battle (Type 2)" },
    { 0x3C, "3C: Sestren Battle (Type 3)" },
    { 0x3D, "3D: Sestren Battle (Type 4)" },
    { 0x3E, "3E: Sestren Boss (Anti-Dragon)" },
    { 0x3F, "3F: Sestren Boss (Anti-Dragon Final)" },
    { 0x40, "40: Movie (Ending 1)" },
    { 0x41, "41: Movie (Ending 2)" },
    { 0x42, "42: Movie (Ending 3)" },
    { 0x43, "43: Movie (Credits)" },
    { 0x44, "44: Movie (Post-Credits)" },
    { 0x4A, "4A: Load Savegame" },
    { 0x4F, "4F: Return to Title" },
    { 0x50, "50: Worldmap (after Excavation)" },
    { 0x51, "51: Worldmap (after Captain)" },
    { 0x53, "53: Worldmap (after Gash)" },
    { 0x66, "66: Movie (Battle)" },
    { 0x70, "70: Game Over" },
    { 0x71, "71: Tutorial 1" },
    { 0x72, "72: Tutorial 2" },
};

static const int GAME_STATUS_COUNT = sizeof(s_gameStatusEntries) / sizeof(s_gameStatusEntries[0]);

void drawDebugGameStatusMenu()
{
    if (!gDebugWindows.gameStatus)
        return;

    static int s_selectedIndex = 0;

    ImGui::SetNextWindowSize(ImVec2(420, 300), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Game Status Selector", &gDebugWindows.gameStatus))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Current: mode=%d sub=%d status=0x%02X prev=0x%02X",
        gGameStatus.m0_gameMode, gGameStatus.m1, gGameStatus.m4_gameStatus, gGameStatus.m6_previousGameStatus);
    if (gGameStatus.m8_nextGameStatus)
        ImGui::TextColored(ImVec4(1,1,0,1), "Pending transition to: 0x%02X", gGameStatus.m8_nextGameStatus);
    ImGui::Separator();

    ImGui::Text("Jump to game status:");
    if (ImGui::BeginListBox("##statuslist", ImVec2(-1, -40)))
    {
        for (int i = 0; i < GAME_STATUS_COUNT; i++)
        {
            bool selected = (s_selectedIndex == i);
            if (ImGui::Selectable(s_gameStatusEntries[i].name, selected))
                s_selectedIndex = i;
        }
        ImGui::EndListBox();
    }

    if (gModuleManager)
    {
        if (ImGui::Button("Jump!", ImVec2(120, 0)))
        {
            setNextGameStatus(s_gameStatusEntries[s_selectedIndex].id);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Will transition after current frame");
    }
    else
    {
        ImGui::TextColored(ImVec4(1,0.5f,0,1), "Module manager not running (debug menu flow only)");
    }

    ImGui::End();
}

#else
void drawDebugGameStatusMenu() {}
#endif

