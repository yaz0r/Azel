#include "PDS.h"
#include "excaEntity.h"
#include "town/town.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "town/ruin/twn_ruin.h" // TODO: Cleanup

struct sExcaEntity0 : public s_workAreaTemplateWithArgWithCopy<sExcaEntity0, sSaturnPtr>, sTownObject
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sExcaEntity0::Init, &sExcaEntity0::Update, nullptr, &sExcaEntity0::Delete };
        return &taskDefinition;
    }

    static void Init(sExcaEntity0* pThis, sSaturnPtr arg)
    {
        pThis->mC = arg;
        pThis->mC4_position = readSaturnVec3(arg + 8);
        pThis->mD0_rotation = readSaturnVec3(arg + 0x14);
        pThis->mDC_status = readSaturnU8(arg + 0x26);

        s16 npcIndex = readSaturnS16(arg + 0x2c);
        if (npcIndex > -1) {
            npcData0.m70_npcPointerArray[npcIndex].workArea = pThis;
        }
    }

    static void Update(sExcaEntity0* pThis)
    {
        s32 fileIndex = readSaturnU32(pThis->mC);
        if (isDataLoaded(fileIndex))
        {
            s16 modelIndex = readSaturnS16(pThis->mC + 0x20);
            s16 animationIndex = readSaturnS16(pThis->mC + 0x22);
            s16 poseIndex = readSaturnS16(pThis->mC + 0x24);

            sModelHierarchy* pHierarchy = pThis->m0_fileBundle->getModelHierarchy(modelIndex);
            sStaticPoseData* pPose = pThis->m0_fileBundle->getStaticPose(poseIndex, pHierarchy->countNumberOfBones());

            if (animationIndex == 0)
            {
                init3DModelRawData(pThis, &pThis->m10_3dModel, 0, pThis->m0_fileBundle, modelIndex, nullptr, pPose, nullptr, nullptr);
            }
            else
            {
                sAnimationData* pAnimation = pThis->m0_fileBundle->getAnimation(animationIndex);
                init3DModelRawData(pThis, &pThis->m10_3dModel, 0, pThis->m0_fileBundle, modelIndex, pAnimation, pPose, nullptr, nullptr);
                stepAnimation(&pThis->m10_3dModel);
            }

            sSaturnPtr scriptConfigEA = readSaturnEA(pThis->mC + 0x28);
            if (!scriptConfigEA.isNull())
            {
                pThis->m60_scriptContext.m30_pPosition = &pThis->mC4_position;
                pThis->m60_scriptContext.m34_pRotation = &pThis->mD0_rotation;
                pThis->m60_scriptContext.m38_pOwner = pThis;
                pThis->m60_scriptContext.m3C_scriptEA = readSaturnEA(scriptConfigEA + 4);

                s16 sVar1 = readSaturnS16(scriptConfigEA + 2) >> 0xF;
                u8* puVar3 = nullptr;
                if (sVar1)
                {
                    puVar3 = pThis->mC.getRawPointer() + sVar1;
                }

                pThis->m60_scriptContext.m40 = puVar3;

                mainLogicInitSub0(&pThis->m60_scriptContext, readSaturnU8(scriptConfigEA + 0));
                mainLogicInitSub1(&pThis->m60_scriptContext, readSaturnVec3(scriptConfigEA + 0x8), readSaturnVec3(scriptConfigEA + 0x14));
            }

            pThis->m_UpdateMethod = &sExcaEntity0::Update2;
            pThis->m_DrawMethod = &sExcaEntity0::Draw2;
        }
    }

    static void Update2(sExcaEntity0* pThis)
    {
        switch (pThis->mDC_status)
        {
        case 0:
            break;
        case 1:
        {
            stepAnimation(&pThis->m10_3dModel);
            int numFrames = 0;
            if (pThis->m10_3dModel.m30_pCurrentAnimation)
            {
                numFrames = pThis->m10_3dModel.m30_pCurrentAnimation->m4_numFrames;
            }
            if (numFrames - 1 <= pThis->m10_3dModel.m16_previousAnimationFrame)
            {
                pThis->mDC_status = 0;
            }
            break;
        }
        case 2:
            stepAnimation(&pThis->m10_3dModel);
            break;
        case 3:
            assert(0); //delete
            break;
        default:
            assert(0);
            break;
        }

        // don't update if there is no script config
        if (readSaturnEA(pThis->mC + 0x28).isNull())
            return;

        EdgeUpdateSub0(&pThis->m60_scriptContext);
    }

    static void Draw2(sExcaEntity0* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pThis->mC4_position);
        rotateCurrentMatrixZYX(pThis->mD0_rotation);
        pThis->m10_3dModel.m18_drawFunction(&pThis->m10_3dModel);
        popMatrix();
    }

    static void Delete(sExcaEntity0* pThis)
    {
        Unimplemented();
    }

    sSaturnPtr mC;
    s_3dModel m10_3dModel;
    sMainLogic_74 m60_scriptContext;
    sVec3_FP mC4_position;
    sVec3_FP mD0_rotation;
    u8 mDC_status;
    //size: 0xE0
};

sTownObject* createExcaEntity(s_workAreaCopy* parent, sSaturnPtr arg) {
    return createSubTaskWithArgWithCopy<sExcaEntity0, sSaturnPtr>(parent, arg);
}
