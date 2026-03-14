#include "PDS.h"
#include "campDragon.h"
#include "town/town.h"
#include "town/townDragon.h"
#include "kernel/fileBundle.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "town/collisionRegistry.h"
#include "audio/systemSounds.h"

void initDragonForTown(sTownDragon* pThis); // TODO: Cleanup
void updateTownDragon(sTownDragon* pThis); // TODO: cleanup

struct sDragonAnimDataSub
{
    s32 count;
    sDragonAnimDataSubRanges* m_data;
};

struct sCampDragon_F8 {
    u8 m0_boneIndex;
    u8 m1_hotpointIndex;
    sVec3_FP m4;
    sCollisionBody m10_collisionBody;
    sVec3_FP m74;
    sVec3_FP m80;
    // size 0x8C
};

struct sCampDragon : public sTownDragon
{
    //0-E8: sTownDragon
    sVec3_FP mE8;
    s32 mF4 = 0; // count of F8
    std::array<sCampDragon_F8, 9> mF8;
    //size: 0x624
};

// 06054af8
void initDragonHotpoints(sTownDragon* pThisBase) {
    sCampDragon* pThis = (sCampDragon*)pThisBase;
    s_3dModel& model = gDragonState->m28_dragon3dModel;
    if (!model.m40) return;

    std::vector<s_hotpointDefinition>& hotpointDefs = *model.m40;
    int f8Count = 0;
    int e0Count = 0;

    for (int boneIdx = 0; boneIdx < (int)hotpointDefs.size(); boneIdx++) {
        s_hotpointDefinition& def = hotpointDefs[boneIdx];
        for (int hpIdx = 0; hpIdx < (int)def.m0.size(); hpIdx++) {
            s_hotpoinEntry& entry = def.m0[hpIdx];
            if (entry.m10 < 0x1801) {
                // Small hotpoint -> mE0 height-tracking pair
                if (e0Count < 4) {
                    pThis->mE0_hotpointPairs[e0Count].m0_boneIndex = (u8)boneIdx;
                    pThis->mE0_hotpointPairs[e0Count].m1_hotpointIndex = (u8)hpIdx;
                    e0Count++;
                }
            }
            else {
                // Large hotpoint -> mF8 collision body entry
                if (f8Count < 9) {
                    auto& f8 = pThis->mF8[f8Count];
                    f8.m0_boneIndex = (u8)boneIdx;
                    f8.m1_hotpointIndex = (u8)hpIdx;
                    fixedPoint size = performDivision(10, entry.m10);
                    f8.m74 = { -size, -(size + 0x800), -size };
                    f8.m80 = { size, size, size };
                    setCollisionBounds(&f8.m10_collisionBody, f8.m74, f8.m80);
                    f8Count++;
                }
            }
        }
    }

    pThis->mF4 = f8Count;
    pThis->mDC_hotpointPairCount = e0Count;
}

// 06054c94
s32 sCampDragon_InitSub1(sTownDragon* pThis) {
    // npcPointerArray[0] is the edge/player task; offset 0xE8 is its position
    sVec3_FP& edgePos = *(sVec3_FP*)((u8*)npcData0.m70_npcPointerArray[0].workArea + 0xE8);
    return atan2_FP(edgePos.m0_X - pThis->m58_position.m0_X,
                    edgePos.m8_Z - pThis->m58_position.m8_Z);
}

void increaseGameResource(int param_1, int param_2) {
    switch (param_1)
    {
    case 0xB:
        mainGameState.gameStats.m7C_overallRating = mainGameState.gameStats.m7C_overallRating + param_2;
        if ((int)mainGameState.gameStats.m7C_overallRating < 0) {
            mainGameState.gameStats.m7C_overallRating = 0;
        }
        if (0x7c < (int)mainGameState.gameStats.m7C_overallRating) {
            mainGameState.gameStats.m7C_overallRating = 0x7c;
        }
        break;
    default:
        assert(0);
        break;
    }
}

s32 getDragonAffinityLevel() {
    s32 uVar1;
    int iVar2;

    uVar1 = mainGameState.gameStats.m7C_overallRating;
    iVar2 = readPackedBits(mainGameState.bitField, 0x1347, 0x20);
    iVar2 = uVar1 - iVar2;
    if (iVar2 < -0xf) {
        return 2;
    }
    if (iVar2 < -5) {
        return 3;
    }
    if (0xf < iVar2) {
        return 6;
    }
    if (5 < iVar2) {
        return 5;
    }
    return 4;
}

u32 sCampDragon_InitSub2(sCampDragon* pThis) {
    u32 currentFrame = updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
    if (!pThis->m24.isNull()) {
        sSaturnPtr ptr = pThis->m24;
        u32 count = pThis->m28;
        for (u32 i = 0; i < count; i++) {
            if ((s32)currentFrame == (s32)readSaturnS8(ptr)) {
                playSystemSoundEffect((s32)readSaturnS8(ptr + 1));
                return currentFrame;
            }
            ptr = ptr + 2;
        }
    }
    return currentFrame;
}

