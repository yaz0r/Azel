#include "PDS.h"
#include "town.h"
#include "townScript.h"
#include "townLCS.h"
#include "town/ruin/twn_ruin.h"
#include "kernel/cinematicBarsTask.h"
#include "kernel/fileBundle.h"

#include <map>

sResData resData;

s32 resValue0 = 0;
s32 resValue1 = 0;
sScriptTask* currentResTask = nullptr;

struct sScriptUpdateSub0Sub0Var0
{
    sVec3_FP m0_position;
    s32 mC_distance;
    fixedPoint m10_y;
    //size 0x14
};

std::array< sScriptUpdateSub0Sub0Var0, 12> scriptUpdateSub0Sub0Var0;

struct sScriptUpdateSub0Sub0Var1
{
    s32 m0;
    s32 m4;
    s32 m8;
    s32 mC;
    //size 0x10?
}scriptUpdateSub0Sub0Var1;


void scriptUpdateSub0Sub0(sMainLogic_74* r4)
{
    const sVec3_FP& r12 = *r4->m34_pRotation;

    switch (r4->m0_collisionType)
    {
    case 0:
    case 1:
        for (int i = 0; i < 12; i++)
        {
            scriptUpdateSub0Sub0Var0[i].mC_distance = 0;
        }
        scriptUpdateSub0Sub0Var1.m0 = 0;
        scriptUpdateSub0Sub0Var1.m4 = 0;
        scriptUpdateSub0Sub0Var1.m8 = 0;
        scriptUpdateSub0Sub0Var1.mC = 0;
    case 2:
        r4->m48 = 0;
    case 3:
        r4->m44 = 0;
        break;
    default:
        assert(0);
        break;
    }

    pushCurrentMatrix();

    initMatrixToIdentity(pCurrentMatrix);
    pCurrentMatrix->matrix[3] = r4->m14_collisionClip[0];
    pCurrentMatrix->matrix[7] = r4->m14_collisionClip[1];
    pCurrentMatrix->matrix[11] = r4->m14_collisionClip[2];

    rotateCurrentMatrixShiftedZ(-r12[2]);
    rotateCurrentMatrixShiftedX(-r12[0]);
    rotateCurrentMatrixShiftedY(-r12[1]);

    sVec3_FP var0;
    var0[0] = -r4->m8_position[0] - resValue0;
    var0[1] = -r4->m8_position[1];
    var0[2] = -r4->m8_position[2] - resValue0;

    translateCurrentMatrix(var0);
}

void addBackgroundScript(sSaturnPtr r4, s32 r5, p_workArea r6, const sVec3_S16_12_4* r7)
{
    if (r4.m_offset == 0)
        return;

    if (npcData0.m0_numBackgroundScripts >= 4)
        return;

    std::array<sRunningScriptContext, 4>::iterator r14 = npcData0.m4_backgroundScripts.begin();
    if (npcData0.m0_numBackgroundScripts)
    {
        for(s32 r13 = npcData0.m0_numBackgroundScripts; r13 > 0; r13--)
        {
            if (r14->m0_scriptPtr == r4)
            {
                if (r14->m4 == r5)
                {
                    return;
                }
            }
            r14++;
        }
    }

    r14->m0_scriptPtr = r4;
    r14->m4 = r5;
    r14->m8_owner.setTask(r6);
    if (r7)
    {
        r14->mC = *r7;
    }

    npcData0.m0_numBackgroundScripts++;
}

s32 scriptUpdateSub0Sub1(sMainLogic_74* r13, sMainLogic_74* r14)
{
    if (r14->m40)
    {
        assert(0);
    }
    else
    {
        if (distanceSquareBetween2Points(r13->m8_position, r14->m8_position) >= FP_Pow2(r13->m4_collisionRadius + r13->m4_collisionRadius))
            return 0;

        //0600874E
        assert(0);
    }

    return 0;
}

// Z-
void scriptUpdateSub0Sub2Sub0(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[2]) - r13->m14_collisionClip[2];

    sScriptUpdateSub0Sub0Var0* r4;
    if (r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[5];
        r13->m44 |= 0x1;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[11];
        r13->m44 |= 0x100;
    }

    if (r4->mC_distance < r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 - (*r6)[2];
    }
}

// Z+
void scriptUpdateSub0Sub2Sub1(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[2]) + r13->m14_collisionClip[2];

    sScriptUpdateSub0Sub0Var0* r4;
    if (r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[4];
        r13->m44 |= 0x2;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[10];
        r13->m44 |= 0x200;
    }

    if (r4->mC_distance < r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 + (*r6)[2];
    }
}

void scriptUpdateSub0Sub2Sub2(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[0]) - r13->m14_collisionClip[0];

    sScriptUpdateSub0Sub0Var0* r4;
    if (r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[1];
        r13->m44 |= 0x10;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[7];
        r13->m44 |= 0x1000;
    }

    if (r4->mC_distance > r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 - (*r6)[0];
    }
}

// CollisionX+
void scriptUpdateSub0Sub2Sub3(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[0]) + r13->m14_collisionClip[0];

    sScriptUpdateSub0Sub0Var0* r4;
    if (r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[0];
        r13->m44 |= 0x20;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[6];
        r13->m44 |= 0x2000;
    }

    if (r4->mC_distance > r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 + (*r6)[0];
    }
}

// CollisionY-
void scriptUpdateSub0Sub2Sub4(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[1]) - r13->m14_collisionClip[1];

    sScriptUpdateSub0Sub0Var0* r4;
    if(r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[4];
        r13->m44 |= 0x8;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[9];
        r13->m44 |= 0x800;
    }

    if (r4->mC_distance > r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 - (*r6)[1];
    }
}

void scriptUpdateSub0Sub2Sub5(sMainLogic_74* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[1]) + r13->m14_collisionClip[1];

    sScriptUpdateSub0Sub0Var0* r4;
    if (r12 < 0)
    {
        r4 = &scriptUpdateSub0Sub0Var0[2];
        r13->m44 |= 0x4;
    }
    else
    {
        r4 = &scriptUpdateSub0Sub0Var0[8];
        r13->m44 |= 0x400;
    }

    if (r4->mC_distance < r5)
    {
        r4->m0_position = *r14;
        r4->mC_distance = r5;
        r4->m10_y = r12 + (*r6)[1];
    }
}

void scriptUpdateSub0Sub2(sMainLogic_74* r4, fixedPoint r5)
{
    sVec3_FP var4;
    var4[0] = pCurrentMatrix->matrix[1];
    var4[1] = pCurrentMatrix->matrix[5];
    var4[2] = pCurrentMatrix->matrix[9];

    sVec3_FP var10;
    var10[0] = MTH_Mul(var4[0], r4->m14_collisionClip[0]);
    var10[1] = MTH_Mul(var4[1], r4->m14_collisionClip[1]);
    var10[2] = MTH_Mul(var4[2], r4->m14_collisionClip[2]);

    fixedPoint r14 = r5;
    r14 += MTH_Mul(var4[0], pCurrentMatrix->matrix[3]);
    r14 += MTH_Mul(var4[1], pCurrentMatrix->matrix[7]);
    r14 += MTH_Mul(var4[2], pCurrentMatrix->matrix[11]);
    r14 -= var10[0];
    r14 -= var10[1];
    r14 -= var10[2];

    switch (r4->m0_collisionType)
    {
    case 0:
    case 1:
        // 6009474
        if (FP_Pow2(r14) < FP_Pow2(var10[2]))
        {
            if (r14 > var10[2])
            {
                scriptUpdateSub0Sub2Sub0(r4, r14, &var10, &var4);
            }
            else
            {
                scriptUpdateSub0Sub2Sub1(r4, r14, &var10, &var4);
            }
        }
        //60094AA
        if (FP_Pow2(r14) < FP_Pow2(var10[0]))
        {
            if (r14 > var10[0])
            {
                scriptUpdateSub0Sub2Sub2(r4, r14, &var10, &var4);
            }
            else
            {
                scriptUpdateSub0Sub2Sub3(r4, r14, &var10, &var4);
            }
        }
        //60094E0
        if (FP_Pow2(r14) < FP_Pow2(var10[1]))
        {
            if (r14 > var10[1])
            {
                scriptUpdateSub0Sub2Sub4(r4, r14, &var10, &var4);
            }
            else
            {
                scriptUpdateSub0Sub2Sub5(r4, r14, &var10, &var4);
            }
        }
        break;
    default:
        assert(0);
    }
}

