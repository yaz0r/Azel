#include "PDS.h"
#include "townScriptDecompiler.h"
#include "town.h"
#include "townScript.h"

#include <imgui.h>
#include <algorithm>

extern sKernelScriptFunctions gKernelScriptFunctions;
extern sTownOverlay* gCurrentTownOverlay;


static const char* lookupFunctionName(u32 address)
{
    const char* name = gKernelScriptFunctions.lookupName(address);
    if (name) return name;

    if (gCurrentTownOverlay)
    {
        name = gCurrentTownOverlay->overlayScriptFunctions.lookupName(address);
        if (name) return name;
    }

    return nullptr;
}


static sSaturnPtr alignOn2(sSaturnPtr ptr)
{
    ptr.m_offset = (ptr.m_offset + 1) & ~1;
    return ptr;
}

static sSaturnPtr alignOn4(sSaturnPtr ptr)
{
    ptr.m_offset = (ptr.m_offset + 3) & ~3;
    return ptr;
}

// Growable text buffer
struct TextBuf
{
    std::vector<char> data;
    TextBuf() { data.reserve(8192); }
    void append(const char* fmt, ...)
    {
        char tmp[512];
        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(tmp, sizeof(tmp), fmt, args);
        va_end(args);
        if (len > 0)
            data.insert(data.end(), tmp, tmp + len);
    }
    std::string toString() const { return std::string(data.data(), data.size()); }
};


struct Instruction
{
    u32 addr;           // address of the opcode byte
    u32 nextAddr;       // address of the next sequential instruction
    u8 opcode;
    char text[256];     // formatted disassembly (no address prefix)

    // Control flow edges
    enum FlowType { FALL_THROUGH, TERMINATE, BRANCH, UNCONDITIONAL };
    FlowType flow;

    // Branch/jump targets (addresses to also explore)
    u32 branchTargets[64];
    int numBranchTargets;

    // Call targets (separate subroutines to decompile)
    u32 callTargets[64];
    int numCallTargets;

    // For structured if/else: condition text when this instruction can be folded with a following IF_FALSE
    char conditionForIf[128];
    bool hasCondition;
};