void sCampDragon_Init(sTownDragon* pThisBase, sSaturnPtr arg) {
    sCampDragon* pThis = (sCampDragon*)pThisBase;
    pThis->m48_entityEA = arg;
    pThis->m4C_basePosition = pThis->m58_position = readSaturnVec3(arg + 0x8);
    pThis->m64_rotation = readSaturnVec3(arg + 0x14);
    npcData0.m70_npcPointerArray[readSaturnU16(arg + 0x20)].workArea = pThis;
    pThis->m16_timer = 0x3C;
    increaseGameResource(0xb, 0);
    pThis->mF_affinityLevel = getDragonAffinityLevel();
    pThis->m40 = 0x8000000;
    pThis->m3C = 0x8000000;
    pThis->mD8_heightOffset = 0x1000;
    s32 temp = 0x8000000;
    if (mainGameState.getBit(0x263, 0x20) == 0) {
        temp = 0x4000000;
    }
    else {
        temp += performDivision(0x19, mainGameState.gameStats.m7C_overallRating) * 0x2000000;
    }
    pThis->m64_rotation.m4_Y = sCampDragon_InitSub1(pThis) + temp;
    pThis->m70_collisionBody.m30_pPosition = &pThis->m58_position;
    pThis->m70_collisionBody.m34_pRotation = &pThis->m64_rotation;
    pThis->m70_collisionBody.m38_pOwner = pThis;
    pThis->m70_collisionBody.m3C_scriptEA = readSaturnEA(arg + 0x24);
    pThis->m70_collisionBody.m40 = 0;
    setCollisionSetup(&pThis->m70_collisionBody, 3);

    static const sVec3_FP param1 = {
        -0x1800,
        -0x1800,
        -0x1800,
    };

    static const sVec3_FP param2 = {
        0x1800,
        0x1000,
        0x1800,
    };

    setCollisionBounds(&pThis->m70_collisionBody, param1, param2);

    for (int i = 0; i < 9; i++) {
        auto* psVar3 = &pThis->mF8[i];
        (psVar3->m10_collisionBody).m30_pPosition = &psVar3->m4;
        (psVar3->m10_collisionBody).m34_pRotation = &pThis->mE8;
        (psVar3->m10_collisionBody).m38_pOwner = pThis;
        (psVar3->m10_collisionBody).m3C_scriptEA = sSaturnPtr::getNull();
        (psVar3->m10_collisionBody).m40 = nullptr;
        setCollisionSetup(&psVar3->m10_collisionBody, 3);
        setCollisionBounds(&psVar3->m10_collisionBody, psVar3->m74, psVar3->m80);
    }
    initDragonForTown(pThis);
    pThis->m14_readyState = 1;

    playAnimation(&gDragonState->m28_dragon3dModel, pThis->m1C->m0_fileBundle->getAnimation(readSaturnU16(pThis->m20_scriptEA + 2)), 0);
    sCampDragon_InitSub2(pThis);
}

void sCampDragon_UpdateSub0(sTownDragon* pThis) {
    if (pThis->m11_subState < 9) {
        if (((mainGameState.getBit(1, 0x80) == 0) && ((mainGameState.getBit(0x263, 0x20) != 0))))
        {
            if (pThis->m11_subState < 5) {
                pThis->m10_modeOffset = 0;
                int iVar1 = vecDistance(pThis->m58_position, *npcData0.m160_pEdgePosition);
                if (pThis->m11_subState < 2) {
                    if ((iVar1 < 0x5000) &&
                        (pThis->m11_subState = 2, (mainGameState.getBit(0x263, 0x40) == 0))) {
                        iVar1 = performDivision(0x19, mainGameState.gameStats.m7C_overallRating);
                        increaseGameResource(10, iVar1);
                        mainGameState.getBit(0x263, 0x40);
                    }
                }
                else if ((1 < (char)pThis->m11_subState) && (0x6000 < iVar1)) {
                    pThis->m11_subState = 0;
                }
            }
        }
        else {
            pThis->m10_modeOffset = 0;
            pThis->m11_subState = 9;
        }
    }
}

void sCampDragon_UpdateMode4Mode9Sub0(sCampDragon* pThis, int newReadyState)
{
    assert(gCurrentTownOverlay->m_name == "TWN_CAMP.PRG");

    pThis->m14_readyState = newReadyState;
    pThis->m24 = readSaturnEA(gCurrentTownOverlay->getSaturnPtr(0x607acfc + newReadyState * 8));
    pThis->m28 = readSaturnU32(gCurrentTownOverlay->getSaturnPtr(0x607ad00 + newReadyState * 8));
    playAnimation(&gDragonState->m28_dragon3dModel, pThis->m1C->m0_fileBundle->getAnimation(readSaturnU16(pThis->m20_scriptEA + newReadyState * 2)), 0xF);
}