s32 scriptUpdateSub0Sub3Sub1(fixedPoint r4_x, fixedPoint r5_z)
{
    s32 r8 = MTH_Mul32(r4_x, gTownGrid.m30_worldToCellIndex * 2) & 1;
    s32 r0 = MTH_Mul32(r5_z, gTownGrid.m30_worldToCellIndex * 4) & 2;

    return r8 + r0;
}

sTownCellTask* scriptUpdateSub0Sub3Sub0(fixedPoint r4_x, fixedPoint r5_z)
{
    if (compareTrace)
    {
        addTraceLog("scriptUpdateSub0Sub3Sub0 testing cell: 0x%04X 0x%04X\n", r4_x.asS32(), r5_z.asS32());
    }

    s32 r13_cellX = MTH_Mul32(r4_x, gTownGrid.m30_worldToCellIndex);
    s32 r5_cellY = MTH_Mul32(r5_z, gTownGrid.m30_worldToCellIndex);

    if (r13_cellX < 0)
        return nullptr;

    if (r13_cellX >= gTownGrid.m0_sizeX)
        return nullptr;

    if (r5_cellY < 0)
        return nullptr;

    if (r5_cellY >= gTownGrid.m4_sizeY)
        return nullptr;

    r13_cellX -= gTownGrid.m10_currentX;
    r5_cellY -= gTownGrid.m14_currentY;

    if (r13_cellX < -3)
        return nullptr;

    if (r13_cellX > 3)
        return nullptr;

    if (r5_cellY < -3)
        return nullptr;

    if (r5_cellY > 3)
        return nullptr;

    if (compareTrace)
    {
        addTraceLog("scriptUpdateSub0Sub3Sub0 found cell. GridSize: %d %d\n", gTownGrid.m0_sizeX, gTownGrid.m4_sizeY);
    }

    return gTownGrid.m40_cellTasks[(gTownGrid.mC + r5_cellY) & 7][(gTownGrid.m8 + r13_cellX) & 7];
}

void convertVerticeTo16(std::array<fixedPoint, 4 * 3>::iterator& r1, std::array<sVec3_S16, 3>::iterator& r2)
{
    r2->m_value[0] = r1[0] >> 4;
    r2->m_value[1] = r1[1] >> 4;
    r2->m_value[2] = r1[2] >> 4;

    r2++;
    r1 += 3;
}

struct sProcessedVertice
{
    sVec3_FP vertice;
    u32 clipFlag;
};

s32 transformTownMeshVertices(const std::vector<sVec3_S16_12_4>& r4_vertices, std::array<sProcessedVertice, 255>& r5_outputConverted, s32 r6_numVertices, const sVec3_FP& r7_clip)
{
    std::array<fixedPoint, 4 * 3>::iterator r1 = pCurrentMatrix->matrix.begin();
    std::array<sVec3_S16, 3> rotationMatrix;
    std::array<sVec3_S16, 3>::iterator r2 = rotationMatrix.begin();

    convertVerticeTo16(r1, r2);
    s32 r12_X = *r1++;
    convertVerticeTo16(r1, r2);
    s32 r13_Y = *r1++;
    convertVerticeTo16(r1, r2);
    s32 r14_Z = *r1++;

    r12_X <<= 8;
    r13_Y <<= 8;
    r14_Z <<= 8;

    s32 r9_clipX = r7_clip[0] * 2;
    s32 r10_clipY = r7_clip[1] * 2;
    s32 r11_clipZ = r7_clip[2] * 2;

    std::vector<sVec3_S16_12_4>::const_iterator r4 = r4_vertices.begin();
    std::array<sProcessedVertice, 255>::iterator r5 = r5_outputConverted.begin();

    s32 r7 = 0x3F;

    do 
    {
        s32 r1 = 0;
        r2 = rotationMatrix.begin();

        // X
        s64 mac = r12_X;
        mac += (s64)r4->m_value[0] * (s64)r2->m_value[0];
        mac += (s64)r4->m_value[1] * (s64)r2->m_value[1];
        mac += (s64)r4->m_value[2] * (s64)r2->m_value[2];
        r2++;
        r1 <<= 1;
        if (0 >= mac)
        {
            mac >>= 8;
            mac |= 0xFF000000;
            r1 |= 1;
        }
        else
        {
            mac >>= 8;
        }
        r5->vertice[0] = mac;
        r1 <<= 1;
        if (mac >= r9_clipX)
        {
            r1 |= 1;
        }

        // Y
        mac = r13_Y;
        mac += (s64)r4->m_value[0] * (s64)r2->m_value[0];
        mac += (s64)r4->m_value[1] * (s64)r2->m_value[1];
        mac += (s64)r4->m_value[2] * (s64)r2->m_value[2];
        r2++;
        r1 <<= 1;
        if (0 >= mac)
        {
            mac >>= 8;
            mac |= 0xFF000000;
            r1 |= 1;
        }
        else
        {
            mac >>= 8;
        }
        r5->vertice[1] = mac;
        r1 <<= 1;
        if (mac >= r10_clipY)
        {
            r1 |= 1;
        }

        // Z
        mac = r14_Z;
        mac += (s64)r4->m_value[0] * (s64)r2->m_value[0];
        mac += (s64)r4->m_value[1] * (s64)r2->m_value[1];
        mac += (s64)r4->m_value[2] * (s64)r2->m_value[2];
        r2++;
        r1 <<= 1;
        if (0 >= mac)
        {
            mac >>= 8;
            mac |= 0xFF000000;
            r1 |= 1;
        }
        else
        {
            mac >>= 8;
        }
        r5->vertice[2] = mac;
        r1 <<= 1;
        if (mac >= r11_clipZ)
        {
            r1 |= 1;
        }

        r5->clipFlag = r1;

        r7 &= r1;
        r5++;
        r4++;
    } while (--r6_numVertices);

    return r7;
}

void transformNormalByCurrentMatrix(const sVec3_S16_12_4& normal, sVec3_FP& output)
{
    sVec3_FP temp;
    temp[0] = normal[0] << 4;
    temp[1] = normal[1] << 4;
    temp[2] = normal[2] << 4;

    transformVecByCurrentMatrix(temp, output);
}

s32 testTownMeshQuadForCollisionSub1(const fixedPoint& r4, const fixedPoint& r5, const fixedPoint& r6, const fixedPoint& r7)
{
    s64 mac = (s64)r4.asS32() * (s64)r5.asS32();
    mac -= (s64)r6.asS32() * (s64)r7.asS32();

    return mac >> 32;
}

