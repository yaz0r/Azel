#include "PDS.h"
#include "collisionRegistry.h"
#include "townScript.h"
#include "town.h"
#include "processModel.h"
#include "kernel/debug/trace.h"
#include "kernel/fileBundle.h"

sCollisionBodyRegistry gCollisionRegistry;

// Preset collision setups indexed by collision type.
// Fields: m0_collisionType, m1, m2_collisionLayersBitField
static const std::array<sCollisionSetup, 5> collisionSetupArray = {
    {
        {1, 0, 0x10},
        {1, 1, 0x1C},
        {0, 0, 0x18},
        {2, 1, 0},
        {3, 1, 0}
    }
};

// Copies the preset collision setup (type and layer mask) at the given index
// into the body and stores the index for later lookup.
void setCollisionSetup(sCollisionBody* pBody, s32 presetIndex)
{
    pBody->m2C_collisionSetupIndex = presetIndex;
    pBody->m0_collisionSetup = collisionSetupArray[presetIndex];
}

// Derives the AABB center and half-extents from the two corner points,
// then computes the bounding sphere radius as the half-diagonal of the box.
void setCollisionBounds(sCollisionBody* r4, const sVec3_FP& r5, const sVec3_FP& r6)
{
    r4->m20_AABBCenter = (r5 + r6) / 2;

    r4->m14_halfAABB[0] = r4->m20_AABBCenter[0] - std::min(r5[0], r6[0]);
    r4->m14_halfAABB[1] = r4->m20_AABBCenter[1] - std::min(r5[1], r6[1]);
    r4->m14_halfAABB[2] = r4->m20_AABBCenter[2] - std::min(r5[2], r6[2]);

    r4->m4_sphereRadius = sqrt_F(MTH_Product3d_FP(r4->m14_halfAABB, r4->m14_halfAABB));
}

// An oscillating offset applied to AABB world-space positioning; toggled each frame.
s32 resValue0 = 0;
s32 resValue1 = 0;

struct sScriptUpdateSub0Sub0Var0
{
    sVec3_FP m0_position;
    s32 mC_distance;
    fixedPoint m10_y;
    //size 0x14
};

std::array<sScriptUpdateSub0Sub0Var0, 12> scriptUpdateSub0Sub0Var0;

struct sScriptUpdateSub0Sub0Var1
{
    s32 m0;
    s32 m4;
    s32 m8;
    s32 mC;
    //size 0x10?
}scriptUpdateSub0Sub0Var1;


// Inserts the collision body into the per-type linked list for this frame and
// updates its world-space AABB center position from the owner's position/rotation.
void registerCollisionBody(sCollisionBody* r14_pose)
{
    if (gCollisionRegistry.m4 >= 0x3F)
        return;

    sCollisionBodyNode& r5 = gCollisionRegistry.m1C[gCollisionRegistry.m4++];
    r5.m0_pNext = gCollisionRegistry.m8_headOfLinkedList[r14_pose->m2C_collisionSetupIndex];
    r5.m4 = r14_pose;

    gCollisionRegistry.m8_headOfLinkedList[r14_pose->m2C_collisionSetupIndex] = &r5;

    sMatrix4x3 var4;
    initMatrixToIdentity(&var4);
    rotateMatrixYXZ(r14_pose->m34_pRotation, &var4);
    transformVec(r14_pose->m20_AABBCenter, r14_pose->m8_position, var4);

    r14_pose->m8_position += *r14_pose->m30_pPosition;
}

