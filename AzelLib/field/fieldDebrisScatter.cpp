#include "PDS.h"
#include "fieldDebrisScatter.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "audio/systemSounds.h"

// Per-bone debris data (0x38 bytes each on Saturn)
struct sDebrisBoneData
{
    sProcessed3dModel* m0_model;
    sVec3_FP m4_position;
    sVec3_FP m10_velocity;
    sVec3_FP m1C_rotation;
    sVec3_FP m28_rotVelocity;
    s32 m34_state; // -1 = inactive, 0 = flying, 1 = landed
};

struct sDebrisScatterTask : public s_workAreaTemplate<sDebrisScatterTask>
{
    // 0607dc00
    static void Update(sDebrisScatterTask* pThis)
    {
        for (s32 i = 0; i < pThis->m16_numBones; i++)
        {
            sDebrisBoneData& bone = pThis->m_bones[i];
            if (bone.m0_model == nullptr || bone.m34_state < 0)
                continue;

            // Apply gravity
            bone.m10_velocity.m4_Y = bone.m10_velocity.m4_Y - pThis->m0_gravity;

            // Integrate position
            bone.m4_position.m0_X = bone.m4_position.m0_X + bone.m10_velocity.m0_X;
            bone.m4_position.m4_Y = bone.m4_position.m4_Y + bone.m10_velocity.m4_Y;
            bone.m4_position.m8_Z = bone.m4_position.m8_Z + bone.m10_velocity.m8_Z;

            // Integrate rotation
            bone.m1C_rotation.m0_X = bone.m1C_rotation.m0_X + bone.m28_rotVelocity.m0_X;
            bone.m1C_rotation.m4_Y = bone.m1C_rotation.m4_Y + bone.m28_rotVelocity.m4_Y;
            bone.m1C_rotation.m8_Z = bone.m1C_rotation.m8_Z + bone.m28_rotVelocity.m8_Z;

            // Ground collision
            if (bone.m4_position.m4_Y < pThis->m8_groundY)
            {
                if (bone.m34_state == 0)
                {
                    playSystemSoundEffect(pThis->m14_soundEffect);
                    bone.m34_state = 1;
                }
                bone.m4_position.m4_Y = pThis->m8_groundY;
                fixedPoint bounced = MTH_Mul(bone.m10_velocity.m4_Y, pThis->m4_bounce);
                if (bounced < 0)
                    bone.m10_velocity.m4_Y = 0;
                else
                    bone.m10_velocity.m4_Y = bounced;
            }

            // Dampen rotation
            bone.m28_rotVelocity.m0_X = MTH_Mul(0xFD70, bone.m28_rotVelocity.m0_X);
            bone.m28_rotVelocity.m4_Y = MTH_Mul(0xFD70, bone.m28_rotVelocity.m4_Y);
            bone.m28_rotVelocity.m8_Z = MTH_Mul(0xFD70, bone.m28_rotVelocity.m8_Z);
        }
    }

    // 0607dd16
    static void Draw(sDebrisScatterTask* pThis)
    {
        bool allEmpty = true;
        for (s32 i = 0; i < pThis->m16_numBones; i++)
        {
            sDebrisBoneData& bone = pThis->m_bones[i];
            if (bone.m0_model != nullptr)
            {
                pushCurrentMatrix();
                translateCurrentMatrix(bone.m4_position);
                rotateCurrentMatrixYXZ(bone.m1C_rotation);
                addObjectToDrawList(bone.m0_model);
                popMatrix();
                allEmpty = false;
            }
        }
        if (allEmpty)
        {
            pThis->getTask()->markFinished();
        }
    }

    fixedPoint m0_gravity;
    fixedPoint m4_bounce;
    fixedPoint m8_groundY;
    fixedPoint mC_spread;
    s32 m10_callback; // function pointer for ground hit callback
    s16 m14_soundEffect;
    s16 m16_numBones;
    std::vector<sDebrisBoneData> m_bones;
};

// 0607e136 — extract Euler rotation from matrix
static void extractRotationFromMatrix(sVec3_FP& outRotation, const sMatrix4x3& mat)
{
    outRotation.m0_X = atan_FP(MTH_Mul(mat.m[2][0], -0x10000));
    outRotation.m4_Y = atan2_FP(mat.m[0][2], mat.m[2][2]);
    outRotation.m8_Z = atan2_FP(mat.m[1][0], mat.m[1][1]);
}

// 0607e190 — recursively initialize bones from model hierarchy
static void initBonesFromHierarchy(
    sDebrisScatterTask* pTask,
    sModelHierarchy* pNode,
    u16& boneIndex,
    const sMatrix4x3& parentMatrix,
    s_3dModel* pModel)
{
    sMatrix4x3 localMatrix;
    copyMatrix(const_cast<sMatrix4x3*>(&parentMatrix), &localMatrix);

    // Apply bone's pose transform
    if (pModel && boneIndex < pModel->m2C_poseData.size())
    {
        translateMatrix(pModel->m2C_poseData[boneIndex].m0_translation, &localMatrix);
        rotateMatrixZYX(&pModel->m2C_poseData[boneIndex].mC_rotation, &localMatrix);
    }

    sDebrisBoneData& bone = pTask->m_bones[boneIndex];
    bone.m0_model = pNode->m0_3dModel;
    if (bone.m0_model)
    {
        // Position = translation column of the combined matrix
        bone.m4_position.m0_X = localMatrix.m[0][3];
        bone.m4_position.m4_Y = localMatrix.m[1][3];
        bone.m4_position.m8_Z = localMatrix.m[2][3];

        extractRotationFromMatrix(bone.m1C_rotation, localMatrix);
    }

    boneIndex++;

    if (pNode->m4_subNode)
    {
        initBonesFromHierarchy(pTask, pNode->m4_subNode, boneIndex, localMatrix, pModel);
    }
    if (pNode->m8_nextNode)
    {
        initBonesFromHierarchy(pTask, pNode->m8_nextNode, boneIndex, parentMatrix, pModel);
    }
}