void testTownMeshQuadForCollision(sMainLogic_74* r14, const sProcessed3dModel::sQuad& r5_quad, const std::array<sProcessedVertice, 255>& r6_vertices)
{
    const sProcessedVertice& r13_vertice0 = r6_vertices[r5_quad.m0_indices[0]];
    const sProcessedVertice& r10_vertice1 = r6_vertices[r5_quad.m0_indices[1]];
    const sProcessedVertice& r9_vertice2 = r6_vertices[r5_quad.m0_indices[2]];
    const sProcessedVertice& r11_vertice3 = r6_vertices[r5_quad.m0_indices[3]];

    // are all vertices clipped in one common axis?
    if (r13_vertice0.clipFlag & r10_vertice1.clipFlag & r9_vertice2.clipFlag & r11_vertice3.clipFlag)
        return;

    sVec3_FP varC;
    transformNormalByCurrentMatrix(r5_quad.m14_extraData[0].m0_normals, varC);

    sVec3_FP var18;
    var18[0] = MTH_Mul(varC[0], r14->m14_collisionClip[0]);
    var18[1] = MTH_Mul(varC[1], r14->m14_collisionClip[1]);
    var18[2] = MTH_Mul(varC[2], r14->m14_collisionClip[2]);

    fixedPoint var0 = MTH_Product3d_FP(varC, r13_vertice0.vertice) - var18[0] - var18[1] - var18[2];

    if (FP_Pow2(var18[2]) > FP_Pow2(var0))
    {
        //6008D50
        if (var0 > var18[2])
        {
            //06008D58
            if ((testTownMeshQuadForCollisionSub1(
                    r13_vertice0.vertice[0] - r14->m14_collisionClip[0],
                    r10_vertice1.vertice[1] - r14->m14_collisionClip[1],
                    r10_vertice1.vertice[0] - r14->m14_collisionClip[0],
                    r13_vertice0.vertice[1] - r14->m14_collisionClip[1]) >= 0) &&
                //6008D7E
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[0] - r14->m14_collisionClip[0],
                    r9_vertice2.vertice[1] - r14->m14_collisionClip[1],
                    r9_vertice2.vertice[0] - r14->m14_collisionClip[0],
                    r10_vertice1.vertice[1] - r14->m14_collisionClip[1]) >= 0) &&
                //6008DA0
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[0] - r14->m14_collisionClip[0],
                    r11_vertice3.vertice[1] - r14->m14_collisionClip[1],
                    r11_vertice3.vertice[0] - r14->m14_collisionClip[0],
                    r9_vertice2.vertice[1] - r14->m14_collisionClip[1]) >= 0) &&
                //6008DC2
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[0] - r14->m14_collisionClip[0],
                    r13_vertice0.vertice[1] - r14->m14_collisionClip[1],
                    r13_vertice0.vertice[0] - r14->m14_collisionClip[0],
                    r11_vertice3.vertice[1] - r14->m14_collisionClip[1]) >= 0))
            {
                //6008DE4
                switch (r14->m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript(readSaturnEA(npcData0.m64_scriptList + r5_quad.m12_onCollisionScriptIndex * 4), 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
                    }
                case 1:
                    if ((r5_quad.m10_CMDSRCA & 0xF) == 0)
                    {
                        scriptUpdateSub0Sub2Sub0(r14, var0, &var18, &varC);
                    }
                    break;
                default:
                    r14->m44 |= 0x1;
                    break;
                }
            }
        }
        else
        {
            //06008E40
            if ((testTownMeshQuadForCollisionSub1(
                    r13_vertice0.vertice[0] - r14->m14_collisionClip[0],
                    r10_vertice1.vertice[1] - r14->m14_collisionClip[1],
                    r10_vertice1.vertice[0] - r14->m14_collisionClip[0],
                    r13_vertice0.vertice[1] - r14->m14_collisionClip[1]) <= 0) &&
                //06008E62
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[0] - r14->m14_collisionClip[0],
                    r9_vertice2.vertice[1] - r14->m14_collisionClip[1],
                    r9_vertice2.vertice[0] - r14->m14_collisionClip[0],
                    r10_vertice1.vertice[1] - r14->m14_collisionClip[1]) <= 0) &&
                //06008E80
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[0] - r14->m14_collisionClip[0],
                    r11_vertice3.vertice[1] - r14->m14_collisionClip[1],
                    r11_vertice3.vertice[0] - r14->m14_collisionClip[0],
                    r9_vertice2.vertice[1] - r14->m14_collisionClip[1]) <= 0) &&
                //06008E9E
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[0] - r14->m14_collisionClip[0],
                    r13_vertice0.vertice[1] - r14->m14_collisionClip[1],
                    r13_vertice0.vertice[0] - r14->m14_collisionClip[0],
                    r11_vertice3.vertice[1] - r14->m14_collisionClip[1]) <= 0))
            {
                //06008EBC
                switch (r14->m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript(readSaturnEA(npcData0.m64_scriptList + r5_quad.m12_onCollisionScriptIndex * 4), 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
                    }
                case 1:
                    if ((r5_quad.m10_CMDSRCA & 0xF) == 0)
                    {
                        scriptUpdateSub0Sub2Sub1(r14, var0, &var18, &varC);
                    }
                    break;
                default:
                    r14->m44 |= 0x2;
                    break;
                }
            }
        }
    }
    
    //6008F32
    if (FP_Pow2(var18[0]) > FP_Pow2(var0))
    {
        //6008F4E
        if (var0 > var18[0])
        {
            //6008F56
            if ((testTownMeshQuadForCollisionSub1(
                    r13_vertice0.vertice[1] - r14->m14_collisionClip[1],
                    r10_vertice1.vertice[2] - r14->m14_collisionClip[2],
                    r10_vertice1.vertice[1] - r14->m14_collisionClip[1],
                    r13_vertice0.vertice[2] - r14->m14_collisionClip[2]) >= 0) &&
                //6008F7C
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[1] - r14->m14_collisionClip[1],
                    r9_vertice2.vertice[2] - r14->m14_collisionClip[2],
                    r9_vertice2.vertice[1] - r14->m14_collisionClip[1],
                    r10_vertice1.vertice[2] - r14->m14_collisionClip[2]) >= 0) &&
                //6008F9E
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[1] - r14->m14_collisionClip[1],
                    r11_vertice3.vertice[2] - r14->m14_collisionClip[2],
                    r11_vertice3.vertice[1] - r14->m14_collisionClip[1],
                    r9_vertice2.vertice[2] - r14->m14_collisionClip[2]) >= 0) &&
                //6008FC0
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[1] - r14->m14_collisionClip[1],
                    r13_vertice0.vertice[2] - r14->m14_collisionClip[2],
                    r13_vertice0.vertice[1] - r14->m14_collisionClip[1],
                    r11_vertice3.vertice[2] - r14->m14_collisionClip[2]) >= 0))
            {
                //6008FE2
                switch (r14->m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript(readSaturnEA(npcData0.m64_scriptList + r5_quad.m12_onCollisionScriptIndex * 4), 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
                    }
                case 1:
                    if ((r5_quad.m10_CMDSRCA & 0xF) == 0)
                    {
                        scriptUpdateSub0Sub2Sub2(r14, var0, &var18, &varC);
                    }
                    break;
                default:
                    r14->m44 |= 0x10;
                    break;
                }
            }
        }
        else
        {
            //600903E
            if ((testTownMeshQuadForCollisionSub1(
                    r13_vertice0.vertice[1] - r14->m14_collisionClip[1],
                    r10_vertice1.vertice[2] - r14->m14_collisionClip[2],
                    r10_vertice1.vertice[1] - r14->m14_collisionClip[1],
                    r13_vertice0.vertice[2] - r14->m14_collisionClip[2]) <= 0) &&
                //6009060
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[1] - r14->m14_collisionClip[1],
                    r9_vertice2.vertice[2] - r14->m14_collisionClip[2],
                    r9_vertice2.vertice[1] - r14->m14_collisionClip[1],
                    r10_vertice1.vertice[2] - r14->m14_collisionClip[2]) <= 0) &&
                //0600907E
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[1] - r14->m14_collisionClip[1],
                    r11_vertice3.vertice[2] - r14->m14_collisionClip[2],
                    r11_vertice3.vertice[1] - r14->m14_collisionClip[1],
                    r9_vertice2.vertice[2] - r14->m14_collisionClip[2]) <= 0) &&
                //0600909C
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[1] - r14->m14_collisionClip[1],
                    r13_vertice0.vertice[2] - r14->m14_collisionClip[2],
                    r13_vertice0.vertice[1] - r14->m14_collisionClip[1],
                    r11_vertice3.vertice[2] - r14->m14_collisionClip[2]) <= 0))
            {
                //060090BA
                switch (r14->m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript(readSaturnEA(npcData0.m64_scriptList + r5_quad.m12_onCollisionScriptIndex * 4), 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
                    }
                case 1:
                    if ((r5_quad.m10_CMDSRCA & 0xF) == 0)
                    {
                        scriptUpdateSub0Sub2Sub3(r14, var0, &var18, &varC);
                    }
                    break;
                default:
                    r14->m44 |= 0x20;
                    break;
                }
            }
        }
    }

    //6009122
    if (FP_Pow2(var18[1]) > FP_Pow2(var0))
    {
        //600913E
        if (var0 > var18[1])
        {
            //6009146
            if ((testTownMeshQuadForCollisionSub1(
                    r13_vertice0.vertice[2] - r14->m14_collisionClip[2],
                    r10_vertice1.vertice[0] - r14->m14_collisionClip[0],
                    r10_vertice1.vertice[2] - r14->m14_collisionClip[2],
                    r13_vertice0.vertice[0] - r14->m14_collisionClip[0]) >= 0) &&
                //600916C
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[2] - r14->m14_collisionClip[2],
                    r9_vertice2.vertice[0] - r14->m14_collisionClip[0],
                    r9_vertice2.vertice[2] - r14->m14_collisionClip[2],
                    r10_vertice1.vertice[0] - r14->m14_collisionClip[0]) >= 0) &&
                //600918E
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[2] - r14->m14_collisionClip[2],
                    r11_vertice3.vertice[0] - r14->m14_collisionClip[0],
                    r11_vertice3.vertice[2] - r14->m14_collisionClip[2],
                    r9_vertice2.vertice[0] - r14->m14_collisionClip[0]) >= 0) &&
                //60091B0
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[2] - r14->m14_collisionClip[2],
                    r13_vertice0.vertice[0] - r14->m14_collisionClip[0],
                    r13_vertice0.vertice[2] - r14->m14_collisionClip[2],
                    r11_vertice3.vertice[0] - r14->m14_collisionClip[0]) >= 0))
            {
                //60091D2
                switch (r14->m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript(readSaturnEA(npcData0.m64_scriptList + r5_quad.m12_onCollisionScriptIndex * 4), 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
                    }
                case 1:
                    if ((r5_quad.m10_CMDSRCA & 0xF) == 0)
                    {
                        scriptUpdateSub0Sub2Sub4(r14, var0, &var18, &varC);
                    }
                    break;
                default:
                    r14->m44 |= 8;
                    break;
                }
            }
        }
        else
        {
            //600922E
            if ((testTownMeshQuadForCollisionSub1(
                    r13_vertice0.vertice[2] - r14->m14_collisionClip[2],
                    r10_vertice1.vertice[0] - r14->m14_collisionClip[0],
                    r10_vertice1.vertice[2] - r14->m14_collisionClip[2],
                    r13_vertice0.vertice[0] - r14->m14_collisionClip[0]) <= 0) &&
                //06009250
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[2] - r14->m14_collisionClip[2],
                    r9_vertice2.vertice[0] - r14->m14_collisionClip[0],
                    r9_vertice2.vertice[2] - r14->m14_collisionClip[2],
                    r10_vertice1.vertice[0] - r14->m14_collisionClip[0]) <= 0) &&
                //0600926E
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[2] - r14->m14_collisionClip[2],
                    r11_vertice3.vertice[0] - r14->m14_collisionClip[0],
                    r11_vertice3.vertice[2] - r14->m14_collisionClip[2],
                    r9_vertice2.vertice[0] - r14->m14_collisionClip[0]) <= 0) &&
                //0600928C
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[2] - r14->m14_collisionClip[2],
                    r13_vertice0.vertice[0] - r14->m14_collisionClip[0],
                    r13_vertice0.vertice[2] - r14->m14_collisionClip[2],
                    r11_vertice3.vertice[0] - r14->m14_collisionClip[0]) <= 0))
            {
                //060092AA
                switch (r14->m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript(readSaturnEA(npcData0.m64_scriptList + r5_quad.m12_onCollisionScriptIndex * 4), 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
                    }
                case 1:
                    if ((r5_quad.m10_CMDSRCA & 0xF) == 0)
                    {
                        scriptUpdateSub0Sub2Sub5(r14, var0, &var18, &varC);
                    }
                    break;
                default:
                    r14->m44 |= 4;
                    break;
                }
            }
        }
    }
}