void sCampDragon_UpdateMode4Mode9(sCampDragon* param_1, int param_2)
{
    if (param_1->m14_readyState != param_2) {
        if (param_1->m14_readyState < 10) {
            if ((9 < param_2) && (param_2 != 0x17)) {
                param_1->mE = param_1->mE | 1;
                assert(gCurrentTownOverlay->m_name == "TWN_CAMP.PRG");
                param_1->m34 = gCurrentTownOverlay->getSaturnPtr(0x607af5c);
                param_2 = 0x17;
            }
        }
        else if ((param_2 < 10) && (param_2 != 9)) {
            param_2 = 9;
            param_1->mE = param_1->mE | 1;
            assert(gCurrentTownOverlay->m_name == "TWN_CAMP.PRG");
            param_1->m34 = gCurrentTownOverlay->getSaturnPtr(0x607af5c);
        }
        sCampDragon_UpdateMode4Mode9Sub0(param_1, param_2);
    }
    sCampDragon_InitSub2(param_1);
}

// 06054e9c
int sCampDragon_readNextCommand(sCampDragon* pThis) {
    while (true) {
        s16 cmd = readSaturnS16(pThis->m34);
        if (cmd == -2) {
            pThis->m12_eventFlag = 0;
        }
        else if (cmd == -1) {
            pThis->m12_eventFlag = 1;
        }
        else {
            if (cmd != 0) {
                return cmd;
            }
            pThis->mE = pThis->mE & ~1;
            return 0;
        }
        pThis->m34 = pThis->m34 + 2;
    }
}

// 06054f24
void sCampDragon_startAnimSequence(sTownDragon* pThisBase, sSaturnPtr cmdPtr) {
    sCampDragon* pThis = (sCampDragon*)pThisBase;
    pThis->mE = pThis->mE | 1;
    pThis->m34 = cmdPtr;
    int cmd = sCampDragon_readNextCommand(pThis);
    if (cmd != 0) {
        if (pThis->m14_readyState < 10) {
            if (cmd == 9) {
                pThis->m34 = pThis->m34 + 2;
                return;
            }
            if ((9 < cmd) && (cmd != 0x17)) {
                sCampDragon_UpdateMode4Mode9Sub0(pThis, 0x17);
                return;
            }
        }
        else {
            if (cmd == 0x17) {
                pThis->m34 = pThis->m34 + 2;
                return;
            }
            if ((cmd < 10) && (cmd != 9)) {
                sCampDragon_UpdateMode4Mode9Sub0(pThis, 9);
                return;
            }
        }
        sCampDragon_UpdateMode4Mode9Sub0(pThis, cmd);
        pThis->m34 = pThis->m34 + 2;
    }
}

// 0605502c
int sCampDragon_stepAngleTowards(s32* current, s32* target) {
    s32 diff = *target - *current;
    if ((diff & 0x8000000) == 0) {
        diff = diff & 0xFFFFFFF;
    }
    else {
        diff = diff | 0xF0000000;
    }
    if ((diff < 0x111111) && (-0x111111 < diff)) {
        *current = *target;
        return 1;
    }
    if (diff < 1) {
        *current = *current - 0x111111;
    }
    else {
        *current = *current + 0x111111;
    }
    return 0;
}

// 06054eda
void sCampDragon_UpdateModeE1(sCampDragon* pThis) {
    u32 currentFrame = sCampDragon_InitSub2(pThis);
    sAnimationData* pAnim = gDragonState->m28_dragon3dModel.m30_pCurrentAnimation;
    int numFrames = 0;
    if (pAnim) {
        numFrames = pAnim->m4_numFrames;
    }
    if ((numFrames - 1 <= (int)currentFrame)) {
        int cmd = sCampDragon_readNextCommand(pThis);
        if (cmd != 0) {
            sCampDragon_UpdateMode4Mode9Sub0(pThis, cmd);
            pThis->m34 = pThis->m34 + 2;
        }
    }
}

// 06055070
void sCampDragon_UpdateModeE2(sCampDragon* pThis) {
    sCampDragon_UpdateMode4Mode9(pThis, 0x14);
    if ((pThis->m14_readyState == 0x14) &&
        (sCampDragon_stepAngleTowards((s32*)&pThis->m64_rotation.m4_Y, &pThis->m38_savedRotationY) != 0)) {
        pThis->mE = pThis->mE & ~2;
    }
}

// 06055a6c
void sCampDragon_UpdateMode1(sCampDragon* pThis) {
    Unimplemented();
}

// 06055b36
void sCampDragon_UpdateMode2(sCampDragon* pThis) {
    Unimplemented();
}

// 06055c2a
void sCampDragon_UpdateMode3(sCampDragon* pThis) {
    Unimplemented();
}

void sCampDragon_UpdateMode4(sCampDragon* pThis) {
    switch (pThis->m11_subState) {
    case 0:
        pThis->m12_eventFlag = 1;
        pThis->m11_subState = 1;
        break;
    case 9:
        pThis->m12_eventFlag = 0;
        sCampDragon_UpdateMode4Mode9(pThis, 0);
        break;
    default:
        assert(0);
        break;
    }
}