static bool decodeInstruction(sSaturnPtr startPC, Instruction& inst)
{
    sSaturnPtr r14 = startPC;
    inst.addr = r14.m_offset;
    inst.numBranchTargets = 0;
    inst.numCallTargets = 0;
    inst.flow = Instruction::FALL_THROUGH;
    inst.text[0] = 0;
    inst.conditionForIf[0] = 0;
    inst.hasCondition = false;

    u8 opcode = readSaturnU8(r14++);
    inst.opcode = opcode;

    switch (opcode)
    {
    case 1: // RETURN
        snprintf(inst.text, sizeof(inst.text), "return;");
        inst.flow = Instruction::TERMINATE;
        inst.nextAddr = r14.m_offset;
        return true;

    case 2: // WAIT
    {
        sSaturnPtr r4 = alignOn2(r14);
        u16 delay = readSaturnU16(r4);
        r4 += 2;
        snprintf(inst.text, sizeof(inst.text), "wait(%d);", delay);
        inst.flow = Instruction::FALL_THROUGH;
        inst.nextAddr = r4.m_offset;
        return true;
    }

    case 3: // JUMP
    {
        sSaturnPtr target = readSaturnEA(alignOn4(r14));
        snprintf(inst.text, sizeof(inst.text), "goto L_%08X;", target.m_offset);
        inst.flow = Instruction::UNCONDITIONAL;
        inst.branchTargets[inst.numBranchTargets++] = target.m_offset;
        inst.nextAddr = alignOn4(r14).m_offset + 4;
        return true;
    }

    case 5: // IF_FALSE
    {
        sSaturnPtr aligned = alignOn4(r14);
        sSaturnPtr target = readSaturnEA(aligned);
        snprintf(inst.text, sizeof(inst.text), "if (!result) goto L_%08X;", target.m_offset);
        inst.flow = Instruction::BRANCH;
        inst.branchTargets[inst.numBranchTargets++] = target.m_offset;
        inst.nextAddr = aligned.m_offset + 4;
        return true;
    }

    case 6: // CALL
    {
        sSaturnPtr aligned = alignOn4(r14);
        sSaturnPtr target = readSaturnEA(aligned);
        snprintf(inst.text, sizeof(inst.text), "sub_%08X();", target.m_offset);
        inst.flow = Instruction::FALL_THROUGH;
        inst.callTargets[inst.numCallTargets++] = target.m_offset;
        inst.nextAddr = aligned.m_offset + 4;
        return true;
    }

    case 7: // CALL_NATIVE
    {
        u8 numArgs = readSaturnU8(r14);
        r14 = r14 + 1;
        sSaturnPtr aligned = alignOn4(r14);
        sSaturnPtr functionEA = readSaturnEA(aligned);
        const char* name = lookupFunctionName(functionEA.m_offset);

        char argsBuf[256] = "";
        int pos = 0;
        for (u8 a = 0; a < numArgs; a++)
        {
            s32 arg = readSaturnS32(aligned + 4 + a * 4);
            if (a > 0) pos += snprintf(argsBuf + pos, sizeof(argsBuf) - pos, ", ");
            if ((arg & 0x06000000) == 0x06000000)
                pos += snprintf(argsBuf + pos, sizeof(argsBuf) - pos, "0x%08X", arg);
            else
                pos += snprintf(argsBuf + pos, sizeof(argsBuf) - pos, "%d", arg);
        }

        if (name)
            snprintf(inst.text, sizeof(inst.text), "%s(%s);", name, argsBuf);
        else
            snprintf(inst.text, sizeof(inst.text), "native_0x%08X(%s);", functionEA.m_offset, argsBuf);

        inst.flow = Instruction::FALL_THROUGH;
        inst.nextAddr = aligned.m_offset + (numArgs + 1) * 4;
        return true;
    }

    case 8:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 val = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "result = (result == %d);", val);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "result == %d", val);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }
    case 9:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 val = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "result = (result != %d);", val);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "result != %d", val);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }
    case 10:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 val = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "result = (result > %d);", val);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "result > %d", val);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }
    case 11:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 val = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "result = (result >= %d);", val);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "result >= %d", val);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }
    case 12:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 val = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "result = (result < %d);", val);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "result < %d", val);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }
    case 13:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 val = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "result = (result <= %d);", val);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "result <= %d", val);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }
    case 14:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 val = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "result += %d;", val);
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }

    case 15: // SET_BIT
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 bit = readSaturnS16(aligned);
        s16 resolved = bit < 1000 ? bit + 3334 : bit;
        snprintf(inst.text, sizeof(inst.text), "setBit(%d); // bit %d", bit, resolved);
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }
    case 16: // CLEAR_BIT
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 bit = readSaturnS16(aligned);
        s16 resolved = bit < 1000 ? bit + 3334 : bit;
        snprintf(inst.text, sizeof(inst.text), "clearBit(%d); // bit %d", bit, resolved);
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }
    case 17: // GET_BIT
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 bit = readSaturnS16(aligned);
        s16 resolved = bit < 1000 ? bit + 3334 : bit;
        snprintf(inst.text, sizeof(inst.text), "result = getBit(%d); // bit %d", bit, resolved);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "getBit(%d)", bit);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }

    case 18: // READ_PACKED
    {
        s8 width = readSaturnS8(r14++);
        sSaturnPtr aligned = alignOn2(r14);
        s16 var0 = readSaturnS16(aligned);
        s16 resolved = var0 < 1000 ? var0 + 3334 : var0;
        snprintf(inst.text, sizeof(inst.text), "result = readPacked(%d, %d); // bit %d", (int)width, var0, resolved);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "readPacked(%d, %d)", (int)width, var0);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }

    case 20: // ADD_PACKED
    {
        s8 width = readSaturnS8(r14++);
        sSaturnPtr aligned = alignOn2(r14);
        s16 var0 = readSaturnS16(aligned);
        aligned += 2;
        s16 addend = readSaturnS16(aligned);
        aligned += 2;
        s16 resolved = var0 < 1000 ? var0 + 3334 : var0;
        snprintf(inst.text, sizeof(inst.text), "addPacked(%d, %d, %d); // bit %d", (int)width, var0, addend, resolved);
        inst.nextAddr = aligned.m_offset;
        return true;
    }

    case 21: // SWITCH_JUMP
    {
        u8 numEntries = readSaturnU8(r14++);
        sSaturnPtr aligned = alignOn4(r14);
        for (int j = 0; j < numEntries && inst.numBranchTargets < 64; j++)
        {
            sSaturnPtr target = readSaturnEA(aligned + j * 4);
            inst.branchTargets[inst.numBranchTargets++] = target.m_offset;
        }
        snprintf(inst.text, sizeof(inst.text), "switch (result) { /* %d cases */ }", (int)numEntries);
        inst.flow = Instruction::BRANCH;
        inst.nextAddr = aligned.m_offset + numEntries * 4;
        return true;
    }

    case 22: // SWITCH_CALL
    {
        u8 numEntries = readSaturnU8(r14++);
        sSaturnPtr aligned = alignOn4(r14);
        for (int j = 0; j < numEntries && inst.numCallTargets < 64; j++)
        {
            sSaturnPtr target = readSaturnEA(aligned + j * 4);
            inst.callTargets[inst.numCallTargets++] = target.m_offset;
        }
        snprintf(inst.text, sizeof(inst.text), "switch (result) { /* %d calls */ }", (int)numEntries);
        inst.flow = Instruction::FALL_THROUGH;
        inst.nextAddr = aligned.m_offset + numEntries * 4;
        return true;
    }

    case 24:
        snprintf(inst.text, sizeof(inst.text), "result = context.m4;");
        inst.nextAddr = r14.m_offset;
        return true;

    case 25:
        snprintf(inst.text, sizeof(inst.text), "cinematicBarsClose();");
        inst.nextAddr = r14.m_offset;
        return true;

    case 26:
        snprintf(inst.text, sizeof(inst.text), "cinematicBarsOpen();");
        inst.nextAddr = r14.m_offset;
        return true;

    case 27:
    {
        sSaturnPtr aligned = alignOn4(r14);
        sSaturnPtr strPtr = readSaturnEA(aligned);
        std::string strVal = readSaturnString(strPtr);
        snprintf(inst.text, sizeof(inst.text), "drawString(\"%s\");", strVal.c_str());
        inst.nextAddr = aligned.m_offset + 4;
        return true;
    }

    case 29:
        snprintf(inst.text, sizeof(inst.text), "clearString();");
        inst.nextAddr = r14.m_offset;
        return true;

    case 31: // WAIT_NATIVE
    {
        u8 numArgs = readSaturnU8(r14);
        r14 = r14 + 1;
        sSaturnPtr aligned = alignOn4(r14);
        sSaturnPtr functionEA = readSaturnEA(aligned);
        const char* name = lookupFunctionName(functionEA.m_offset);

        char argsBuf[256] = "";
        int pos = 0;
        for (u8 a = 0; a < numArgs; a++)
        {
            s32 arg = readSaturnS32(aligned + 4 + a * 4);
            if (a > 0) pos += snprintf(argsBuf + pos, sizeof(argsBuf) - pos, ", ");
            if ((arg & 0x06000000) == 0x06000000)
                pos += snprintf(argsBuf + pos, sizeof(argsBuf) - pos, "0x%08X", arg);
            else
                pos += snprintf(argsBuf + pos, sizeof(argsBuf) - pos, "%d", arg);
        }

        if (name)
            snprintf(inst.text, sizeof(inst.text), "yield %s(%s);", name, argsBuf);
        else
            snprintf(inst.text, sizeof(inst.text), "yield native_0x%08X(%s);", functionEA.m_offset, argsBuf);

        inst.nextAddr = aligned.m_offset + (numArgs + 1) * 4;
        return true;
    }

    case 32:
        snprintf(inst.text, sizeof(inst.text), "waitFade();");
        inst.nextAddr = r14.m_offset;
        return true;

    case 33:
    {
        s8 id = readSaturnS8(r14++);
        snprintf(inst.text, sizeof(inst.text), "playSound(%d);", (int)id);
        inst.nextAddr = r14.m_offset;
        return true;
    }

    case 34:
    {
        s8 id = readSaturnS8(r14++);
        snprintf(inst.text, sizeof(inst.text), "playPcm(%d);", (int)id);
        inst.nextAddr = r14.m_offset;
        return true;
    }

    case 36:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 duration = readSaturnS16(aligned);
        aligned += 2;
        aligned = alignOn4(aligned);
        sSaturnPtr strPtr = readSaturnEA(aligned);
        aligned += 4;
        std::string strVal = readSaturnString(strPtr);
        snprintf(inst.text, sizeof(inst.text), "displayString(%d, \"%s\");", duration, strVal.c_str());
        inst.nextAddr = aligned.m_offset;
        return true;
    }

    case 39:
    {
        s8 numChoices = readSaturnS8(r14++);
        if (numChoices > 0)
        {
            sSaturnPtr aligned = alignOn4(r14);
            int pos = snprintf(inst.text, sizeof(inst.text), "result = multiChoice(");
            for (s8 j = 0; j < numChoices; j++)
            {
                sSaturnPtr strPtr = readSaturnEA(aligned + j * 4);
                std::string strVal = readSaturnString(strPtr);
                if (pos < (int)sizeof(inst.text) - 1)
                {
                    if (j > 0) pos += snprintf(inst.text + pos, sizeof(inst.text) - pos, ", ");
                    pos += snprintf(inst.text + pos, sizeof(inst.text) - pos, "\"%s\"", strVal.c_str());
                }
            }
            if (pos < (int)sizeof(inst.text) - 1)
                snprintf(inst.text + pos, sizeof(inst.text) - pos, ");");
            inst.nextAddr = aligned.m_offset + numChoices * 4;
        }
        else
        {
            snprintf(inst.text, sizeof(inst.text), "result = multiChoice();");
            inst.nextAddr = r14.m_offset;
        }
        return true;
    }

    case 41:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 itemIndex = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "result = getItemCount(%d);", itemIndex);
        snprintf(inst.conditionForIf, sizeof(inst.conditionForIf), "getItemCount(%d)", itemIndex);
        inst.hasCondition = true;
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }

    case 43:
    {
        s8 count = readSaturnS8(r14++);
        sSaturnPtr aligned = alignOn2(r14);
        s16 itemIndex = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "addItem(%d, %d);", itemIndex, (int)count);
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }

    case 46:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 frame = readSaturnS16(aligned);
        snprintf(inst.text, sizeof(inst.text), "cutsceneWait(%d);", frame);
        inst.nextAddr = aligned.m_offset + 2;
        return true;
    }

    case 48:
    {
        sSaturnPtr aligned = alignOn2(r14);
        s16 r6 = readSaturnS16(aligned); aligned += 2;
        s16 itemIndex = readSaturnS16(aligned); aligned += 2;
        s16 itemCount = readSaturnS16(aligned); aligned += 2;
        snprintf(inst.text, sizeof(inst.text), "receiveItem(%d, %d, %d);", r6, itemIndex, itemCount);
        inst.nextAddr = aligned.m_offset;
        return true;
    }

    default:
        snprintf(inst.text, sizeof(inst.text), "/* UNKNOWN opcode 0x%02X */", opcode);
        inst.flow = Instruction::TERMINATE;
        inst.nextAddr = r14.m_offset;
        return false;
    }
}