void processTownMeshCollision(sMainLogic_74* r4, const sProcessed3dModel* r5)
{
    if (compareTrace)
    {
        addTraceLog("processTownMeshCollision: current matrix: ");
        for (int i = 0; i < 4 * 3; i++)
        {
            addTraceLog("0x%08X ", pCurrentMatrix->matrix[i]);
        }
        addTraceLog("\n");
    }
    std::array<sProcessedVertice, 255> transformedVertices;
    u32 clipFlag = transformTownMeshVertices(r5->m8_vertices, transformedVertices, r5->m4_numVertices, r4->m14_collisionClip);

    if (compareTrace)
    {
        addTraceLog("processTownMeshCollision: vertices: ");
        for (int i = 0; i < r5->m4_numVertices; i++)
        {
            addTraceLog("0x%08X 0x%08X 0x%08X 0x%08X, ", transformedVertices[i].vertice[0].asS32(), transformedVertices[i].vertice[1].asS32(), transformedVertices[i].vertice[2].asS32(), transformedVertices[i].clipFlag);
        }
        addTraceLog("\n");
    }

    if(clipFlag)
    {
        // because all vertices were clipped on one axis
        return;
    }

    std::vector<sProcessed3dModel::sQuad>::const_iterator quadIterator = r5->mC_Quads.begin();
    if (r4->m2C == 0)
    {
        while (quadIterator != r5->mC_Quads.end())
        {
            if ((quadIterator->m10_CMDSRCA & 0xF00) == 0)
            {
                testTownMeshQuadForCollision(r4, *quadIterator, transformedVertices);
            }
            quadIterator++;
        }
    }
    else
    {
        while (quadIterator != r5->mC_Quads.end())
        {
            testTownMeshQuadForCollision(r4, *quadIterator, transformedVertices);
            quadIterator++;
        }
    }
}

void scriptUpdateSub0Sub3Sub2(sMainLogic_74* r12, sTownCellTask* r13)
{
    if (r13 == nullptr)
        return;

    if (r13->m8.m_offset == 0)
        return;

    if (readSaturnU32(r13->m8 + 0x14) == 0)
        return;

    pushCurrentMatrix();
    {
        translateCurrentMatrix(r13->mC_position);

        sVec3_FP var0_positionInCell;
        var0_positionInCell = r12->m8_position - r13->mC_position;

        sSaturnPtr r14 = readSaturnEA(r13->m8 + 0x14);
        while (readSaturnU32(r14))
        {
            sProcessed3dModel* mesh = r13->m0_dramAllocation->get3DModel(readSaturnU32(r14));
            sVec3_FP meshPositionInCell = readSaturnVec3(r14 + 4);
            if (distanceSquareBetween2Points(var0_positionInCell, meshPositionInCell) < FP_Pow2(mesh->m0_radius + r12->m4_collisionRadius))
            {
                pushCurrentMatrix();
                translateCurrentMatrix(meshPositionInCell);
                processTownMeshCollision(r12, mesh);
                popMatrix();
            }

            r14 += 0x10;
        }
    }
    popMatrix();
}

void handleCollisionWithTownEnv(sMainLogic_74* r4)
{
    s32 type = scriptUpdateSub0Sub3Sub1(r4->m8_position[0], r4->m8_position[2]);
    switch (type)
    {
    case 0:
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0] - gTownGrid.m28_cellSize, r4->m8_position[2]));
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0], r4->m8_position[2] - gTownGrid.m28_cellSize));
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0] - gTownGrid.m28_cellSize, r4->m8_position[2] - gTownGrid.m28_cellSize));
        break;
    case 1:
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0] + gTownGrid.m28_cellSize, r4->m8_position[2]));
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0], r4->m8_position[2] - gTownGrid.m28_cellSize));
        scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0] + gTownGrid.m28_cellSize, r4->m8_position[2] - gTownGrid.m28_cellSize));
        break;
    default:
        assert(0);
        break;
    }

    scriptUpdateSub0Sub3Sub2(r4, scriptUpdateSub0Sub3Sub0(r4->m8_position[0], r4->m8_position[2]));
}