// 06055ce8
void sCampDragon_UpdateMode5(sCampDragon* pThis) {
    Unimplemented();
}

// 06055d50
void sCampDragon_UpdateMode6(sCampDragon* pThis) {
    Unimplemented();
}

static s32 signExtend28(s32 value) {
    if (value & 0x8000000)
        return value | 0xF0000000;
    else
        return value & 0xFFFFFFF;
}

// 06054554
void sCampDragon_smoothBoneChain(s_runtimeAnimData* entries, int headIdx, int count) {
    for (int i = headIdx; count > 0; count--, i--) {
        s32 avgX = performDivision(3, entries[i].m_vec_18.m0_X.asS32() * 2 + entries[i - 2].m_vec_18.m0_X.asS32());
        s32 avgY = performDivision(3, entries[i].m_vec_18.m4_Y.asS32() * 2 + entries[i - 2].m_vec_18.m4_Y.asS32());
        s32 smoothedX = MTH_Mul(0xCCCC, entries[i - 1].m_vec_18.m0_X.asS32() - avgX) + avgX;
        s32 smoothedY = MTH_Mul(0xCCCC, entries[i - 1].m_vec_18.m4_Y.asS32() - avgY) + avgY;
        entries[i - 1].m_vec_18.m0_X = smoothedX;
        entries[i - 1].m_vec_18.m4_Y = smoothedY;
    }
}

// 060545ca
void sCampDragon_updateHeadTracking(sVec3_FP* targetAngles, int mode, u32 clampParam) {
    s3DModelAnimData& animData = gDragonState->m78_animData;
    s_runtimeAnimData* entries = animData.m8_runtimeAnimData;
    int headIdx = (s8)animData.count0;
    s_runtimeAnimData& head = entries[headIdx];

    // Adjust target X based on Y distance
    s32 diffY = signExtend28(head.m_vec_18.m4_Y.asS32() - targetAngles->m4_Y.asS32());
    if (diffY > 0xE38E38 || diffY < -0xE38E38) {
        if (diffY < 0) diffY = -diffY;
        if (diffY > 0x4000000) diffY = 0x4000000;
        targetAngles->m0_X = setDividend(targetAngles->m0_X.asS32(), 0x4000000 - diffY, 0x31C71C7);
    }

    // Compute deltas towards target with clamping
    s32 deltaX = (targetAngles->m0_X.asS32() - head.m_vec_18.m0_X.asS32()) >> 2;
    s32 deltaY = (targetAngles->m4_Y.asS32() - head.m_vec_18.m4_Y.asS32()) >> 2;
    if (deltaX > 0x2D82D8) deltaX = 0x2D82D8;
    if (deltaX < -0x2D82D8) deltaX = -0x2D82D8;
    if (deltaY > 0x2D82D8) deltaY = 0x2D82D8;
    if (deltaY < -0x2D82D8) deltaY = -0x2D82D8;
    head.m_vec_18.m0_X = head.m_vec_18.m0_X.asS32() + deltaX;
    head.m_vec_18.m4_Y = head.m_vec_18.m4_Y.asS32() + deltaY;

    // Z rotation
    if (mode == 0) {
        head.m_vec_18.m8_Z = targetAngles->m8_Z;
    }
    else {
        s32 diffZ = signExtend28(targetAngles->m8_Z.asS32() - head.m_vec_18.m8_Z.asS32());
        s32 deltaZ = diffZ >> 1;
        if (deltaZ > 0x2D82D8) deltaZ = 0x2D82D8;
        if (deltaZ < -0x2D82D8) deltaZ = -0x2D82D8;
        head.m_vec_18.m8_Z = head.m_vec_18.m8_Z.asS32() + deltaZ;
    }

    // Sign-extend head X and Y to 28-bit angle range
    entries[0].m_vec_18.m0_X = signExtend28(entries[0].m_vec_18.m0_X.asS32());
    entries[0].m_vec_18.m4_Y = signExtend28(entries[0].m_vec_18.m4_Y.asS32());

    // Smooth intermediate bones
    sCampDragon_smoothBoneChain(entries, headIdx, (s8)animData.count0 - 1);

    // Clamp bone rotations against pose limits
    int numBones = (s8)animData.count0;
    s_runtimeAnimData* bone = &entries[1];
    for (int i = 0; i < numBones; i++, bone++) {
        s32 boneIdx = bone->dataSource->count;
        sPoseData& pose = gDragonState->m28_dragon3dModel.m2C_poseData[boneIdx];

        s32 totalX = signExtend28(bone->m_vec_18.m0_X.asS32() + pose.mC_rotation.m0_X.asS32());
        if (totalX > 0x18E38E3) totalX = 0x18E38E3;
        if (totalX < -0x18E38E3) totalX = -0x18E38E3;
        bone->m_vec_18.m0_X = totalX - pose.mC_rotation.m0_X.asS32();

        s32 totalY = signExtend28(bone->m_vec_18.m4_Y.asS32() + pose.mC_rotation.m4_Y.asS32());
        if (totalY > (s32)clampParam) totalY = (s32)clampParam;
        if (totalY < -(s32)clampParam) totalY = -(s32)clampParam;
        bone->m_vec_18.m4_Y = totalY - pose.mC_rotation.m4_Y.asS32();
    }
}

