#include "PDS.h"
#include "battleDebris.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "BTL_A3_2/BTL_A3_2_particles.h"
#include "kernel/graphicalObject.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "particleEffect.h"

struct sDebrisBone
{
    sProcessed3dModel* m0_modelData; // pointer to processed model data, nullptr = inactive
    sVec3_FP m4_position;
    sVec3_FP m10_velocity;
    sVec3_FP m1C_rotation;
    sVec3_FP m28_angularVelocity;
    s16 m34_bounceCount;
    // size 0x38
};

struct sBoneDebrisTask : public s_workAreaTemplateWithCopy<sBoneDebrisTask>
{
    u16 m8_numBones;
    sDebrisBone* mC_bones;
    sVec3_FP m10_baseVelocity;
    sVec3_FP m1C_velocityRandomMask;
    s32 m28_gravity;
    npcFileDeleter* m2C_fileBundle;
    sSaturnPtr m30_particleSpriteData;
    s32 m34_rotationRandomMask;
    s16 m38_bounceCountMax;
    s32 m3C_scale;
    s32 m40_scaleDecrement;
    // size 0x44
};

extern void extractEulerRotationFromMatrix(sVec3_FP& outputRotation, const sMatrix4x3& inMatrix);

// 0605f8e8
static void debrisInitBones(sDebrisBone* bones, s_3dModel* model, sModelHierarchy* hierarchy, u16* boneIndex)
{
    sModelHierarchy* current = hierarchy;
    while (true)
    {
        pushCurrentMatrix();

        u16 idx = *boneIndex;
        // Translate/rotate by bone's animation pose
        u32 boneOffset = idx * 0xB4;
        u8* boneData = (u8*)model->m2C_poseData.data();
        if (boneData)
        {
            translateCurrentMatrix(*(sVec3_FP*)(boneData + boneOffset));
            rotateCurrentMatrixYXZ(*(sVec3_FP*)(boneData + boneOffset + 0xC));
        }

        sDebrisBone* bone = &bones[idx];
        if (current->m0_3dModel == nullptr)
        {
            bone->m0_modelData = nullptr;
        }
        else
        {
            bone->m0_modelData = current->m0_3dModel;
            bone->m4_position[0] = pCurrentMatrix->m[0][3];
            bone->m4_position[1] = pCurrentMatrix->m[1][3];
            bone->m4_position[2] = pCurrentMatrix->m[2][3];
            extractEulerRotationFromMatrix(bone->m1C_rotation, *pCurrentMatrix);
        }

        (*boneIndex)++;

        // Recurse into children
        if (current->m4_subNode != nullptr)
        {
            debrisInitBones(bones, model, current->m4_subNode, boneIndex);
        }

        popMatrix();

        // Continue to siblings
        if (current->m8_nextNode == nullptr) break;
        current = current->m8_nextNode;
    }
}

// 060364ac — extract Euler rotation from current matrix
// Simplified: returns zero rotation (exact implementation requires atan2 decomposition)
void extractRotationFromMatrix(sVec3_FP* output)
{
    output->zeroize();
}

// 0605f99a
static void debrisRandomizeVelocities(sBoneDebrisTask* pThis)
{
    for (s32 i = 0; i < pThis->m8_numBones; i++)
    {
        sDebrisBone* bone = &pThis->mC_bones[i];
        if (bone->m0_modelData == nullptr) continue;

        bone->m10_velocity = pThis->m10_baseVelocity;
        bone->m10_velocity[0] += (s32)(randomNumber() & (u32)pThis->m1C_velocityRandomMask[0]) - ((s32)pThis->m1C_velocityRandomMask[0] >> 1);
        bone->m10_velocity[1] += (s32)(randomNumber() & (u32)pThis->m1C_velocityRandomMask[1]) - ((s32)pThis->m1C_velocityRandomMask[1] >> 1);
        bone->m10_velocity[2] += (s32)(randomNumber() & (u32)pThis->m1C_velocityRandomMask[2]) - ((s32)pThis->m1C_velocityRandomMask[2] >> 1);

        bone->m28_angularVelocity[0] = (s32)(randomNumber() & pThis->m34_rotationRandomMask) - (pThis->m34_rotationRandomMask >> 1);
        bone->m28_angularVelocity[1] = (s32)(randomNumber() & pThis->m34_rotationRandomMask) - (pThis->m34_rotationRandomMask >> 1);
        bone->m28_angularVelocity[2] = (s32)(randomNumber() & pThis->m34_rotationRandomMask) - (pThis->m34_rotationRandomMask >> 1);

        if (pThis->m38_bounceCountMax < 1)
        {
            bone->m34_bounceCount = 0;
        }
        else
        {
            bone->m34_bounceCount = (s16)performModulo2(pThis->m38_bounceCountMax, randomNumber());
        }
    }
}