void scriptUpdateSub0Sub4Sub0(sMainLogic_74* r12)
{
    std::array< sScriptUpdateSub0Sub0Var0, 12>& r13 = scriptUpdateSub0Sub0Var0;

    r12->m44 &= ~(r12->m44 << 8);

    if ((r12->m44 & 0x400) && (r12->m44 & 0x8))
    {
        //06007C86
        assert(0);
    }
    else if ((r12->m44 & 0x800) && (r12->m44 & 0x4))
    {
        //06007CBA
        assert(0);
    }
    else if ((r12->m44 & 0x200) && !(r12->m44 & 0x1))
    {
        //06007D24
        r12->m44 |= 2;
        r13[4] = r13[10];
    }
    else if ((r12->m44 & 0x100) && !(r12->m44 & 0x2))
    {
        //06007D5E
        r12->m44 |= 1;
        r13[5] = r13[11];
    }
    else if ((r12->m44 & 0x2000) && !(r12->m44 & 0x10))
    {
        //06007D98
        r12->m44 |= 0x20;
        r13[0] = r13[6];
    }
    else if ((r12->m44 & 0x1000) && !(r12->m44 & 0x20))
    {
        //06007DD6
        r12->m44 |= 0x10;
        r13[0] = r13[7];
    }

    //6007DF8
    sVec3_FP var14;
    var14.zero();

    sVec3_FP var8;
    sVec3_FP& r9 = var8;

    //const sScriptUpdateSub0Sub0Var0& r10 = scriptUpdateSub0Sub0Var0[0];
    //const sScriptUpdateSub0Sub0Var0& r8 = scriptUpdateSub0Sub0Var0[1];
    //const sScriptUpdateSub0Sub0Var0& var0 = scriptUpdateSub0Sub0Var0[4];
    //const sScriptUpdateSub0Sub0Var0& var4 = scriptUpdateSub0Sub0Var0[5];

    //06007E1A
    switch (r12->m44 & 0x33)
    {
    case 0:
        break;
    case 1: //
    {
        //6007F24
        fixedPoint r10 = FP_Div(scriptUpdateSub0Sub0Var0[5].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[5].m0_position[1])));
        var14[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[5].m0_position[0], r10);
        var14[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[5].m0_position[2], r10);
        break;
    }
    case 2: // Y-
    {
        //6007EFA
        fixedPoint r10 = FP_Div(scriptUpdateSub0Sub0Var0[4].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[4].m0_position[1])));
        var14[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[4].m0_position[0], r10);
        var14[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[4].m0_position[2], r10);
        break;
    }
    case 0x10:
    {
        //6007ED4
        fixedPoint r10 = FP_Div(scriptUpdateSub0Sub0Var0[1].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[1].m0_position[1])));
        var14[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[1].m0_position[0], r10);
        var14[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[1].m0_position[2], r10);
        break;
    }
    case 0x20: // X+
    {
        //6007E94
        fixedPoint r10 = FP_Div(scriptUpdateSub0Sub0Var0[0].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[0].m0_position[1])));
        var14[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[0], r10);
        var14[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[2], r10);
        break;
    }
    case 0x11:
        //600814C
        if ((scriptUpdateSub0Sub0Var0[0].m0_position[2] <= 0) && (scriptUpdateSub0Sub0Var0[5].m0_position[0] <= 0))
        {
            //0600815A
            fixedPoint r8 = FP_Div(scriptUpdateSub0Sub0Var0[0].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[0].m0_position[1])));
            var14[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[0], r8);
            var14[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[2], r8);

            //06008182
            {
                fixedPoint r8 = FP_Div(scriptUpdateSub0Sub0Var0[5].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[5].m0_position[1])));
                r9[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[5].m0_position[0], r8);
                r9[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[5].m0_position[2], r8);
            }

            if (var14[0] <= r9[0])
            {
                var14[0] = r9[0];
            }

            if (var14[2] > r9[2])
            {
                var14[2] = r9[2];
            }
        }
        else
        {
            //600807A
            var14[2] = scriptUpdateSub0Sub0Var0[4].mC_distance;
            var14[0] = scriptUpdateSub0Sub0Var0[1].mC_distance - MTH_Mul(FP_Div(scriptUpdateSub0Sub0Var0[1].m0_position[2], scriptUpdateSub0Sub0Var0[1].m0_position[0]), var14[2]);
        }
        break;
    case 0x12:
        //6007FF8
        if ((scriptUpdateSub0Sub0Var0[0].m0_position[2] >= 0) && (scriptUpdateSub0Sub0Var0[4].m0_position[0] <= 0))
        {
            //06008006
            fixedPoint r8 = FP_Div(scriptUpdateSub0Sub0Var0[0].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[0].m0_position[1])));
            var14[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[0], r8);
            var14[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[2], r8);

            //0600802E
            {
                fixedPoint r8 = FP_Div(scriptUpdateSub0Sub0Var0[4].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[4].m0_position[1])));
                r9[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[4].m0_position[0], r8);
                r9[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[4].m0_position[2], r8);
            }

            if (var14[0] > r9[0])
            {
                var14[0] = r9[0];
            }

            if (var14[2] < r9[2])
            {
                var14[2] = r9[2];
            }
        }
        else
        {
            //600807A
            var14[2] = scriptUpdateSub0Sub0Var0[4].mC_distance;
            var14[0] = scriptUpdateSub0Sub0Var0[1].mC_distance - MTH_Mul(FP_Div(scriptUpdateSub0Sub0Var0[1].m0_position[2], scriptUpdateSub0Sub0Var0[1].m0_position[0]), var14[2]);
        }
        break;
    case 0x21:
        //0600809C
        if ((scriptUpdateSub0Sub0Var0[0].m0_position[2] >= 0) && (scriptUpdateSub0Sub0Var0[5].m0_position[0] <= 0))
        {
            //060080AA
            fixedPoint r8 = FP_Div(scriptUpdateSub0Sub0Var0[0].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[0].m0_position[1])));
            var14[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[0], r8);
            var14[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[2], r8);

            //060080D2
            {
                fixedPoint r8 = FP_Div(scriptUpdateSub0Sub0Var0[5].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[5].m0_position[1])));
                r9[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[5].m0_position[0], r8);
                r9[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[5].m0_position[2], r8);
            }

            if (var14[0] < r9[0])
            {
                var14[0] = r9[0];
            }

            if (var14[2] > r9[2])
            {
                var14[2] = r9[2];
            }
        }
        else
        {
            //600811E
            assert(0);
        }
        break;
    case 0x22:
        //6007F56
        if ((scriptUpdateSub0Sub0Var0[0].m0_position[2] >= 0) && (scriptUpdateSub0Sub0Var0[4].m0_position[0] >= 0))
        {
            //06007F64
            fixedPoint r8 = FP_Div(scriptUpdateSub0Sub0Var0[0].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[0].m0_position[1])));
            var14[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[0], r8);
            var14[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[2], r8);

            //6007F8A
            {
                fixedPoint r8 = FP_Div(scriptUpdateSub0Sub0Var0[4].m10_y, sqrt_F(0x10000 - FP_Pow2(scriptUpdateSub0Sub0Var0[4].m0_position[1])));
                r9[0] = MTH_Mul(scriptUpdateSub0Sub0Var0[4].m0_position[0], r8);
                r9[2] = MTH_Mul(scriptUpdateSub0Sub0Var0[4].m0_position[2], r8);
            }

            if (var14[0] < r9[0])
            {
                var14[0] = r9[0];
            }

            if (var14[2] < r9[2])
            {
                var14[2] = r9[2];
            }
        }
        else
        {
            //06007FD8
            var14[2] = scriptUpdateSub0Sub0Var0[4].mC_distance;
            var14[0] = scriptUpdateSub0Sub0Var0[0].mC_distance - MTH_Mul(FP_Div(scriptUpdateSub0Sub0Var0[0].m0_position[2], scriptUpdateSub0Sub0Var0[0].m0_position[0]), var14[2]);
        }
        break;
    case 0x32:
        //0600821E
        var14[2] = FP_Div(
            MTH_Mul_5_6(scriptUpdateSub0Sub0Var0[0].mC_distance - scriptUpdateSub0Sub0Var0[1].mC_distance, scriptUpdateSub0Sub0Var0[0].m0_position[0], scriptUpdateSub0Sub0Var0[1].m0_position[0]),
            MTH_Mul(scriptUpdateSub0Sub0Var0[0].m0_position[2], scriptUpdateSub0Sub0Var0[1].m0_position[0]) - MTH_Mul(scriptUpdateSub0Sub0Var0[1].m0_position[2], scriptUpdateSub0Sub0Var0[0].m0_position[0]));

        if (var14[2] < scriptUpdateSub0Sub0Var0[4].mC_distance)
        {
            var14[2] = scriptUpdateSub0Sub0Var0[4].mC_distance;
        }

        //600829A
        var14[0] = scriptUpdateSub0Sub0Var0[0].mC_distance - MTH_Mul(FP_Div(scriptUpdateSub0Sub0Var0[0].m0_position[2], scriptUpdateSub0Sub0Var0[0].m0_position[0]), var14[2]);
        break;
    default:
        //assert(0);
        PDS_Log("Unhandled collision case 0x%X!", r12->m44 & 0x33)
        break;
    }

    //6008446
    const sScriptUpdateSub0Sub0Var0& r10 = r13[3];
    switch (r12->m44 & 0xC)
    {
    case 0:
        break;
    case 4:
        var14[1] = r13[2].mC_distance;
        break;
    case 8:
        var14[1] = r13[3].mC_distance;
        break;
    default:
        assert(0);
        break;
    }

    //0600854A
    if ((scriptUpdateSub0Sub0Var1.m0 != 0) || (scriptUpdateSub0Sub0Var1.m4 != 0))
    {
        assert(0);
    }

    //060085A4
    if ((scriptUpdateSub0Sub0Var1.m8 != 0) || (scriptUpdateSub0Sub0Var1.mC != 0))
    {
        assert(0);
    }

    //06008604
    if (r12->m44 & 4)
    {
        r12->m4C = r13[2].m0_position;
    }

    //6008622
    if (var14[0] > r12->m14_collisionClip[0] / 2)
    {
        var14[0] = r12->m14_collisionClip[0] / 2;
    }

    if (var14[0] < -r12->m14_collisionClip[0] / 2)
    {
        var14[0] = -r12->m14_collisionClip[0] / 2;
    }

    if (var14[1] > r12->m14_collisionClip[1] / 2)
    {
        var14[1] = r12->m14_collisionClip[1] / 2;
    }

    if (var14[1] < -r12->m14_collisionClip[1] / 2)
    {
        var14[1] = -r12->m14_collisionClip[1] / 2;
    }

    if (var14[2] > r12->m14_collisionClip[2] / 2)
    {
        var14[2] = r12->m14_collisionClip[2] / 2;
    }

    if (var14[2] < -r12->m14_collisionClip[2] / 2)
    {
        var14[2] = -r12->m14_collisionClip[2] / 2;
    }

    //6008668
    sMatrix4x3 var20;
    initMatrixToIdentity(&var20);
    rotateMatrixYXZ(r12->m34_pRotation, &var20);
    transformVec(var14, r12->m58_collisionSolveTranslation, var20);
}