// Pushes a local-space matrix for r4 centred on its AABB half-extents, rotated by owner
// rotation, and translated so r4's world position maps to the origin.
void scriptUpdateSub0Sub0(sCollisionBody* r4)
{
    const sVec3_FP& r12 = *r4->m34_pRotation;

    switch (r4->m0_collisionSetup.m0_collisionType)
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
    pCurrentMatrix->m[0][3] = r4->m14_halfAABB[0];
    pCurrentMatrix->m[1][3] = r4->m14_halfAABB[1];
    pCurrentMatrix->m[2][3] = r4->m14_halfAABB[2];

    rotateCurrentMatrixShiftedZ(-r12[2]);
    rotateCurrentMatrixShiftedX(-r12[0]);
    rotateCurrentMatrixShiftedY(-r12[1]);

    sVec3_FP var0;
    var0[0] = -r4->m8_position[0] - resValue0;
    var0[1] = -r4->m8_position[1];
    var0[2] = -r4->m8_position[2] - resValue0;

    translateCurrentMatrix(var0);
}

// Tests if spheres of r13 and r14 overlap; returns 1 if they do.
s32 scriptUpdateSub0Sub1(sCollisionBody* r13, sCollisionBody* r14)
{
    if (r14->m40)
    {
        assert(0);
    }
    else
    {
        if (distanceSquareBetween2Points(r13->m8_position, r14->m8_position) >= FP_Pow2(r13->m4_sphereRadius + r13->m4_sphereRadius))
            return 0;

        //0600874E
        Unimplemented();
    }

    return 0;
}

// Records the deepest Z- contact face (face normal pointing away from origin in Z).
void scriptUpdateSub0Sub2Sub0(sCollisionBody* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[2]) - r13->m14_halfAABB[2];

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

// Records the deepest Z+ contact face.
void scriptUpdateSub0Sub2Sub1(sCollisionBody* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[2]) + r13->m14_halfAABB[2];

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

// Records the deepest X- contact face.
void scriptUpdateSub0Sub2Sub2(sCollisionBody* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[0]) - r13->m14_halfAABB[0];

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

// Records the deepest X+ contact face.
void scriptUpdateSub0Sub2Sub3(sCollisionBody* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[0]) + r13->m14_halfAABB[0];

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

// Records the deepest Y- contact face.
void scriptUpdateSub0Sub2Sub4(sCollisionBody* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[1]) - r13->m14_halfAABB[1];

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