// 06035c9c
static void translateMatrixByX(fixedPoint val, sMatrix4x3* mat) {
    for (int i = 0; i < 3; i++) {
        mat->m[i][3] = mat->m[i][3] + MTH_Mul(val, mat->m[i][0]);
    }
}

// 06035cd8
static void translateMatrixByY(fixedPoint val, sMatrix4x3* mat) {
    for (int i = 0; i < 3; i++) {
        mat->m[i][3] = mat->m[i][3] + MTH_Mul(val, mat->m[i][1]);
    }
}

// 06035d14
static void translateMatrixByZ(fixedPoint val, sMatrix4x3* mat) {
    for (int i = 0; i < 3; i++) {
        mat->m[i][3] = mat->m[i][3] + MTH_Mul(val, mat->m[i][2]);
    }
}

// 060543e6
static void applyInversePoseTransform(const sVec3_FP& translation, const sVec3_FP& rotation, sMatrix4x3* mat) {
    rotateMatrixShiftedX(-rotation.m0_X, mat);
    rotateMatrixShiftedY(-rotation.m4_Y, mat);
    rotateMatrixShiftedZ(-rotation.m8_Z, mat);
    translateMatrixByZ(-translation.m8_Z, mat);
    translateMatrixByY(-translation.m4_Y, mat);
    translateMatrixByX(-translation.m0_X, mat);
}

// 06054438
static void buildInverseBoneChainMatrix(s_runtimeAnimData* headEntry, s_3dModel* pModel, sMatrix4x3* mat) {
    assert(gCurrentTownOverlay->m_name == "TWN_CAMP.PRG");
    u32 dragonLevel = (u8)mainGameState.gameStats.m1_dragonLevel;
    // Table at 0x607abac has 8-byte entries: {pointer to config pairs, chain length}
    sSaturnPtr configPairPtr = readSaturnEA(gCurrentTownOverlay->getSaturnPtr(0x607abac + dragonLevel * 8));
    s32 chainLength = readSaturnS32(gCurrentTownOverlay->getSaturnPtr(0x607abb0 + dragonLevel * 8));

    s_runtimeAnimData* curEntry = headEntry;
    for (int i = 0; i < chainLength; i++) {
        s8 boneIdx = readSaturnS8(configPairPtr);
        s8 useRotation = readSaturnS8(configPairPtr + 1);

        s_runtimeAnimData* prevEntry = curEntry;
        if (useRotation != 0) {
            curEntry = curEntry - 1;
            rotateMatrixShiftedZ(-curEntry->m_vec_18.m8_Z, mat);
            rotateMatrixShiftedX(-curEntry->m_vec_18.m0_X, mat);
            rotateMatrixShiftedY(-curEntry->m_vec_18.m4_Y, mat);
        }

        sPoseData& pose = pModel->m2C_poseData[boneIdx];
        applyInversePoseTransform(pose.m0_translation, pose.mC_rotation, mat);

        configPairPtr = configPairPtr + 2;
    }
}

// 060544e4
static s32 computeBoneChainClamp(s_runtimeAnimData* entry, int count, s_3dModel* pModel) {
    sVec3_FP accum = { 0, 0, 0 };
    for (int i = 0; i < count; i++) {
        s32 boneIdx = (entry - 1)->dataSource->count;
        sPoseData& pose = pModel->m2C_poseData[boneIdx];
        accum.m0_X = accum.m0_X + pose.m0_translation.m0_X;
        accum.m4_Y = accum.m4_Y + pose.m0_translation.m4_Y;
        accum.m8_Z = accum.m8_Z + pose.m0_translation.m8_Z;
        entry = entry - 1;
    }

    s32 lengthSq = (s32)(((s64)accum.m0_X.asS32() * accum.m0_X.asS32() +
                          (s64)accum.m4_Y.asS32() * accum.m4_Y.asS32() +
                          (s64)accum.m8_Z.asS32() * accum.m8_Z.asS32()) >> 16);
    s32 length = sqrt_I(lengthSq);
    s32 result = length * 0x9F4;
    if (result > 0x18E38E3) result = 0x18E38E3;
    if (result < 0xB60B60) result = 0xB60B60;
    return result;
}

void transformVec(const sVec3_FP& r4, sVec3_FP& r5, const sMatrix4x3& r6); // from menu_dragonMorph
void updateEdgePositionSub3Sub1(const sVec3_FP& r4, sVec2_FP* r5); // from twn_ruin