struct DecompiledFunction
{
    u32 entryAddr;
    std::vector<Instruction> instructions; // sorted by addr
    std::vector<u32> labels;               // addresses that are branch targets
};

static void exploreFunction(sSaturnPtr entry, DecompiledFunction& func, std::vector<u32>& pendingCalls, int maxInstructions)
{
    func.entryAddr = entry.m_offset;

    // Addresses we've already decoded
    std::vector<u32> decoded;

    // Work queue of addresses to explore within this function
    std::vector<u32> workQueue;
    workQueue.push_back(entry.m_offset);

    while (!workQueue.empty() && (int)func.instructions.size() < maxInstructions)
    {
        u32 pc = workQueue.back();
        workQueue.pop_back();

        assert(pc >= 0x06000000 && pc < 0x06100000);

        // Skip if already decoded
        bool alreadyDone = false;
        for (u32 d : decoded)
        {
            if (d == pc) { alreadyDone = true; break; }
        }
        if (alreadyDone) continue;

        // Linear decode from this address until terminator or already-decoded addr
        sSaturnPtr cursor;
        cursor.m_offset = pc;
        cursor.m_file = entry.m_file;

        while ((int)func.instructions.size() < maxInstructions)
        {
            // Check if we've already decoded this address
            bool hit = false;
            for (u32 d : decoded)
            {
                if (d == cursor.m_offset) { hit = true; break; }
            }
            if (hit) break;

            decoded.push_back(cursor.m_offset);

            Instruction inst;
            memset(&inst, 0, sizeof(inst));
            bool ok = decodeInstruction(cursor, inst);

            func.instructions.push_back(inst);

            if (!ok)
                break;

            // Record branch targets as labels + add to work queue
            for (int t = 0; t < inst.numBranchTargets; t++)
            {
                assert(inst.branchTargets[t] >= 0x06000000 && inst.branchTargets[t] < 0x06100000);
                func.labels.push_back(inst.branchTargets[t]);
                workQueue.push_back(inst.branchTargets[t]);
            }

            // Record call targets for later
            for (int t = 0; t < inst.numCallTargets; t++)
            {
                assert(inst.callTargets[t] >= 0x06000000 && inst.callTargets[t] < 0x06100000);
                pendingCalls.push_back(inst.callTargets[t]);
            }

            if (inst.flow == Instruction::TERMINATE || inst.flow == Instruction::UNCONDITIONAL)
                break;

            // Continue to next sequential instruction
            cursor.m_offset = inst.nextAddr;
        }
    }

    // Sort instructions by address for output
    std::sort(func.instructions.begin(), func.instructions.end(),
        [](const Instruction& a, const Instruction& b) { return a.addr < b.addr; });

    // Deduplicate labels
    std::sort(func.labels.begin(), func.labels.end());
    func.labels.erase(std::unique(func.labels.begin(), func.labels.end()), func.labels.end());
}