// 0607e252
p_workArea createDebrisScatterTask(p_workArea parent, sDebrisScatterParams* params, bool reverseMode)
{
    s_fileBundle* pBundle = params->m_pBundle;
    if (!pBundle)
        return nullptr;

    sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(params->m34_modelOffset);
    s32 numBones = pHierarchy->countNumberOfBones();

    static const sDebrisScatterTask::TypedTaskDefinition definition = {
        nullptr, &sDebrisScatterTask::Update, &sDebrisScatterTask::Draw, nullptr
    };

    sDebrisScatterTask* pTask = createSubTask<sDebrisScatterTask>(parent, &definition);
    if (!pTask)
        return nullptr;

    pTask->m0_gravity = params->m0_gravity;
    pTask->m4_bounce = params->m4_bounce;
    pTask->m8_groundY = params->m38_groundY;
    pTask->mC_spread = params->mC_randomMask;
    pTask->m10_callback = 0;
    pTask->m14_soundEffect = params->m44_soundEffect;
    pTask->m16_numBones = (s16)numBones;
    pTask->m_bones.resize(numBones);
    memset(pTask->m_bones.data(), 0, numBones * sizeof(sDebrisBoneData));

    // Get pose data for initial bone positions
    s_3dModel* pModel = nullptr;
    // The parent task should have a s_3dModel — find it
    // For the crashed ship, the model is at offset 0x40 in the parent struct
    // We pass it through the params indirectly via position/rotation

    // Build initial transform matrix
    sMatrix4x3 rootMatrix;
    if (params->m14_pRotation == nullptr)
    {
        // Copy matrix from position (already a matrix)
        initMatrixToIdentity(&rootMatrix);
        if (params->m10_pPosition)
        {
            rootMatrix.m[0][3] = params->m10_pPosition->m0_X;
            rootMatrix.m[1][3] = params->m10_pPosition->m4_Y;
            rootMatrix.m[2][3] = params->m10_pPosition->m8_Z;
        }
    }
    else
    {
        initMatrixToIdentity(&rootMatrix);
        translateMatrix(*params->m10_pPosition, &rootMatrix);
        rotateMatrixYXZ(params->m14_pRotation, &rootMatrix);
    }

    // Initialize bone positions from hierarchy
    u16 boneIdx = 0;
    // We need the parent's s_3dModel for pose data — try to get it
    // For now, init bones without pose (positions will be at origin)
    initBonesFromHierarchy(pTask, pHierarchy, boneIdx, rootMatrix, nullptr);

    if (!reverseMode)
    {
        // Explosion mode — set random outward velocities per bone
        for (s32 i = 0; i < numBones; i++)
        {
            sDebrisBoneData& bone = pTask->m_bones[i];
            bone.m34_state = -1;
            if (bone.m0_model != nullptr)
            {
                s32 mask = params->mC_randomMask.asS32();
                bone.m28_rotVelocity.m0_X = (s32)(randomNumber() & mask) - (mask >> 1);
                bone.m28_rotVelocity.m4_Y = (s32)(randomNumber() & mask) - (mask >> 1);
                bone.m28_rotVelocity.m8_Z = (s32)(randomNumber() & mask) - (mask >> 1);

                bone.m28_rotVelocity.m0_X = bone.m28_rotVelocity.m0_X + params->m24_rotVelX;
                bone.m28_rotVelocity.m4_Y = bone.m28_rotVelocity.m4_Y + params->m28_rotVelY;
                bone.m28_rotVelocity.m8_Z = bone.m28_rotVelocity.m8_Z + params->m2C_rotVelZ;

                // Compute outward velocity based on angle from center
                sVec3_FP delta;
                delta.m0_X = bone.m4_position.m0_X - params->m10_pPosition->m0_X;
                delta.m8_Z = bone.m4_position.m8_Z - params->m10_pPosition->m8_Z;

                fixedPoint dist = fixedPoint(sqrt_I(
                    FP_Pow2(delta.m0_X) + FP_Pow2(delta.m8_Z)));
                if (dist == 0) dist = 0x10;

                fixedPoint speed = FP_Div(
                    fixedPoint(sqrt_I(MTH_Mul(params->m8_spread, params->m0_gravity) * 4)),
                    dist);

                s32 rng = randomNumber() & 3;
                static const fixedPoint speedVariation[] = { 0x10000, 0xE000, 0xC000, 0x14000 };
                speed = MTH_Mul(speed, speedVariation[rng]);

                bone.m10_velocity.m4_Y = speed;
                bone.m10_velocity.m0_X = params->m18_velX;
                bone.m10_velocity.m8_Z = params->m20_velZ;

                // Distribute bones at unique angles
                fixedPoint angle = atan2_FP(delta.m8_Z, delta.m0_X);
                s32 angleInt = (angle.asS32() & 0xFFFFFFF);
                s32 sinIdx = (angleInt >> 16) & 0xFFF;
                bone.m10_velocity.m0_X = bone.m10_velocity.m0_X + MTH_Mul(speed, getSin(sinIdx));
                bone.m10_velocity.m8_Z = bone.m10_velocity.m8_Z + MTH_Mul(speed, getCos(sinIdx));

                bone.m34_state = 0;
            }
        }
    }

    return pTask;
}