// 06054800
void sCampDragon_updateEventTracking(sCampDragon* pThis) {
    // Get player position with Y offset
    sVec3_FP playerPos;
    playerPos.m0_X = npcData0.m160_pEdgePosition->m0_X;
    playerPos.m8_Z = npcData0.m160_pEdgePosition->m8_Z;
    playerPos.m4_Y = npcData0.m160_pEdgePosition->m4_Y + 0xCCC;

    s3DModelAnimData& animData = gDragonState->m78_animData;
    s_runtimeAnimData* entries = animData.m8_runtimeAnimData;
    s_runtimeAnimData& headEntry = entries[(s8)animData.count0];

    // Build inverse transform matrix from head bone to world
    sMatrix4x3 invMat;
    initMatrixToIdentity(&invMat);
    buildInverseBoneChainMatrix(&headEntry, &gDragonState->m28_dragon3dModel, &invMat);

    // Apply inverse dragon rotation and position
    rotateMatrixShiftedZ(-pThis->m64_rotation.m8_Z, &invMat);
    rotateMatrixShiftedX(-pThis->m64_rotation.m0_X, &invMat);
    rotateMatrixShiftedY(-pThis->m64_rotation.m4_Y, &invMat);
    translateMatrixByZ(-pThis->m58_position.m8_Z, &invMat);
    translateMatrixByY(-pThis->m58_position.m4_Y, &invMat);
    translateMatrixByX(-pThis->m58_position.m0_X, &invMat);

    // Build forward rotation matrix to get the forward direction
    sMatrix4x3 fwdMat;
    initMatrixToIdentity(&fwdMat);
    rotateMatrixYXZ(&pThis->m64_rotation, &fwdMat);

    // Get forward direction vector (column 2 = Z axis after rotation)
    sVec3_FP fwdDir = { fwdMat.m[0][2], fwdMat.m[1][2], fwdMat.m[2][2] };

    // Compute angles to forward direction
    sVec3_FP transformedFwd;
    transformVec(fwdDir, transformedFwd, invMat);
    sVec2_FP fwdAngles;
    updateEdgePositionSub3Sub1(transformedFwd, &fwdAngles);

    s32 neutralPitch = (0x8000000 - fwdAngles[0].asS32()) & 0xFFFFFFF;
    s32 neutralYaw = (fwdAngles[1].asS32() + 0x8000000) & 0xFFFFFFF;
    s32 currentPitch = (pThis->m3C - fwdAngles[0].asS32()) & 0xFFFFFFF;
    s32 currentYaw = (fwdAngles[1].asS32() + pThis->m40) & 0xFFFFFFF;

    // Compute angles to player position
    sVec3_FP transformedPlayer;
    transformAndAddVec(playerPos, transformedPlayer, invMat);
    sVec2_FP playerAngles;
    updateEdgePositionSub3Sub1(transformedPlayer, &playerAngles);

    s32 playerPitch = (-playerAngles[0].asS32()) & 0xFFFFFFF;
    s32 playerYaw = playerAngles[1].asS32() & 0xFFFFFFF;

    // Select target yaw, update m40
    if (playerYaw < currentYaw) {
        if (neutralYaw < playerYaw) {
            playerYaw = neutralYaw;
        }
        pThis->m40 = 0x8E38E38;
    }
    else {
        if (playerYaw < neutralYaw) {
            playerYaw = neutralYaw;
        }
        playerYaw = playerYaw | 0xF0000000;
        pThis->m40 = 0x71C71C7;
    }

    // Select target pitch, update m3C
    if (playerPitch < currentPitch) {
        if (neutralPitch < playerPitch) {
            playerPitch = neutralPitch;
        }
        pThis->m3C = 0x8E38E38;
    }
    else {
        if (playerPitch < neutralPitch) {
            playerPitch = neutralPitch;
        }
        playerPitch = playerPitch | 0xF0000000;
        pThis->m3C = 0x71C71C7;
    }

    // Compute roll from inverse matrix
    s32 roll = atan2_FP(-invMat.m[0][1], invMat.m[1][1]);

    // Compute clamp parameter from bone chain
    s32 clampParam = computeBoneChainClamp(
        &entries[(s8)animData.count0] + 1,
        (s8)animData.count0,
        &gDragonState->m28_dragon3dModel);

    // Build target angles and update head tracking
    sVec3_FP targetAngles = { playerPitch, playerYaw, roll };
    sCampDragon_updateHeadTracking(&targetAngles, 0, clampParam);
}