static void emitIndent(TextBuf& out, int indent)
{
    for (int i = 0; i < indent; i++)
        out.append("    ");
}

static int findIdxForAddr(const DecompiledFunction& func, u32 addr)
{
    for (int i = 0; i < (int)func.instructions.size(); i++)
        if (func.instructions[i].addr == addr) return i;
    return -1;
}

// Find the last instruction address-wise in [startAddr, endAddr)
static int findLastIdxInRange(const DecompiledFunction& func, u32 startAddr, u32 endAddr)
{
    int last = -1;
    for (int i = 0; i < (int)func.instructions.size(); i++)
    {
        if (func.instructions[i].addr >= startAddr && func.instructions[i].addr < endAddr)
            last = i;
    }
    return last;
}

// Determine the merge point after an if/else.
// trueStart: first instruction of the true block (fall-through from IF_FALSE)
// falseTarget: jump target of IF_FALSE (start of else block or continuation)
// Returns: address where both paths merge. If == falseTarget, there's no else block.
static u32 findMergePoint(const DecompiledFunction& func, u32 trueStart, u32 falseTarget)
{
    int lastIdx = findLastIdxInRange(func, trueStart, falseTarget);
    if (lastIdx < 0) return falseTarget;

    auto& lastInst = func.instructions[lastIdx];
    if (lastInst.flow == Instruction::UNCONDITIONAL && lastInst.numBranchTargets > 0)
    {
        u32 jumpTarget = lastInst.branchTargets[0];
        if (jumpTarget > falseTarget)
            return jumpTarget; // merge is after else block
    }
    return falseTarget; // no else block
}