void scriptUpdateSub0Sub4(sMainLogic_74* r4)
{
    popMatrix();

    switch (r4->m0_collisionType)
    {
    case 0:
    case 1:
        scriptUpdateSub0Sub4Sub0(r4);
        break;
    case 2:
    case 3:
        break;
    default:
        assert(0);
        break;
    }
}

void scriptUpdateSub0()
{
    for (int r8 = 4-1; r8 >= 0; r8--)
    {
        sResData1C* r11 = resData.m8_headOfLinkedList[r8];
        while (r11)
        {
            sMainLogic_74* r14 = r11->m4;
            r11 = r11->m0_pNext;

            if (r14->m2_collisionLayersBitField)
            {
                scriptUpdateSub0Sub0(r14);
                s32 r9 = r8 + 1;
                do 
                {
                    if (r14->m2_collisionLayersBitField & (1 << r9))
                    {
                        sResData1C* r12 = resData.m8_headOfLinkedList[r9];
                        while (r12)
                        {
                            sMainLogic_74* r13 = r12->m4;
                            r12 = r12->m0_pNext;
                            if (scriptUpdateSub0Sub1(r14, r13))
                            {
                                if ((r14->m0_collisionType == 0) && r13->m3C_scriptEA.m_offset)
                                {
                                    //06007B16
                                    addBackgroundScript(r13->m3C_scriptEA, 1, r13->m38_pOwner, 0);
                                }

                                r14->m48 = r13;
                                r13->m48 = r14;
                                if (r14->m0_collisionType >= 2)
                                {
                                    goto endOfLoop;
                                }
                            }
                        }
                    }
                } while (++r9 < 5);
                //06007B3C
                
                if (r14->m2_collisionLayersBitField & 0x10)
                {
                    scriptUpdateSub0Sub2(r14, resData.m0);
                }

                handleCollisionWithTownEnv(r14);

            endOfLoop:
                //06007B4E
                scriptUpdateSub0Sub4(r14);
            }
        }
    }
}

void addTraceLog(const char* fmt, ...)
{
    static FILE* fHandle = nullptr;
    if (fHandle == nullptr)
    {
        fHandle = fopen("azelLog.txt", "r");
        assert(fHandle);
    }

    char buffer[1024];

    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    char buffer2[1024];
    fread(buffer2, 1, strlen(buffer), fHandle);
    buffer2[strlen(buffer)] = 0;

    printf(buffer);
    assert(strcmp(buffer2, buffer) == 0);
}

void initResTable()
{
    if (compareTrace)
    {
        addTraceLog("InitResTable: resValue0=%d\n", resValue0);
    }
    resData.m8_headOfLinkedList.fill(0);
    resData.m4 = 0;

    if (resValue0 < 0)
    {
        resValue0 = 2;
    }
    else
    {
        resValue0 = -2;
    }
}

s32 setSomethingInNpc0(s32 arg0, s32 arg1)
{
    fixedPoint value;
    switch (arg1)
    {
    case 0:
        value = 0x7D0000;
        break;
    case 1:
        value = 0x5000;
        break;
    case 2:
        value = 0x2000;
        break;
    case 3:
        value = -1;
        break;
    default:
        assert(0);
    }

    if (arg0 != 1)
    {
        npcData0.m54_activationNear = value;
    }
    else
    {
        npcData0.m58_activationFar = value;
    }

    return 0;
}

sNPC* getNpcDataByIndex(s32 r4)
{
    if (r4 == -1)
    {
        return npcData0.m104_currentScript.m8_owner.getNPC();
    }
    
    return npcData0.m70_npcPointerArray[r4].pNPC;
}

s32 setNpcLocation(s32 r4_npcIndex, s32 r5_X, s32 r6_Y, s32 r7_Z)
{
    sNPC* pNPC = getNpcDataByIndex(r4_npcIndex);
    if (pNPC)
    {
        pNPC->mE8.m0_position[0] = r5_X;
        pNPC->mE8.m0_position[1] = r6_Y;
        pNPC->mE8.m0_position[2] = r7_Z;
    }

    return 0;
}

s32 setNpcOrientation(s32 r4_npcIndex, s32 r5_X, s32 r6_Y, s32 r7_Z)
{
    sNPC* pNPC = getNpcDataByIndex(r4_npcIndex);
    if (pNPC)
    {
        pNPC->mE8.mC_rotation[0] = r5_X;
        pNPC->mE8.mC_rotation[1] = r6_Y;
        pNPC->mE8.mC_rotation[2] = r7_Z;
    }

    return 0;
}

sKernelScriptFunctions gKernelScriptFunctions =
{
    // zero arg
    {},
    // one arg
    {
        {0x600CCB4, &initNPC},
        {0x602c2ca, &playSoundEffect},
    },
    // one arg ptr
    {
        {0x6014DF2, &initNPCFromStruct}
    },
    // two arg
    {
        {0x600CC78, &setSomethingInNpc0},
    },
    // four arg
    {
        {0x605AEE0, &setNpcLocation},
        {0x605AF0E, &setNpcOrientation},
    },
};

sSaturnPtr callNativeWithArguments(sNpcData* r4_pThis, sSaturnPtr r5)
{
    u8 numArguments = readSaturnU8(r5);
    r5 = r5 + 1;
    sSaturnPtr r14 = r5 + 3;
    r14.m_offset &= ~3;

    sSaturnPtr functionEA = readSaturnEA(r14);

    switch (numArguments)
    {
    case 0:
        if (gKernelScriptFunctions.m_zeroArg.count(functionEA.m_offset))
        {
            scriptFunction_zero_arg& pFunction = gKernelScriptFunctions.m_zeroArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction();
        }
        else if(gCurrentTownOverlay->overlayScriptFunctions.m_zeroArg.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_zero_arg& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_zeroArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction();
        }
        else
        {
            return sSaturnPtr::getNull();
        }
        break;
    case 1:
        if (gKernelScriptFunctions.m_oneArg.count(functionEA.m_offset))
        {
            scriptFunction_one_arg& pFunction = gKernelScriptFunctions.m_oneArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4));
        }
        else if (gKernelScriptFunctions.m_oneArgPtr.count(functionEA.m_offset))
        {
            scriptFunction_one_arg_ptr pFunction = gKernelScriptFunctions.m_oneArgPtr.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnEA(r14 + 4));
        }
        else if (gCurrentTownOverlay->overlayScriptFunctions.m_oneArg.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_one_arg& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_oneArg.find(readSaturnEA(r14).m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4));
        }
        else if (gCurrentTownOverlay->overlayScriptFunctions.m_oneArgPtr.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_one_arg_ptr& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_oneArgPtr.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnEA(r14 + 4));
        }
        else
        {
            return sSaturnPtr::getNull();
        }
        break;
    case 2:
        if (gKernelScriptFunctions.m_twoArg.count(functionEA.m_offset))
        {
            scriptFunction_two_arg pFunction = gKernelScriptFunctions.m_twoArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8));
        }
        else if (gCurrentTownOverlay->overlayScriptFunctions.m_twoArg.count(functionEA.m_offset))
        {
            assert(gCurrentTownOverlay == r14.m_file);
            scriptFunction_two_arg& pFunction = gCurrentTownOverlay->overlayScriptFunctions.m_twoArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8));
        }
        else
        {
            return sSaturnPtr::getNull();
        }
        break;
    case 4:
        if (gKernelScriptFunctions.m_fourArg.count(functionEA.m_offset))
        {
            scriptFunction_four_arg pFunction = gKernelScriptFunctions.m_fourArg.find(functionEA.m_offset)->second;
            r4_pThis->m118_currentResult = pFunction(readSaturnS32(r14 + 4), readSaturnS32(r14 + 8), readSaturnS32(r14 + 12), readSaturnS32(r14 + 16));
        }
        else
        {
            return sSaturnPtr::getNull();
        }
        break;
    default:
        assert(0);
        break;
    }

    return r14 + (numArguments + 1) * 4;
}