// 0605fa8e
static void boneDebris_update(sBoneDebrisTask* pThis)
{
    bool anyActive = false;

    for (s32 i = 0; i < pThis->m8_numBones; i++)
    {
        sDebrisBone* bone = &pThis->mC_bones[i];
        if (bone->m0_modelData == nullptr) continue;

        bone->m10_velocity[1] += pThis->m28_gravity;
        bone->m4_position += bone->m10_velocity;
        bone->m1C_rotation += bone->m28_angularVelocity;

        // Ground collision
        // Ground level at offset 0x204 in the target system
        s32 groundLevel = *(s32*)((u8*)gBattleManager->m10_battleOverlay->mC_targetSystem + 0x204);
        if (bone->m4_position[1] < groundLevel)
        {
            bone->m4_position[1] = groundLevel;
            bone->m34_bounceCount--;
            if (bone->m34_bounceCount < 0)
            {
                bone->m0_modelData = nullptr;
            }
            else
            {
                bone->m10_velocity[1] = -MTH_Mul(0x4CCC, bone->m10_velocity[1]);
            }

            // Spawn particle on ground hit
            if (pThis->m30_particleSpriteData.m_offset != 0)
            {
                static std::vector<sVdp1Quad> debrisParticleQuads;
                debrisParticleQuads = initVdp1Quad(pThis->m30_particleSpriteData);
                createBattleParticle((s_workAreaCopy*)pThis->m2C_fileBundle, &debrisParticleQuads,
                    &bone->m4_position, nullptr, nullptr, 0x10000, nullptr, 0);
            }
        }

        anyActive = true;
    }

    if (!anyActive)
    {
        pThis->getTask()->markFinished();
    }

    pThis->m3C_scale -= pThis->m40_scaleDecrement;
    if (pThis->m3C_scale < 1)
    {
        pThis->getTask()->markFinished();
    }
}

// 0605fbb0
static void boneDebris_draw(sBoneDebrisTask* pThis)
{
    for (s32 i = 0; i < pThis->m8_numBones; i++)
    {
        sDebrisBone* bone = &pThis->mC_bones[i];
        if (bone->m0_modelData == nullptr) continue;

        pushCurrentMatrix();
        translateCurrentMatrix(bone->m4_position);
        rotateCurrentMatrixYXZ(bone->m1C_rotation);
        scaleCurrentMatrixRow0(pThis->m3C_scale);
        scaleCurrentMatrixRow1(pThis->m3C_scale);
        scaleCurrentMatrixRow2(pThis->m3C_scale);
        addObjectToDrawList(bone->m0_modelData);
        popMatrix();
    }
}