// Thread-local set of emitted addresses (reset per function)
static std::vector<u32> s_emittedAddrs;

static bool isEmitted(u32 addr)
{
    for (u32 a : s_emittedAddrs)
        if (a == addr) return true;
    return false;
}

static void markEmitted(u32 addr)
{
    s_emittedAddrs.push_back(addr);
}

static void emitStructuredBlock(const DecompiledFunction& func, TextBuf& out, u32 startAddr, u32 endAddr, int indent)
{
    int idx = findIdxForAddr(func, startAddr);
    if (idx < 0) return;

    while (idx < (int)func.instructions.size())
    {
        auto& inst = func.instructions[idx];
        if (inst.addr >= endAddr) break;
        if (isEmitted(inst.addr)) break;
        markEmitted(inst.addr);

        // Pattern: condition producer + IF_FALSE → if { } else { }
        if (inst.hasCondition && idx + 1 < (int)func.instructions.size())
        {
            auto& ifInst = func.instructions[idx + 1];
            if (ifInst.opcode == 5 && ifInst.addr < endAddr && !isEmitted(ifInst.addr))
            {
                markEmitted(ifInst.addr);
                u32 falseTarget = ifInst.branchTargets[0];
                u32 trueStart = ifInst.nextAddr;
                u32 merge = findMergePoint(func, trueStart, falseTarget);

                emitIndent(out, indent);
                out.append("if (%s) { // @%08X\n", inst.conditionForIf, inst.addr);

                // Emit true block
                emitStructuredBlock(func, out, trueStart, falseTarget, indent + 1);

                // Mark the JUMP at end of true block as emitted (it's the implicit "skip else")
                if (merge != falseTarget)
                {
                    int lastTrue = findLastIdxInRange(func, trueStart, falseTarget);
                    if (lastTrue >= 0)
                        markEmitted(func.instructions[lastTrue].addr);

                    emitIndent(out, indent);
                    out.append("} else {\n");
                    emitStructuredBlock(func, out, falseTarget, merge, indent + 1);
                }

                emitIndent(out, indent);
                out.append("}\n");

                idx = findIdxForAddr(func, merge);
                if (idx < 0) return;
                continue;
            }
        }

        // Pattern: standalone IF_FALSE (no preceding condition) → if (result) { ... }
        if (inst.opcode == 5)
        {
            u32 falseTarget = inst.branchTargets[0];
            u32 trueStart = inst.nextAddr;
            u32 merge = findMergePoint(func, trueStart, falseTarget);

            emitIndent(out, indent);
            out.append("if (result) { // @%08X\n", inst.addr);

            emitStructuredBlock(func, out, trueStart, falseTarget, indent + 1);

            if (merge != falseTarget)
            {
                int lastTrue = findLastIdxInRange(func, trueStart, falseTarget);
                if (lastTrue >= 0)
                    markEmitted(func.instructions[lastTrue].addr);

                emitIndent(out, indent);
                out.append("} else {\n");
                emitStructuredBlock(func, out, falseTarget, merge, indent + 1);
            }

            emitIndent(out, indent);
            out.append("}\n");

            idx = findIdxForAddr(func, merge);
            if (idx < 0) return;
            continue;
        }

        // Pattern: SWITCH_JUMP → switch (result) { case N: ... }
        if (inst.opcode == 21 && inst.numBranchTargets > 0)
        {
            // Collect and sort targets with their case indices
            struct CaseEntry { int caseIdx; u32 target; };
            std::vector<CaseEntry> cases;
            for (int j = 0; j < inst.numBranchTargets; j++)
                cases.push_back({j, inst.branchTargets[j]});

            // Sort by target address for block boundary detection
            std::sort(cases.begin(), cases.end(),
                [](const CaseEntry& a, const CaseEntry& b) { return a.target < b.target; });

            // Find merge point: most common JUMP target from within switch blocks
            u32 switchMerge = endAddr;
            u32 maxTarget = cases.back().target;
            std::map<u32, int> jumpCounts;
            for (auto& i2 : func.instructions)
            {
                if (i2.addr >= cases.front().target && i2.flow == Instruction::UNCONDITIONAL && i2.numBranchTargets > 0)
                {
                    u32 jt = i2.branchTargets[0];
                    if (jt >= maxTarget)
                        jumpCounts[jt]++;
                }
            }
            if (!jumpCounts.empty())
            {
                int maxCount = 0;
                for (auto& p : jumpCounts)
                {
                    if (p.second > maxCount) { maxCount = p.second; switchMerge = p.first; }
                }
            }

            emitIndent(out, indent);
            out.append("switch (result) { // @%08X\n", inst.addr);

            // Deduplicate targets (multiple cases → same address)
            std::vector<u32> uniqueTargets;
            for (auto& c : cases)
            {
                if (uniqueTargets.empty() || uniqueTargets.back() != c.target)
                    uniqueTargets.push_back(c.target);
            }

            for (int ci = 0; ci < (int)uniqueTargets.size(); ci++)
            {
                u32 caseStart = uniqueTargets[ci];
                u32 caseEnd = (ci + 1 < (int)uniqueTargets.size()) ? uniqueTargets[ci + 1] : switchMerge;

                // Print all case labels that point to this target
                for (auto& c : cases)
                {
                    if (c.target == caseStart)
                    {
                        emitIndent(out, indent);
                        out.append("case %d:\n", c.caseIdx);
                    }
                }

                emitStructuredBlock(func, out, caseStart, caseEnd, indent + 1);

                // Mark end-of-case JUMP as emitted
                int lastCase = findLastIdxInRange(func, caseStart, caseEnd);
                if (lastCase >= 0 && func.instructions[lastCase].flow == Instruction::UNCONDITIONAL)
                {
                    markEmitted(func.instructions[lastCase].addr);
                    emitIndent(out, indent + 1);
                    out.append("break;\n");
                }
            }

            emitIndent(out, indent);
            out.append("}\n");

            idx = findIdxForAddr(func, switchMerge);
            if (idx < 0) return;
            continue;
        }

        // Normal instruction
        emitIndent(out, indent);
        out.append("%s // @%08X\n", inst.text, inst.addr);

        if (inst.flow == Instruction::TERMINATE) return;
        if (inst.flow == Instruction::UNCONDITIONAL)
        {
            // Backward jump = loop goto (already in text)
            return;
        }

        idx++;
    }
}