sSaturnPtr getAlignOn2(const sSaturnPtr& inPtr)
{
    sSaturnPtr output = inPtr;
    output.m_offset += 1;
    output.m_offset &= ~1;

    return output;
}

sSaturnPtr getAlignOn4(const sSaturnPtr& inPtr)
{
    sSaturnPtr output = inPtr;
    output.m_offset += 3;
    output.m_offset &= ~3;

    return output;
}

sSaturnPtr runScript(sNpcData* r13_pThis)
{
    sRunningScriptContext* varC = &r13_pThis->m104_currentScript;
    sSaturnPtr r14 = r13_pThis->m104_currentScript.m0_scriptPtr;

    do
    {
        u8 r0_opcode = readSaturnU8(r14++);

        if (compareTrace)
        {
            addTraceLog("runScript_interceptOpcode 0x%02X\n", r0_opcode);
        }

        switch (r0_opcode)
        {
        case 1: //end
            if (r13_pThis->m11C_currentStackPointer == r13_pThis->m120_stack.end())
            {
                r14.m_offset = 0;
                return r14;
            }
            else
            {
                r14 = *r13_pThis->m11C_currentStackPointer;
                r13_pThis->m11C_currentStackPointer++;
            }
            break;
        case 2: //wait
        {
            sSaturnPtr r4 = r14 + 1;
            r4.m_offset &= ~1;
            u16 delay = readSaturnU16(r4);
            r4 += 2;

            if (r13_pThis->mF0)
            {
                assert(0);
            }

            r13_pThis->m100 = delay-1;
            return r4;
        }
        case 3: //jump
            r14 = readSaturnEA(getAlignOn4(r14));
            break;
        case 5: //if
            if (r13_pThis->m118_currentResult)
            {
                r14 += 4;
                r14 = getAlignOn4(r14);
            }
            else
            {
                r14 = readSaturnEA(getAlignOn4(r14));
            }
            break;
        case 6: //callScript
            *(--r13_pThis->m11C_currentStackPointer) = getAlignOn4(r14) + 4;
            r14 = readSaturnEA(getAlignOn4(r14));
            break;
        case 7: //callNative
            r14 = callNativeWithArguments(r13_pThis, r14);

            // hack: this isn't in the original assembly, but allow returning null for unhandled functions and just stopping the script
            if (r14.isNull())
                return r14;
            break;
        case 8: //equal
            r14 = getAlignOn2(r14);
            if (readSaturnS16(r14) == r13_pThis->m118_currentResult)
            {
                r13_pThis->m118_currentResult = 1;
            }
            else
            {
                r13_pThis->m118_currentResult = 0;
            }
            r14 += 2;
            break;
        case 9: //not equal
            r14 = getAlignOn2(r14);
            if (readSaturnS16(r14) != r13_pThis->m118_currentResult)
            {
                r13_pThis->m118_currentResult = 1;
            }
            else
            {
                r13_pThis->m118_currentResult = 0;
            }
            r14 += 2;
            break;
        case 12: // less
            r14 = getAlignOn2(r14);
            if (readSaturnS16(r14) > r13_pThis->m118_currentResult)
            {
                r13_pThis->m118_currentResult = 1;
            }
            else
            {
                r13_pThis->m118_currentResult = 0;
            }
            r14 += 2;
            break;
        case 15:
        {
            r14 = getAlignOn2(r14);
            s16 r4 = readSaturnS16(r14);
            r14 += 2;
            if (r4 < 1000)
            {
                r4 += 3334;
            }

            mainGameState.setBit(r4);
            break;
        }
        case 17:
        {
            r14 = getAlignOn2(r14);
            s16 r4 = readSaturnS16(r14);
            r14 += 2;
            if (r4 < 1000)
            {
                r4 += 3334;
            }

            r13_pThis->m118_currentResult = (mainGameState.getBit(r4) != 0);
            break;
        }
        case 18: // read packed bits
        {
            s8 r6 = readSaturnS8(r14++);
            r14 = getAlignOn2(r14);
            s16 var0 = readSaturnS16(r14);
            r14 += 2;
            if (var0 < 1000)
            {
                var0 += 3334;
            }
            r13_pThis->m118_currentResult = mainGameState.readPackedBits(var0, r6);
            break;
        }
        case 21:
        {
            s8 arg = readSaturnS8(r14++);
            r14 = getAlignOn4(r14);
            if ((r13_pThis->m118_currentResult < readSaturnU8(r14)) && (r13_pThis->m118_currentResult >= 0))
            {
                r14 = readSaturnEA(r14 + r13_pThis->m118_currentResult * 4);
            }
            else
            {
                r14 += arg * 4;
            }
            break;
        }
        case 24: //setResult
            r13_pThis->m118_currentResult = varC->m4;
            break;
        case 25: // add cinematic bars
            if (r13_pThis->m164_cinematicBars)
            {
                if (r13_pThis->m164_cinematicBars->m0_status != 1)
                {
                    return --r14;
                }
            }
            else
            {
                r13_pThis->m164_cinematicBars = createCinematicBarTask(currentResTask);
                setupCinematicBars(r13_pThis->m164_cinematicBars, 4);
                return --r14;
            }
            break;
        case 26: // remove cinematic bars
            if (r13_pThis->m164_cinematicBars)
            {
                if (r13_pThis->m164_cinematicBars->m0_status == 1)
                {
                    r13_pThis->m164_cinematicBars->cinematicBarTaskSub0(4);
                    return --r14;
                }
                else if(r13_pThis->m164_cinematicBars->m0_status)
                {
                    return --r14;
                }
                else
                {
                    r13_pThis->m164_cinematicBars->getTask()->markFinished();
                    r13_pThis->m164_cinematicBars = nullptr;
                }
                return r14;
            }
        case 27: // draw string
            r14 = getAlignOn4(r14);
            setupVDP2StringRendering(3, 25, 38, 2);
            VDP2DrawString(readSaturnString(readSaturnEA(r14)).c_str());
            r14 += 4;
            r13_pThis->mF8 = 1;
            break;
        case 29: // clear string
            setupVDP2StringRendering(3, 25, 38, 2);
            clearVdp2TextArea();
            break;
        case 32: // wait fade
        {
            if (!g_fadeControls.m0_fade0.m20_stopped)
            {
                return r14 - 1;
            }
            break;
        }
        case 36: // display string
            if (r13_pThis->m16C_displayStringTask)
            {
                r13_pThis->m16C_displayStringTask->getTask()->markFinished();
                return r14 - 1;
            }
            else
            {
                r14 = getAlignOn2(r14);
                s16 duration = readSaturnS16(r14);
                r14 += 2;
                r14 = getAlignOn4(r14);

                sSaturnPtr stringPtr = readSaturnEA(r14);
                r14 += 4;

                createDisplayStringBorromScreenTask(currentResTask, &r13_pThis->m16C_displayStringTask, duration, stringPtr);
            }
            break;
        default:
            assert(0);
        }
    } while (1);
}

void scriptUpdateRunScript()
{
    s32 r8_numBackgroundScripts = npcData0.m0_numBackgroundScripts;

    if (!npcData0.m104_currentScript.m0_scriptPtr.isNull())
    {
        if ((graphicEngineStatus.m4514.m0_inputDevices->m0_current.m8_newButtonDown & (graphicEngineStatus.m4514.mD8_buttonConfig[0][1] | graphicEngineStatus.m4514.mD8_buttonConfig[0][2]))
            && (npcData0.mF4 == 0))
        {
            npcData0.m100 = 0;
            npcData0.mF8 = 0;
            npcData0.mF0 = 0;
        }

        //600CF06
        if (npcData0.m100)
        {
            --npcData0.m100;
            return;
        }

        npcData0.m104_currentScript.m0_scriptPtr = runScript(&npcData0);
        if (npcData0.m104_currentScript.m0_scriptPtr.isNull())
        {
            npcData0.mFC &= ~0x1F;
            graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
        }
        else
        {
            npcData0.m0_numBackgroundScripts = 0;
        }
        return;
    }
    else
    {
        // 0x600CF48
        if (r8_numBackgroundScripts)
        {
            npcData0.mF4 = 0;
            npcData0.mF0 = 0;

            std::array<sRunningScriptContext, 4>::iterator r9 = npcData0.m4_backgroundScripts.begin();

            do
            {
                npcData0.m104_currentScript = *r9;

                npcData0.m104_currentScript.m0_scriptPtr = runScript(&npcData0);
                if (!npcData0.m104_currentScript.m0_scriptPtr.isNull())
                {
                    npcData0.mFC |= 0xF;
                    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
                    npcData0.m0_numBackgroundScripts = 0;
                }

                r9++;
            } while (--r8_numBackgroundScripts);
        }
    }
}