void sCampDragon_Update(sTownDragon* pThisBase) {
    sCampDragon* pThis = (sCampDragon*)pThisBase;

    pThis->mD_drawExtras = 1;
    updateTownDragon(pThis);

    if ((gDragonState->mC_dragonType < 0) || (7 < gDragonState->mC_dragonType)) {
        return;
    }

    pThis->m58_position = pThis->m4C_basePosition;
    pThis->m58_position[1] += pThis->mD8_heightOffset;
    registerCollisionBody(&pThis->m70_collisionBody);

    for (int i = 0; i < pThis->mF4; i++) {
        registerCollisionBody(&pThis->mF8[i].m10_collisionBody);
    }

    sCampDragon_UpdateSub0(pThis);

    if (pThis->mE == 0) {
        switch (pThis->mF_affinityLevel + pThis->m10_modeOffset) {
        case 1:
            sCampDragon_UpdateMode1(pThis);
            break;
        case 2:
            sCampDragon_UpdateMode2(pThis);
            break;
        case 3:
            sCampDragon_UpdateMode3(pThis);
            break;
        case 4:
            sCampDragon_UpdateMode4(pThis);
            break;
        case 5:
            sCampDragon_UpdateMode5(pThis);
            break;
        case 6:
            sCampDragon_UpdateMode6(pThis);
            break;
        default:
            assert(0);
            break;
        }
    }
    else if ((pThis->mE & 1) == 0) {
        if (pThis->mE & 2) {
            sCampDragon_UpdateModeE2(pThis);
        }
    }
    else {
        sCampDragon_UpdateModeE1(pThis);
    }

    if (pThis->m12_eventFlag == 0) {
        static sVec3_FP gCampDragon_headTrackingTarget = { 0, 0, 0 };
        sCampDragon_updateHeadTracking(&gCampDragon_headTrackingTarget, 1, 0x2000000);
    }
    else {
        sCampDragon_updateEventTracking(pThis);
    }
}

void updateAnimationMatricesSub1WithScale(s3DModelAnimData* r4, s_3dModel* r5)
{
    u32 r9 = r5->m12_numBones;
    if (r9)
    {
        std::vector<sMatrix4x3>::iterator r14 = r4->m4_boneMatrices->begin();
        if (r5->m48_poseDataInterpolation.size())
        {
            std::vector<sPoseDataInterpolation>::iterator r13 = r5->m48_poseDataInterpolation.begin();

            do
            {
                initMatrixToIdentity(&(*r14));
                translateMatrix(&r13->m0_translation, &(*r14));
                rotateMatrixZYX(&r13->mC_rotation, &(*r14));
                scaleMatrixRow0(r13->m18_scale[0], &(*r14));
                scaleMatrixRow1(r13->m18_scale[1], &(*r14));
                scaleMatrixRow2(r13->m18_scale[2], &(*r14));
                r13++;
                r14++;
            } while (--r9);
        }
        else
        {
            std::vector<sPoseData>::iterator r13 = r5->m2C_poseData.begin();

            do
            {
                initMatrixToIdentity(&(*r14));
                translateMatrix(&r13->m0_translation, &(*r14));
                rotateMatrixZYX(&r13->mC_rotation, &(*r14));
                scaleMatrixRow0(r13->m18_scale[0], &(*r14));
                scaleMatrixRow1(r13->m18_scale[1], &(*r14));
                scaleMatrixRow2(r13->m18_scale[2], &(*r14));
                r13++;
                r14++;
            } while (--r9);
        }
    }
}

void updateAnimationMatricesSub2(s3DModelAnimData* r4); // todo: cleanup

void updateAnimationMatricesSub2Sub1(s_runtimeAnimData* r4, sVec3_FP& r5, sVec3_FP& r6);
void updateAnimationMatricesSub2Sub2(s_runtimeAnimData* r4);
void updateAnimationMatricesSub2Sub3(s_runtimeAnimData* r4);

// 06009810
static void applyBoneRotation(s3DModelAnimData* pAnimData, s_runtimeAnimData* entry) {
    s32 boneIndex = entry->dataSource->count;
    sMatrix4x3& boneMatrix = (*pAnimData->m4_boneMatrices)[boneIndex];
    rotateMatrixYXZ(&entry->m_vec_18, &boneMatrix);
}

// 06054320
static void applyBoneChainRotations(s3DModelAnimData* pAnimData, s_runtimeAnimData*& curEntry, int count) {
    for (int i = 0; i < count; i++) {
        applyBoneRotation(pAnimData, curEntry);
        curEntry->m24_rootDelta.m0_X = 0;
        curEntry->m24_rootDelta.m4_Y = 0;
        curEntry->m24_rootDelta.m8_Z = 0;
        curEntry++;
    }
}

// updateAnimationMatricesSub3Sub1
static void updateBoneChainDynamics(s3DModelAnimData* pAnimData, s_runtimeAnimData*& curEntry,
                                     sVec3_FP& prevVec18, sVec3_FP& accumDelta, int count) {
    for (int i = 0; i < count; i++) {
        updateAnimationMatricesSub2Sub1(curEntry, curEntry->m_vec_18, prevVec18);
        curEntry->m0_root -= accumDelta;
        curEntry->m0_root += curEntry->m24_rootDelta;
        updateAnimationMatricesSub2Sub2(curEntry);
        updateAnimationMatricesSub2Sub3(curEntry);
        applyBoneRotation(pAnimData, curEntry);
        accumDelta += curEntry->m24_rootDelta;
        curEntry->m24_rootDelta.m0_X = 0;
        curEntry->m24_rootDelta.m4_Y = 0;
        curEntry->m24_rootDelta.m8_Z = 0;
        prevVec18 = curEntry->m_vec_18;
        curEntry++;
    }
}