std::string decompileScript(sSaturnPtr startAddress, int maxOpcodes)
{
    if (startAddress.isNull())
        return "// (null script)\n";

    std::vector<DecompiledFunction> functions;
    std::vector<u32> pendingCalls;
    std::vector<u32> doneFunctions;

    // Start with the entry point
    pendingCalls.push_back(startAddress.m_offset);

    int totalInstructions = 0;

    while (!pendingCalls.empty() && totalInstructions < maxOpcodes)
    {
        u32 callAddr = pendingCalls.back();
        pendingCalls.pop_back();

        // Skip if already decompiled
        bool done = false;
        for (u32 d : doneFunctions)
        {
            if (d == callAddr) { done = true; break; }
        }
        if (done) continue;
        doneFunctions.push_back(callAddr);

        sSaturnPtr entry;
        entry.m_offset = callAddr;
        entry.m_file = startAddress.m_file;

        DecompiledFunction func;
        exploreFunction(entry, func, pendingCalls, maxOpcodes - totalInstructions);
        totalInstructions += (int)func.instructions.size();
        functions.push_back(func);
    }

    // Format output using structured emitter
    TextBuf out;

    for (int fi = 0; fi < (int)functions.size(); fi++)
    {
        auto& func = functions[fi];

        if (fi > 0)
            out.append("\n");

        if (fi == 0)
            out.append("void script_%08X() {\n", func.entryAddr);
        else
            out.append("void sub_%08X() {\n", func.entryAddr);

        s_emittedAddrs.clear();
        emitStructuredBlock(func, out, func.entryAddr, 0xFFFFFFFF, 1);

        out.append("}\n");
    }

    return out.toString();
}