// 0605f790
void createBoneDebris(s_workAreaCopy* parent, npcFileDeleter* fileBundle, s_3dModel* model,
    sVec3_FP* position, sVec3_FP* rotation,
    sVec3_FP* velocity, sVec3_FP* acceleration,
    s32 gravity, s32 velocityRandomMask,
    s32 param10, s32 param11, s16 bounceCountMax,
    s32 scale, s16 lifetime)
{
    static const sBoneDebrisTask::TypedTaskDefinition definition = {
        nullptr, &boneDebris_update, &boneDebris_draw, nullptr
    };

    sBoneDebrisTask* pThis = createSubTaskWithCopy<sBoneDebrisTask>(parent, &definition);
    if (!pThis) return;

    pThis->m8_numBones = model->m12_numBones;
    pThis->mC_bones = new sDebrisBone[pThis->m8_numBones]; // TODO: allocateHeapForTask
    if (!pThis->mC_bones)
    {
        pThis->getTask()->markFinished();
        return;
    }
    memset(pThis->mC_bones, 0, pThis->m8_numBones * sizeof(sDebrisBone));

    // Initialize bone positions from model hierarchy
    pushCurrentMatrix();
    initMatrixToIdentity(pCurrentMatrix);
    translateCurrentMatrix(*position);
    rotateCurrentMatrixYXZ(*rotation);

    u16 boneIndex = 0;
    sModelHierarchy* hierarchy = model->m4_pModelFile->getModelHierarchy(model->mC_modelIndexOffset);
    if (hierarchy)
    {
        debrisInitBones(pThis->mC_bones, model, hierarchy, &boneIndex);
    }
    popMatrix();

    // Set parameters
    pThis->m2C_fileBundle = fileBundle;
    pThis->m30_particleSpriteData = sSaturnPtr();

    if (velocity)
        pThis->m10_baseVelocity = *velocity;

    if (acceleration)
        pThis->m1C_velocityRandomMask = *acceleration;
    else
        pThis->m1C_velocityRandomMask = sVec3_FP(0xFFF, 0xFFF, 0xFFF);

    pThis->m28_gravity = gravity;
    pThis->m34_rotationRandomMask = velocityRandomMask;
    pThis->m38_bounceCountMax = bounceCountMax;
    pThis->m3C_scale = scale;

    if (lifetime < 1)
        pThis->m40_scaleDecrement = 0;
    else
        pThis->m40_scaleDecrement = FP_Div(scale, (s32)lifetime << 16);

    debrisRandomizeVelocities(pThis);
}

// 0605c27c
static void debrisInitBonesZeroRotation(sDebrisBone* bones, s_3dModel* model, sModelHierarchy* hierarchy, u16* boneIndex)
{
    sModelHierarchy* current = hierarchy;
    while (true)
    {
        pushCurrentMatrix();

        u16 idx = *boneIndex;
        u32 boneOffset = idx * 0xB4;
        u8* boneData = (u8*)model->m2C_poseData.data();
        if (boneData)
        {
            translateCurrentMatrix(*(sVec3_FP*)(boneData + boneOffset));
            rotateCurrentMatrixYXZ(*(sVec3_FP*)(boneData + boneOffset + 0xC));
        }

        sDebrisBone* bone = &bones[idx];
        if (current->m0_3dModel == nullptr)
        {
            bone->m0_modelData = nullptr;
        }
        else
        {
            bone->m0_modelData = current->m0_3dModel;
            bone->m4_position[0] = pCurrentMatrix->m[0][3];
            bone->m4_position[1] = pCurrentMatrix->m[1][3];
            bone->m4_position[2] = pCurrentMatrix->m[2][3];
            extractRotationFromMatrix(&bone->m1C_rotation); // 060364ac — zeroes rotation
        }

        (*boneIndex)++;

        if (current->m4_subNode != nullptr)
        {
            debrisInitBonesZeroRotation(bones, model, current->m4_subNode, boneIndex);
        }

        popMatrix();

        if (current->m8_nextNode == nullptr) break;
        current = current->m8_nextNode;
    }
}

// 0605c422
static void boneDebrisExplosion_update(sBoneDebrisTask* pThis)
{
    bool anyActive = false;

    for (s32 i = 0; i < pThis->m8_numBones; i++)
    {
        sDebrisBone* bone = &pThis->mC_bones[i];
        if (bone->m0_modelData == nullptr) continue;

        bone->m10_velocity[1] += pThis->m28_gravity;
        bone->m4_position += bone->m10_velocity;
        bone->m1C_rotation += bone->m28_angularVelocity;

        s32 groundLevel = *(s32*)((u8*)gBattleManager->m10_battleOverlay->mC_targetSystem + 0x204);
        if (bone->m4_position[1] < groundLevel)
        {
            bone->m4_position[1] = groundLevel;
            bone->m34_bounceCount--;
            if (bone->m34_bounceCount < 0)
            {
                bone->m0_modelData = nullptr;
            }
            else
            {
                bone->m10_velocity[1] = -MTH_Mul(0x4CCC, bone->m10_velocity[1]);
            }
            if (pThis->m30_particleSpriteData.m_offset != 0)
            {
                createParticleEffect(pThis->m2C_fileBundle, nullptr, &bone->m4_position, nullptr, nullptr, 0x10000, 0, 0);
            }
        }

        anyActive = true;
    }

    if (!anyActive)
    {
        pThis->getTask()->markFinished();
    }

    pThis->m3C_scale -= pThis->m40_scaleDecrement;
    if (pThis->m3C_scale < 1)
    {
        pThis->getTask()->markFinished();
    }
}