// 0605435a
void sCampDragon_applyBoneChainMatrices(s3DModelAnimData* pAnimData) {
    s_runtimeAnimData* entries = pAnimData->m8_runtimeAnimData;
    s_runtimeAnimData* curEntry = &entries[1];

    applyBoneChainRotations(pAnimData, curEntry, (s8)pAnimData->count0);

    // Both chains use entries[0].m0_root as the accumulator (modified in place)
    sVec3_FP prevVec18 = entries[0].m_vec_18;
    updateBoneChainDynamics(pAnimData, curEntry, prevVec18, entries[0].m0_root, (s8)pAnimData->count1);

    prevVec18 = entries[0].m_vec_18;
    updateBoneChainDynamics(pAnimData, curEntry, prevVec18, entries[0].m0_root, (s8)pAnimData->count2);
}

// 060360a0
static s32 transformVecGetY(const sVec3_FP& vec, const sMatrix4x3& mat) {
    s64 mac = 0;
    mac += (s64)mat.m[1][0].asS32() * (s64)vec[0].asS32();
    mac += (s64)mat.m[1][1].asS32() * (s64)vec[1].asS32();
    mac += (s64)mat.m[1][2].asS32() * (s64)vec[2].asS32();
    return (s32)(mac >> 16) + mat.m[1][3].asS32();
}

// 06054a3c
s32 sCampDragon_computeNearestHotpointDistance(sCampDragon* pThis) {
    if ((gDragonState->mC_dragonType < 0) || (7 < gDragonState->mC_dragonType)) {
        return 0;
    }

    // Transform F8 hotpoints into view space for collision bodies
    for (int i = 0; i < pThis->mF4; i++) {
        auto& entry = pThis->mF8[i];
        sVec3_FP& hotpoint = gDragonState->m28_dragon3dModel.m44_hotpointData[entry.m0_boneIndex][entry.m1_hotpointIndex];
        transformAndAddVec(hotpoint, entry.m4, cameraProperties2.m28[0]);
    }

    // Find nearest hotpoint Y among the mE0 pairs
    s32 nearest = 0x7FFFFFFF;
    for (int i = 0; i < pThis->mDC_hotpointPairCount; i++) {
        auto& pair = pThis->mE0_hotpointPairs[i];
        sVec3_FP& hotpoint = gDragonState->m28_dragon3dModel.m44_hotpointData[pair.m0_boneIndex][pair.m1_hotpointIndex];
        s32 y = transformVecGetY(hotpoint, cameraProperties2.m28[0]);
        if (y < nearest) {
            nearest = y;
        }
    }
    return nearest;
}

void sCampDragon_Draw(sTownDragon* pThisBase) {
    sCampDragon* pThis = (sCampDragon*)pThisBase;

    if ((-1 < gDragonState->mC_dragonType) && (gDragonState->mC_dragonType < 8))
    {
        updateAnimationMatricesSub1WithScale(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);
        updateAnimationMatricesSub2(&gDragonState->m78_animData);
        sCampDragon_applyBoneChainMatrices(&gDragonState->m78_animData);
        submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &pThis->m58_position, &pThis->m64_rotation, 0);
        if (pThis->mD_drawExtras)
        {
            pThis->mD_drawExtras = 0;
            s32 nearestDist = sCampDragon_computeNearestHotpointDistance(pThis);
            pThis->mD8_heightOffset = pThis->mD8_heightOffset - (nearestDist - pThis->m4C_basePosition.m4_Y.asS32());
        }
        if (enableDebugTask) {
            vdp2DebugPrintSetPosition(1, 0xB);
            vdp2PrintfSmallFont("Cnfdns:%1d", (s32)pThis->mF_affinityLevel - 1);
            vdp2DebugPrintSetPosition(1, 0xC);
            vdp2PrintfSmallFont("cfd :%3d", mainGameState.gameStats.m7C_overallRating);
            vdp2DebugPrintSetPosition(1, 0xD);
            vdp2PrintfSmallFont("exp :%3d", mainGameState.gameStats.m78_exp);
        }
    }
}

void sCampDragon_Delete(sTownDragon* pThis) {
    Unimplemented();
}

sTownObject* createCampDragon(p_workArea parent, sSaturnPtr arg) {
    static const sCampDragon::TypedTaskDefinition definition = {
        &sCampDragon_Init,
        &sCampDragon_Update,
        &sCampDragon_Draw,
        &sCampDragon_Delete,
    };

    return createSubTaskWithArg<sCampDragon, sSaturnPtr>(parent, arg, &definition);
}