void sScriptTask::Init(sScriptTask* pThis)
{
    resData.m0 = 0;
    initResTable();
    copyCameraPropertiesToRes();
}

struct sBundleEntry
{
    u8* m_bundleBase;
    u8* m_entry;
};

sBundleEntry getBundleEntry(u8* pBundleBase, u32 entryOffset)
{
    u32 offsetInBundle = READ_BE_U32(pBundleBase + entryOffset);
    u8* pEntry = pBundleBase + offsetInBundle;

    sBundleEntry newEntry;
    newEntry.m_bundleBase = pBundleBase;
    newEntry.m_entry = pEntry;

    return newEntry;
}

sVec3_FP getEnvLCSTargetPosition(s32 index)
{
    sVec3_FP position;
    if (index >= npcData0.m68_numEnvLCSTargets)
    {
        position.zero();
    }
    else
    {
        position = readSaturnVec3(npcData0.m6C_LCSTargets + index * 12);
    }

    return position;
}

void sScriptTask::Update(sScriptTask* pThis)
{
    if (pThis->m18_LCSFocusLineScale)
    {
        pThis->m18_LCSFocusLineScale--;
    }

    pThis->m8_currentLCSType = 0;
    pThis->m4 = sSaturnPtr::getNull();
    if (!(npcData0.mFC & 0x10))
    {
        pThis->m8_currentLCSType = 0;
        pThis->m4 = sSaturnPtr::getNull();
    }
    else if(!(npcData0.mFC & 1))
    {
        if (pThis->m8_currentLCSType == 2)
        {
            assert(0);
        }

        //60305A8
        setupDataForLCSCollision();
        findLCSCollision();

        switch (LCSCollisionData.m10_activeLCSType)
        {
        case 1:
            pThis->m8_currentLCSType = LCSCollisionData.m10_activeLCSType;
            pThis->mC_AsIndex = LCSCollisionData.m14_activeLCSEnvironmentIndex;
            break;
        case 2:
            pThis->m8_currentLCSType = LCSCollisionData.m10_activeLCSType;
            pThis->mC = LCSCollisionData.m14_activeLCS;
            break;
        default:
            break;
        }

        sScriptTask* var0;

        //06030618
        sSaturnPtr r13 = sSaturnPtr::getNull();
        p_workArea r11 = nullptr;
        switch (pThis->m8_currentLCSType)
        {
        case 0: // nothing
            r13 = sSaturnPtr::getNull();
            break;
        case 1: // environment object
            var0 = pThis;
            pThis->m10_distanceToLCS = vecDistance(getEnvLCSTargetPosition(pThis->mC_AsIndex), *npcData0.m160_pEdgePosition);
            r13 = readSaturnEA(npcData0.m64_scriptList + pThis->mC_AsIndex * 4);
            r11 = nullptr;
            break;
        case 2: // NPC object
            var0 = pThis;
            pThis->m10_distanceToLCS = vecDistance(pThis->mC->m8_position, *npcData0.m160_pEdgePosition);
            r13 = pThis->mC->m3C_scriptEA;
            r11 = pThis->mC->m38_pOwner;
            break;
        default:
            assert(0);
            break;
        }

        //06030670
        if (!r13.isNull())
        {
            addBackgroundScript(r13, 2, r11, nullptr);
            if (pThis->m4 != r13)
            {
                pThis->m18_LCSFocusLineScale = 8;
                pThis->m1C_LCS_X = LCSCollisionData.m0_LCS_X.getInteger();
                pThis->m1E_LCS_Y = LCSCollisionData.m4_LCS_Y.getInteger();
                playSoundEffect(0x24);
            }
        }

        pThis->m4 = r13;
    }

    // 060306B4
    scriptUpdateSub0();
    initResTable();
    scriptUpdateRunScript();
}

void updateTownLCSTargetPosition(sScriptTask* pThis)
{
    if (pThis->m8_currentLCSType == 0)
        return;

    pushCurrentMatrix();
    switch (pThis->m8_currentLCSType)
    {
    case 1:
        translateCurrentMatrix(getEnvLCSTargetPosition(pThis->mC_AsIndex));
        break;
    case 2:
        translateCurrentMatrix(pThis->mC->m8_position);
        if (pThis->mC->m2C == 3)
        {
            adjustMatrixTranslation(pThis->mC->m14_collisionClip[1] / 2);
        }
        break;
    default:
        assert(0);
        break;
    }

    if ((pCurrentMatrix->matrix[11] >= graphicEngineStatus.m405C.m14_farClipDistance) || (pCurrentMatrix->matrix[11] < graphicEngineStatus.m405C.m10_nearClipDistance))
    {
        pThis->m8_currentLCSType = 0;
    }
    else
    {
        sVec2_S16 LCSScreenCoordinates;
        LCSScreenCoordinates[0] = setDividend(graphicEngineStatus.m405C.m18_widthScale, pCurrentMatrix->matrix[3], pCurrentMatrix->matrix[11]);
        LCSScreenCoordinates[1] = setDividend(graphicEngineStatus.m405C.m1C_heightScale, pCurrentMatrix->matrix[7], pCurrentMatrix->matrix[11]);

        if (
            (LCSScreenCoordinates[0] < graphicEngineStatus.m405C.mC - 0x10) || (LCSScreenCoordinates[0] > graphicEngineStatus.m405C.mE + 0x10) ||
            (LCSScreenCoordinates[1] < graphicEngineStatus.m405C.mA - 0x10) || (LCSScreenCoordinates[1] > graphicEngineStatus.m405C.m8 + 0x10))
        {
            pThis->m8_currentLCSType = 0;
        }
        else
        {
            pThis->m14_LCS = LCSScreenCoordinates;
        }
    }

    popMatrix();
}

void drawLineRectangle(u8 r4, s16* position, u32 color)
{
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1005); // command 0
    setVdp1VramU16(vdp1WriteEA + 0x04, 0x400 | r4); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, color); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x0C, position[0]); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, -position[1]); // CMDYA
    setVdp1VramU16(vdp1WriteEA + 0x010, position[2]); // CMDXC
    setVdp1VramU16(vdp1WriteEA + 0x012, -position[3]); // CMDYX
    setVdp1VramU16(vdp1WriteEA + 0x014, position[4]);
    setVdp1VramU16(vdp1WriteEA + 0x016, -position[5]);
    setVdp1VramU16(vdp1WriteEA + 0x018, position[6]);
    setVdp1VramU16(vdp1WriteEA + 0x01A, -position[7]);

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void sScriptTask::Draw(sScriptTask* pThis)
{
    if (pThis->m18_LCSFocusLineScale)
    {
        s16 var8[8];
        var8[0] = var8[6] = pThis->m1C_LCS_X + performDivision(8, (-pThis->m1C_LCS_X - 176) * pThis->m18_LCSFocusLineScale); // X1
        var8[1] = var8[3] = pThis->m1E_LCS_Y + performDivision(8, (-pThis->m1E_LCS_Y - 112) * pThis->m18_LCSFocusLineScale); // Y1
        var8[2] = var8[4] = pThis->m1C_LCS_X + performDivision(8, (-pThis->m1C_LCS_X + 176) * pThis->m18_LCSFocusLineScale); // X2
        var8[5] = var8[7] = pThis->m1E_LCS_Y + performDivision(8, (-pThis->m1E_LCS_Y + 112) * pThis->m18_LCSFocusLineScale); // Y2

        drawLineRectangle(0xC0, var8, (((pThis->m18_LCSFocusLineScale * 3) + 7) << 10) | ((pThis->m18_LCSFocusLineScale + 2) << 5) | (pThis->m18_LCSFocusLineScale + 2) | 0x8000);
    }
    
    updateTownLCSTargetPosition(pThis);
}

void startScriptTask(p_workArea r4)
{
    currentResTask = createSubTask<sScriptTask>(r4);
}