// 0605c544
static void boneDebrisExplosion_draw(sBoneDebrisTask* pThis)
{
    for (s32 i = 0; i < pThis->m8_numBones; i++)
    {
        sDebrisBone* bone = &pThis->mC_bones[i];
        if (bone->m0_modelData == nullptr) continue;

        pushCurrentMatrix();
        translateCurrentMatrix(bone->m4_position);
        rotateCurrentMatrixYXZ(bone->m1C_rotation);
        scaleCurrentMatrixRow0(pThis->m3C_scale);
        scaleCurrentMatrixRow1(pThis->m3C_scale);
        scaleCurrentMatrixRow2(pThis->m3C_scale);
        addObjectToDrawList(bone->m0_modelData);
        popMatrix();
    }
}

// 0605c124
void createBoneDebrisExplosion(s_workAreaCopy* parent, s_3dModel* model,
    sVec3_FP* position, sVec3_FP* rotation,
    sVec3_FP* velBase, sVec3_FP* velRandom,
    s32 gravity, npcFileDeleter* particleBundle, s32 particleData,
    s32 angVelRandom, s16 bounceCountMax, s32 scale, s16 lifetime)
{
    static const sBoneDebrisTask::TypedTaskDefinition definition = {
        nullptr, &boneDebrisExplosion_update, &boneDebrisExplosion_draw, nullptr
    };

    sBoneDebrisTask* pThis = createSubTaskWithCopy<sBoneDebrisTask>(parent, &definition);
    if (!pThis) return;

    pThis->m8_numBones = model->m12_numBones;
    pThis->mC_bones = static_cast<sDebrisBone*>(allocateHeapForTask(pThis, pThis->m8_numBones * sizeof(sDebrisBone)));
    if (!pThis->mC_bones)
    {
        pThis->getTask()->markFinished();
        return;
    }
    memset(pThis->mC_bones, 0, pThis->m8_numBones * sizeof(sDebrisBone));

    pushCurrentMatrix();
    initMatrixToIdentity(pCurrentMatrix);
    translateCurrentMatrix(*position);
    rotateCurrentMatrixYXZ(*rotation);

    u16 boneIndex = 0;
    sModelHierarchy* hierarchy = model->m4_pModelFile->getModelHierarchy(model->mC_modelIndexOffset);
    if (hierarchy)
    {
        debrisInitBonesZeroRotation(pThis->mC_bones, model, hierarchy, &boneIndex);
    }
    popMatrix();

    pThis->m2C_fileBundle = particleBundle;
    pThis->m30_particleSpriteData.m_offset = particleData;
    pThis->m30_particleSpriteData.m_file = nullptr;

    if (velBase)
        pThis->m10_baseVelocity = *velBase;
    else
        pThis->m10_baseVelocity = sVec3_FP(0, 0, 0);

    if (velRandom)
        pThis->m1C_velocityRandomMask = *velRandom;
    else
        pThis->m1C_velocityRandomMask = sVec3_FP(0xFFF, 0xFFF, 0xFFF);

    pThis->m28_gravity = gravity;
    pThis->m34_rotationRandomMask = angVelRandom;
    pThis->m38_bounceCountMax = bounceCountMax;
    pThis->m3C_scale = scale;

    if (lifetime < 1)
        pThis->m40_scaleDecrement = 0;
    else
        pThis->m40_scaleDecrement = FP_Div(scale, (s32)lifetime << 16);

    debrisRandomizeVelocities(pThis);
}