static bool s_showWindow = false;
static char s_addressInput[32] = "";
static std::string s_manualDecompile;

void drawScriptDecompilerWindow()
{
    if (isShipping())
        return;

    if (!s_showWindow)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Debug"))
            {
                ImGui::MenuItem("Script Decompiler", nullptr, &s_showWindow);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Script Decompiler", &s_showWindow))
    {
        ImGui::End();
        return;
    }

    // VM State
    if (ImGui::CollapsingHeader("VM State", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("result: %d (0x%X)", npcData0.m118_currentResult, npcData0.m118_currentResult);
        ImGui::Text("delay: %d", npcData0.m100);
        ImGui::Text("flags: 0x%X  mF0=%d mF4=%d mF8=%d", npcData0.mFC, npcData0.mF0, npcData0.mF4, npcData0.mF8);

        sSaturnPtr currentPC = npcData0.m104_currentScript.m0_scriptPtr;
        if (!currentPC.isNull())
            ImGui::Text("PC: %08X (owner: %p)", currentPC.m_offset, npcData0.m104_currentScript.m8_owner);
        else
            ImGui::Text("PC: (idle)");

        int stackDepth = (int)(npcData0.m120_stack.end() - npcData0.m11C_currentStackPointer);
        ImGui::Text("Stack depth: %d", stackDepth);
        for (auto it = npcData0.m11C_currentStackPointer; it != npcData0.m120_stack.end(); ++it)
            ImGui::Text("  [%d] %08X", (int)(it - npcData0.m11C_currentStackPointer), it->m_offset);
    }

    // NPC Pointer Array
    if (ImGui::CollapsingHeader("NPC Array"))
    {
        for (int i = 0; i < (int)npcData0.m70_npcPointerArray.size(); i++)
        {
            auto& proxy = npcData0.m70_npcPointerArray[i];
            if (proxy.workArea || proxy.pNPC)
                ImGui::Text("[%2d] workArea=%p  pNPC=%p", i, proxy.workArea, proxy.pNPC);
        }
    }

    // Background Scripts
    if (ImGui::CollapsingHeader("Background Scripts"))
    {
        ImGui::Text("Count: %d", npcData0.m0_numBackgroundScripts);
        for (int i = 0; i < npcData0.m0_numBackgroundScripts; i++)
        {
            auto& ctx = npcData0.m4_backgroundScripts[i];
            ImGui::Text("[%d] script=%08X  type=%d  owner=%p",
                i, ctx.m0_scriptPtr.m_offset, ctx.m4, ctx.m8_owner);

            // Button to decompile this background script
            char btnLabel[64];
            snprintf(btnLabel, sizeof(btnLabel), "Decompile##bg%d", i);
            ImGui::SameLine();
            if (ImGui::SmallButton(btnLabel))
            {
                s_manualDecompile = decompileScript(ctx.m0_scriptPtr, 500);
                snprintf(s_addressInput, sizeof(s_addressInput), "%08X", ctx.m0_scriptPtr.m_offset);
            }
        }
    }

    ImGui::Separator();

    // Current script decompilation
    if (ImGui::CollapsingHeader("Current Script", ImGuiTreeNodeFlags_DefaultOpen))
    {
        sSaturnPtr currentPC = npcData0.m104_currentScript.m0_scriptPtr;
        if (!currentPC.isNull())
        {
            std::string decompiled = decompileScript(currentPC, 100);
            ImGui::TextUnformatted(decompiled.c_str());
        }
        else
        {
            ImGui::TextDisabled("(no active script)");
        }
    }

    ImGui::Separator();

    // Manual decompilation
    if (ImGui::CollapsingHeader("Decompile Address", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputText("Address (hex)", s_addressInput, sizeof(s_addressInput));
        ImGui::SameLine();
        if (ImGui::Button("Decompile"))
        {
            u32 addr = 0;
            if (sscanf(s_addressInput, "%x", &addr) == 1 && gCurrentTownOverlay)
            {
                sSaturnPtr ptr = gCurrentTownOverlay->getSaturnPtr(addr);
                s_manualDecompile = decompileScript(ptr, 500);
            }
        }

        // Quick buttons for script list
        if (npcData0.m64_scriptList)
        {
            auto& scripts = *npcData0.m64_scriptList;
            for (int i = 0; i < (int)scripts.size(); i++)
            {
                if (scripts[i].m_offset == 0)
                    continue;
                char label[32];
                snprintf(label, sizeof(label), "Script[%d] %08X", i, scripts[i].m_offset);
                if (ImGui::SmallButton(label))
                {
                    s_manualDecompile = decompileScript(scripts[i], 500);
                    snprintf(s_addressInput, sizeof(s_addressInput), "%08X", scripts[i].m_offset);
                }
                if ((i + 1) % 4 != 0)
                    ImGui::SameLine();
            }
            ImGui::NewLine();
        }

        if (!s_manualDecompile.empty())
        {
            ImGui::BeginChild("DecompileOutput", ImVec2(0, 0), true);
            ImGui::TextUnformatted(s_manualDecompile.c_str());
            ImGui::EndChild();
        }
    }

    ImGui::End();
}