// Records the deepest Y+ contact face.
void scriptUpdateSub0Sub2Sub5(sCollisionBody* r13, fixedPoint r12, sVec3_FP* r6, sVec3_FP* r14)
{
    fixedPoint r5 = FP_Div(r12, (*r14)[1]) + r13->m14_halfAABB[1];

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

// Projects a face plane normal onto r4's AABB half-extents then dispatches to the
// appropriate axis-contact recorder (Sub0–Sub5) based on penetration direction.
void scriptUpdateSub0Sub2(sCollisionBody* r4, fixedPoint r5)
{
    sVec3_FP var4;
    var4[0] = pCurrentMatrix->m[0][1];
    var4[1] = pCurrentMatrix->m[1][1];
    var4[2] = pCurrentMatrix->m[2][1];

    sVec3_FP var10;
    var10[0] = MTH_Mul(var4[0], r4->m14_halfAABB[0]);
    var10[1] = MTH_Mul(var4[1], r4->m14_halfAABB[1]);
    var10[2] = MTH_Mul(var4[2], r4->m14_halfAABB[2]);

    fixedPoint r14 = r5;
    r14 += MTH_Mul(var4[0], pCurrentMatrix->m[0][3]);
    r14 += MTH_Mul(var4[1], pCurrentMatrix->m[1][3]);
    r14 += MTH_Mul(var4[2], pCurrentMatrix->m[2][3]);
    r14 -= var10[0];
    r14 -= var10[1];
    r14 -= var10[2];

    switch (r4->m0_collisionSetup.m0_collisionType)
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

// Returns a sub-cell index (0–3) encoding which quadrant of the cell the position is in.
s32 scriptUpdateSub0Sub3Sub1(fixedPoint r4_x, fixedPoint r5_z)
{
    s32 r8 = MTH_Mul32(r4_x, gTownGrid.m30_worldToCellIndex * 2) & 1;
    s32 r0 = MTH_Mul32(r5_z, gTownGrid.m30_worldToCellIndex * 4) & 2;

    return r8 + r0;
}

// Returns the sTownCellTask covering world position (r4_x, r5_z), or nullptr if out of range.
sTownCellTask* scriptUpdateSub0Sub3Sub0(fixedPoint r4_x, fixedPoint r5_z)
{
    if (isTraceEnabled())
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

    if (isTraceEnabled())
    {
        addTraceLog("scriptUpdateSub0Sub3Sub0 found cell. GridSize: %d %d\n", gTownGrid.m0_sizeX, gTownGrid.m4_sizeY);
    }

    return gTownGrid.m40_cellTasks[(gTownGrid.mC + r5_cellY) & 7][(gTownGrid.m8 + r13_cellX) & 7];
}

static void convertVerticeTo16(fixedPoint*& r1, std::array<sVec3_S16, 3>::iterator& r2)
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

// Transforms all mesh vertices by the current matrix, outputs fixed-point positions and
// clip flags; returns the bitwise-AND of all clip flags (non-zero ⟹ all vertices off-screen).
s32 transformTownMeshVertices(const std::vector<sVec3_S16_12_4>& r4_vertices, std::array<sProcessedVertice, 255>& r5_outputConverted, s32 r6_numVertices, const sVec3_FP& r7_clip)
{
    fixedPoint* r1 = &pCurrentMatrix->m[0][0];
    std::array<sVec3_S16, 3> rotationMatrix;
    std::array<sVec3_S16, 3>::iterator r2 = rotationMatrix.begin();

    convertVerticeTo16(r1, r2);
    s32 r12_X = *r1++;
    convertVerticeTo16(r1, r2);
    s32 r13_Y = *r1++;
    convertVerticeTo16(r1, r2);
    s32 r14_Z = *r1++;

    r12_X *= 0x100;
    r13_Y *= 0x100;
    r14_Z *= 0x100;

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

static void transformNormalByCurrentMatrix(const sVec3_S16_12_4& normal, sVec3_FP& output)
{
    transformVecByCurrentMatrix(normal.toSVec3_FP(), output);
}

// 2D cross product of (r4,r5) and (r6,r7); used for point-in-quad winding tests.
static s32 testTownMeshQuadForCollisionSub1(const fixedPoint& r4, const fixedPoint& r5, const fixedPoint& r6, const fixedPoint& r7)
{
    s64 mac = (s64)r4.asS32() * (s64)r5.asS32();
    mac -= (s64)r6.asS32() * (s64)r7.asS32();

    return mac >> 32;
}

// Tests one quad of a town mesh against r14's AABB. When the AABB centre lies inside the
// quad and within AABB half-extents of the face plane, records the penetrating contact face
// and optionally fires the quad's on-collision script.
void testTownMeshQuadForCollision(sCollisionBody* r14, const sProcessed3dModel::sQuad& r5_quad, const std::array<sProcessedVertice, 255>& r6_vertices)
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
    var18[0] = MTH_Mul(varC[0], r14->m14_halfAABB[0]);
    var18[1] = MTH_Mul(varC[1], r14->m14_halfAABB[1]);
    var18[2] = MTH_Mul(varC[2], r14->m14_halfAABB[2]);

    fixedPoint var0 = MTH_Product3d_FP(varC, r13_vertice0.vertice) - var18[0] - var18[1] - var18[2];

    if (FP_Pow2(var18[2]) > FP_Pow2(var0))
    {
        //6008D50
        if (var0 > var18[2])
        {
            //06008D58
            if ((testTownMeshQuadForCollisionSub1(
                    r13_vertice0.vertice[0] - r14->m14_halfAABB[0],
                    r10_vertice1.vertice[1] - r14->m14_halfAABB[1],
                    r10_vertice1.vertice[0] - r14->m14_halfAABB[0],
                    r13_vertice0.vertice[1] - r14->m14_halfAABB[1]) >= 0) &&
                //6008D7E
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[0] - r14->m14_halfAABB[0],
                    r9_vertice2.vertice[1] - r14->m14_halfAABB[1],
                    r9_vertice2.vertice[0] - r14->m14_halfAABB[0],
                    r10_vertice1.vertice[1] - r14->m14_halfAABB[1]) >= 0) &&
                //6008DA0
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[0] - r14->m14_halfAABB[0],
                    r11_vertice3.vertice[1] - r14->m14_halfAABB[1],
                    r11_vertice3.vertice[0] - r14->m14_halfAABB[0],
                    r9_vertice2.vertice[1] - r14->m14_halfAABB[1]) >= 0) &&
                //6008DC2
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[0] - r14->m14_halfAABB[0],
                    r13_vertice0.vertice[1] - r14->m14_halfAABB[1],
                    r13_vertice0.vertice[0] - r14->m14_halfAABB[0],
                    r11_vertice3.vertice[1] - r14->m14_halfAABB[1]) >= 0))
            {
                //6008DE4
                switch (r14->m0_collisionSetup.m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript((*npcData0.m64_scriptList)[r5_quad.m12_onCollisionScriptIndex], 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
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
                    r13_vertice0.vertice[0] - r14->m14_halfAABB[0],
                    r10_vertice1.vertice[1] - r14->m14_halfAABB[1],
                    r10_vertice1.vertice[0] - r14->m14_halfAABB[0],
                    r13_vertice0.vertice[1] - r14->m14_halfAABB[1]) <= 0) &&
                //06008E62
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[0] - r14->m14_halfAABB[0],
                    r9_vertice2.vertice[1] - r14->m14_halfAABB[1],
                    r9_vertice2.vertice[0] - r14->m14_halfAABB[0],
                    r10_vertice1.vertice[1] - r14->m14_halfAABB[1]) <= 0) &&
                //06008E80
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[0] - r14->m14_halfAABB[0],
                    r11_vertice3.vertice[1] - r14->m14_halfAABB[1],
                    r11_vertice3.vertice[0] - r14->m14_halfAABB[0],
                    r9_vertice2.vertice[1] - r14->m14_halfAABB[1]) <= 0) &&
                //06008E9E
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[0] - r14->m14_halfAABB[0],
                    r13_vertice0.vertice[1] - r14->m14_halfAABB[1],
                    r13_vertice0.vertice[0] - r14->m14_halfAABB[0],
                    r11_vertice3.vertice[1] - r14->m14_halfAABB[1]) <= 0))
            {
                //06008EBC
                switch (r14->m0_collisionSetup.m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript((*npcData0.m64_scriptList)[r5_quad.m12_onCollisionScriptIndex], 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
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
                    r13_vertice0.vertice[1] - r14->m14_halfAABB[1],
                    r10_vertice1.vertice[2] - r14->m14_halfAABB[2],
                    r10_vertice1.vertice[1] - r14->m14_halfAABB[1],
                    r13_vertice0.vertice[2] - r14->m14_halfAABB[2]) >= 0) &&
                //6008F7C
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[1] - r14->m14_halfAABB[1],
                    r9_vertice2.vertice[2] - r14->m14_halfAABB[2],
                    r9_vertice2.vertice[1] - r14->m14_halfAABB[1],
                    r10_vertice1.vertice[2] - r14->m14_halfAABB[2]) >= 0) &&
                //6008F9E
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[1] - r14->m14_halfAABB[1],
                    r11_vertice3.vertice[2] - r14->m14_halfAABB[2],
                    r11_vertice3.vertice[1] - r14->m14_halfAABB[1],
                    r9_vertice2.vertice[2] - r14->m14_halfAABB[2]) >= 0) &&
                //6008FC0
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[1] - r14->m14_halfAABB[1],
                    r13_vertice0.vertice[2] - r14->m14_halfAABB[2],
                    r13_vertice0.vertice[1] - r14->m14_halfAABB[1],
                    r11_vertice3.vertice[2] - r14->m14_halfAABB[2]) >= 0))
            {
                //6008FE2
                switch (r14->m0_collisionSetup.m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript((*npcData0.m64_scriptList)[r5_quad.m12_onCollisionScriptIndex], 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
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
                    r13_vertice0.vertice[1] - r14->m14_halfAABB[1],
                    r10_vertice1.vertice[2] - r14->m14_halfAABB[2],
                    r10_vertice1.vertice[1] - r14->m14_halfAABB[1],
                    r13_vertice0.vertice[2] - r14->m14_halfAABB[2]) <= 0) &&
                //6009060
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[1] - r14->m14_halfAABB[1],
                    r9_vertice2.vertice[2] - r14->m14_halfAABB[2],
                    r9_vertice2.vertice[1] - r14->m14_halfAABB[1],
                    r10_vertice1.vertice[2] - r14->m14_halfAABB[2]) <= 0) &&
                //0600907E
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[1] - r14->m14_halfAABB[1],
                    r11_vertice3.vertice[2] - r14->m14_halfAABB[2],
                    r11_vertice3.vertice[1] - r14->m14_halfAABB[1],
                    r9_vertice2.vertice[2] - r14->m14_halfAABB[2]) <= 0) &&
                //0600909C
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[1] - r14->m14_halfAABB[1],
                    r13_vertice0.vertice[2] - r14->m14_halfAABB[2],
                    r13_vertice0.vertice[1] - r14->m14_halfAABB[1],
                    r11_vertice3.vertice[2] - r14->m14_halfAABB[2]) <= 0))
            {
                //060090BA
                switch (r14->m0_collisionSetup.m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript((*npcData0.m64_scriptList)[r5_quad.m12_onCollisionScriptIndex], 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
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
                    r13_vertice0.vertice[2] - r14->m14_halfAABB[2],
                    r10_vertice1.vertice[0] - r14->m14_halfAABB[0],
                    r10_vertice1.vertice[2] - r14->m14_halfAABB[2],
                    r13_vertice0.vertice[0] - r14->m14_halfAABB[0]) >= 0) &&
                //600916C
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[2] - r14->m14_halfAABB[2],
                    r9_vertice2.vertice[0] - r14->m14_halfAABB[0],
                    r9_vertice2.vertice[2] - r14->m14_halfAABB[2],
                    r10_vertice1.vertice[0] - r14->m14_halfAABB[0]) >= 0) &&
                //600918E
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[2] - r14->m14_halfAABB[2],
                    r11_vertice3.vertice[0] - r14->m14_halfAABB[0],
                    r11_vertice3.vertice[2] - r14->m14_halfAABB[2],
                    r9_vertice2.vertice[0] - r14->m14_halfAABB[0]) >= 0) &&
                //60091B0
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[2] - r14->m14_halfAABB[2],
                    r13_vertice0.vertice[0] - r14->m14_halfAABB[0],
                    r13_vertice0.vertice[2] - r14->m14_halfAABB[2],
                    r11_vertice3.vertice[0] - r14->m14_halfAABB[0]) >= 0))
            {
                //60091D2
                switch (r14->m0_collisionSetup.m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript((*npcData0.m64_scriptList)[r5_quad.m12_onCollisionScriptIndex], 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
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
                    r13_vertice0.vertice[2] - r14->m14_halfAABB[2],
                    r10_vertice1.vertice[0] - r14->m14_halfAABB[0],
                    r10_vertice1.vertice[2] - r14->m14_halfAABB[2],
                    r13_vertice0.vertice[0] - r14->m14_halfAABB[0]) <= 0) &&
                //06009250
                (testTownMeshQuadForCollisionSub1(
                    r10_vertice1.vertice[2] - r14->m14_halfAABB[2],
                    r9_vertice2.vertice[0] - r14->m14_halfAABB[0],
                    r9_vertice2.vertice[2] - r14->m14_halfAABB[2],
                    r10_vertice1.vertice[0] - r14->m14_halfAABB[0]) <= 0) &&
                //0600926E
                (testTownMeshQuadForCollisionSub1(
                    r9_vertice2.vertice[2] - r14->m14_halfAABB[2],
                    r11_vertice3.vertice[0] - r14->m14_halfAABB[0],
                    r11_vertice3.vertice[2] - r14->m14_halfAABB[2],
                    r9_vertice2.vertice[0] - r14->m14_halfAABB[0]) <= 0) &&
                //0600928C
                (testTownMeshQuadForCollisionSub1(
                    r11_vertice3.vertice[2] - r14->m14_halfAABB[2],
                    r13_vertice0.vertice[0] - r14->m14_halfAABB[0],
                    r13_vertice0.vertice[2] - r14->m14_halfAABB[2],
                    r11_vertice3.vertice[0] - r14->m14_halfAABB[0]) <= 0))
            {
                //060092AA
                switch (r14->m0_collisionSetup.m0_collisionType)
                {
                case 0:
                    resValue1 = 1;
                    if (r5_quad.m12_onCollisionScriptIndex)
                    {
                        //060092C4
                        addBackgroundScript((*npcData0.m64_scriptList)[r5_quad.m12_onCollisionScriptIndex], 1, nullptr, &r5_quad.m14_extraData[0].m0_normals);
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

// Transforms all mesh vertices into r4's AABB-local space and runs per-quad collision tests.
void processTownMeshCollision(sCollisionBody* r4, const sProcessed3dModel* r5)
{
    if (isTraceEnabled())
    {
        addTraceLog("processTownMeshCollision: current matrix: ");
        for (int row = 0; row < 3; row++)
            for (int col = 0; col < 4; col++)
                addTraceLog("0x%08X ", pCurrentMatrix->m[row][col]);
        addTraceLog("\n");
    }
    std::array<sProcessedVertice, 255> transformedVertices;
    u32 clipFlag = transformTownMeshVertices(r5->m8_vertices, transformedVertices, r5->m4_numVertices, r4->m14_halfAABB);

    if (isTraceEnabled())
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
    if (r4->m2C_collisionSetupIndex == 0)
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

// Tests r12's AABB against all collision meshes inside the given town cell.
void scriptUpdateSub0Sub3Sub2(sCollisionBody* r12, sTownCellTask* r13)
{
    if (r13 == nullptr)
        return;

    if (r13->m8_cellPtr.m_offset == 0)
        return;

    if (readSaturnU32(r13->m8_cellPtr + 0x14) == 0)
        return;

    pushCurrentMatrix();
    {
        translateCurrentMatrix(r13->mC_position);

        sVec3_FP var0_positionInCell;
        var0_positionInCell = r12->m8_position - r13->mC_position;

        sSaturnPtr r14 = readSaturnEA(r13->m8_cellPtr + 0x14);
        while (readSaturnU32(r14))
        {
            sProcessed3dModel* mesh = r13->m0_fileBundle->getCollisionModel(readSaturnU32(r14));
            sVec3_FP meshPositionInCell = readSaturnVec3(r14 + 4);
            if (distanceSquareBetween2Points(var0_positionInCell, meshPositionInCell) < FP_Pow2(mesh->m0_radius + r12->m4_sphereRadius))
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

// Tests r4's AABB against the town environment cells that overlap its position.
void handleCollisionWithTownEnv(sCollisionBody* r4)
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

// Resolves the final world-space push translation for r12 from the recorded contact faces,
// clamps it to half the AABB extents, then rotates it into world space.
void scriptUpdateSub0Sub4Sub0(sCollisionBody* r12)
{
    std::array<sScriptUpdateSub0Sub0Var0, 12>& r13 = scriptUpdateSub0Sub0Var0;

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
    var14.zeroize();

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
    if (var14[0] > r12->m14_halfAABB[0] / 2)
    {
        var14[0] = r12->m14_halfAABB[0] / 2;
    }

    if (var14[0] < -r12->m14_halfAABB[0] / 2)
    {
        var14[0] = -r12->m14_halfAABB[0] / 2;
    }

    if (var14[1] > r12->m14_halfAABB[1] / 2)
    {
        var14[1] = r12->m14_halfAABB[1] / 2;
    }

    if (var14[1] < -r12->m14_halfAABB[1] / 2)
    {
        var14[1] = -r12->m14_halfAABB[1] / 2;
    }

    if (var14[2] > r12->m14_halfAABB[2] / 2)
    {
        var14[2] = r12->m14_halfAABB[2] / 2;
    }

    if (var14[2] < -r12->m14_halfAABB[2] / 2)
    {
        var14[2] = -r12->m14_halfAABB[2] / 2;
    }

    //6008668
    sMatrix4x3 var20;
    initMatrixToIdentity(&var20);
    rotateMatrixYXZ(r12->m34_pRotation, &var20);
    transformVec(var14, r12->m58_collisionSolveTranslation, var20);
}

// Pops the matrix pushed by scriptUpdateSub0Sub0, then for type 0/1 bodies computes
// the final world-space separation vector.
void scriptUpdateSub0Sub4(sCollisionBody* r4)
{
    popMatrix();

    switch (r4->m0_collisionSetup.m0_collisionType)
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

// Processes all active collision bodies for this frame: runs sphere broad-phase,
// per-face AABB environment tests, inter-body penetration resolution, and collision scripts.
void scriptUpdateSub0()
{
    for (int r8 = 4-1; r8 >= 0; r8--)
    {
        sCollisionBodyNode* r11 = gCollisionRegistry.m8_headOfLinkedList[r8];
        while (r11)
        {
            sCollisionBody* r14 = r11->m4;
            r11 = r11->m0_pNext;

            if (r14->m0_collisionSetup.m2_collisionLayersBitField)
            {
                scriptUpdateSub0Sub0(r14);
                s32 r9 = r8 + 1;
                do
                {
                    if (r14->m0_collisionSetup.m2_collisionLayersBitField & (1 << r9))
                    {
                        sCollisionBodyNode* r12 = gCollisionRegistry.m8_headOfLinkedList[r9];
                        while (r12)
                        {
                            sCollisionBody* r13 = r12->m4;
                            r12 = r12->m0_pNext;
                            if (scriptUpdateSub0Sub1(r14, r13))
                            {
                                if ((r14->m0_collisionSetup.m0_collisionType == 0) && r13->m3C_scriptEA.m_offset)
                                {
                                    //06007B16
                                    addBackgroundScript(r13->m3C_scriptEA, 1, r13->m38_pOwner, 0);
                                }

                                r14->m48 = r13;
                                r13->m48 = r14;
                                if (r14->m0_collisionSetup.m0_collisionType >= 2)
                                {
                                    goto endOfLoop;
                                }
                            }
                        }
                    }
                } while (++r9 < 5);
                //06007B3C

                if (r14->m0_collisionSetup.m2_collisionLayersBitField & 0x10)
                {
                    scriptUpdateSub0Sub2(r14, gCollisionRegistry.m0);
                }

                handleCollisionWithTownEnv(r14);

            endOfLoop:
                //06007B4E
                scriptUpdateSub0Sub4(r14);
            }
        }
    }
}

// Resets the per-frame collision registry (clears linked list heads, resets node counter)
// and toggles the resValue0 oscillator used by AABB positioning.
void initResTable()
{
    if (isTraceEnabled())
    {
        addTraceLog("InitResTable: resValue0=%d\n", resValue0);
    }
    gCollisionRegistry.m8_headOfLinkedList.fill(0);
    gCollisionRegistry.m4 = 0;

    if (resValue0 < 0)
    {
        resValue0 = 2;
    }
    else
    {
        resValue0 = -2;
    }
}
