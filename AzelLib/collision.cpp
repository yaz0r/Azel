#include "PDS.h"

static bool bDrawCollisions = false;

void transformCollisionVertices(s32 r4, std::vector<sVec3_S16_12_4>& r5, std::array<sVec3_FP, 256>& r6)
{
    for (int i = 0; i < r4; i++)
    {
        sVec3_FP convertedVector = r5[i].toSVec3_FP();
        transformAndAddVecByCurrentMatrix(&convertedVector, &r6[i]);
    }
}

struct sCollisionTempStruct
{
    sVec3_FP m0_translationToResolve;
    fixedPoint mC_collisionRadius;
    fixedPoint m10;
};

s32 testQuadsForCollisionsSub4(fixedPoint r4, fixedPoint r5)
{
    if (r4 < 0)
        return 0;

    if (r5 < 0)
        return 0;

    if (r4 + r5 <= 0x10000)
        return 1;

    return 0;
}

s32 testQuadsForCollisionsSub1(fixedPoint r4, fixedPoint r5)
{
    if (r4 < r5)
        return 0;

    if ((r5 * 2) + r4 <= 0x10000)
        return 1;

    return 0;
}

s32 testQuadsForCollisionsSub2(fixedPoint r4, fixedPoint r5)
{
    if ((r5 * 2) + r4 < 0x10000)
        return 0;

    if ((r4 * 2) + r5 < 0x10000)
        return 0;

    return 1;
}

s32 testQuadsForCollisionsSub3(fixedPoint r4, fixedPoint r5)
{
    if (r4 > r5)
        return 0;

    if ((r4 * 2) + r5 <= 0x10000)
        return 1;

    return 0;
}

void testQuadsForCollisionsSub0(const sVec3_FP& vertice0, const sVec3_FP& vertice1, const sVec3_FP& vertice2, const sVec3_FP& transformedNormal, s32& r11_result0, s32& r10_result1, const sVec3_FP& r14_arg2)
{
    sVec3_FP var10 = r14_arg2 - vertice0;
    sVec3_FP var28 = vertice1 - vertice0;
    sVec3_FP var1C = vertice2 - vertice0;
    sVec3_FP var4;

    var4[0] = transformedNormal[0].getAbs();
    var4[1] = transformedNormal[1].getAbs();
    var4[2] = transformedNormal[2].getAbs();

    s32 r4_largestComponentIndex = -1;
    if (var4[1] > var4[0])
    {
        if (var4[2] > var4[1])
        {
            r4_largestComponentIndex = 2;
        }
        else
        {
            r4_largestComponentIndex = 1;
        }
    }
    else
    {
        if (var4[2] > var4[0])
        {
            r4_largestComponentIndex = 2;
        }
        else
        {
            r4_largestComponentIndex = 0;
        }
    }

    fixedPoint r15;
    fixedPoint r9;
    switch (r4_largestComponentIndex)
    {
    case 0:
        r15 = MTH_Mul(var1C[2], var10[1]) - MTH_Mul(var1C[1], var10[2]);
        r9 = MTH_Mul(var28[1], var1C[2]) - MTH_Mul(var28[2], var1C[1]);
        asyncDivStart(r15, r9);
        r10_result1 = asyncDivEnd();
        r11_result0 = FP_Div(MTH_Mul(-var28[2], var10[1]) + MTH_Mul(var28[1], var10[2]), r9);
        break;
    case 1:
        r15 = MTH_Mul(var1C[0], var10[2]) - MTH_Mul(var1C[2], var10[0]);
        r9 = MTH_Mul(var28[2], var1C[0]) - MTH_Mul(var28[0], var1C[2]);
        asyncDivStart(r15, r9);
        r10_result1 = asyncDivEnd();
        r11_result0 = FP_Div(MTH_Mul(-var28[0], var10[2]) + MTH_Mul(var28[2], var10[0]), r9);
        break;
    case 2:
        r15 = MTH_Mul(var1C[1], var10[0]) - MTH_Mul(var1C[0], var10[1]);
        r9 = MTH_Mul(var28[0], var1C[1]) - MTH_Mul(var28[1], var1C[0]);
        asyncDivStart(r15, r9);
        r10_result1 = asyncDivEnd();
        r11_result0 = FP_Div(MTH_Mul(-var28[1], var10[0]) + MTH_Mul(var28[0], var10[1]), r9);
        break;
    default:
        assert(0);
    }
}

// TODO: kernel
fixedPoint distanceSquareBetween2Points(const sVec3_FP& r4_vertice0, const sVec3_FP& r5_vertice1)
{
    s32 r1 = r5_vertice1[0] - r4_vertice0[0];
    s32 r0 = (((s64)r1 * (s64)r1) >> 16);

    s32 r3 = r5_vertice1[1] - r4_vertice0[1];
    s32 r2 = (((s64)r3 * (s64)r3) >> 16);

    s32 r5 = r5_vertice1[2] - r4_vertice0[2];
    s32 r4 = (((s64)r5 * (s64)r5) >> 16);

    return r0 + r2 + r4;
}

s32 testQuadsForCollisionsSub5(const sVec3_FP& r14_vertice0, const sVec3_FP& r11_vertice1, const sCollisionTempStruct& r13_r6)
{
    getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m1294.mC++;

    sVec3_FP var10 = r13_r6.m0_translationToResolve - r14_vertice0;
    sVec3_FP var4 = r11_vertice1 - r14_vertice0;

    fixedPoint r5;
    fixedPoint r14 = dot3_FP(&var10, &var4);
    if ( r14 < 0)
    {
        r5 = dot3_FP(&var10, &var10);
    }
    else
    {
        r5 = dot3_FP(&var4, &var4);
        if (r14 > r5)
        {
            r5 = distanceSquareBetween2Points(r13_r6.m0_translationToResolve, r11_vertice1);
        }
        else
        {
            asyncDivStart(r14, r5);
            r5 = dot3_FP(&var10, &var10) - MTH_Mul(asyncDivEnd(), r14);
        }
    }

    if (r5 < 0)
        return 0;

    if (r5 > r13_r6.m10)
        return 0;

    return 1;
}

s32 testQuadForCollisions(const sProcessed3dModel::sQuad& r9_currentQuad, std::array<sVec3_FP, 256>& r12, sCollisionTempStruct& r14, sVec3_FP& r11_pointToProject, fixedPoint arg0, std::array<s_visibilityGridWorkArea_5A8, 8>::iterator arg1)
{
    s_visibilityGridWorkArea* r8 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    sVec3_FP var14_quadNormal;
    var14_quadNormal[0] = r9_currentQuad.m14_extraData[0].m0_normals[0];
    var14_quadNormal[1] = r9_currentQuad.m14_extraData[0].m0_normals[1];
    var14_quadNormal[2] = r9_currentQuad.m14_extraData[0].m0_normals[2];

    sVec3_FP var5C_transformedQuadNormal;
    transformVecByCurrentMatrix(var14_quadNormal, var5C_transformedQuadNormal);

    var5C_transformedQuadNormal[0] = MTH_Mul(arg0, var5C_transformedQuadNormal[0]);
    var5C_transformedQuadNormal[1] = MTH_Mul(arg0, var5C_transformedQuadNormal[1]);
    var5C_transformedQuadNormal[2] = MTH_Mul(arg0, var5C_transformedQuadNormal[2]);

    // project r11_pointToProject on the plane
    sVec3_FP var50_transformedVertice = r12[r9_currentQuad.m0_indices[0]] - r11_pointToProject;
    fixedPoint r4_dotResult = dot3_FP(&var50_transformedVertice, &var5C_transformedQuadNormal) * 16;

    if (r4_dotResult.m_value > 0)
        return 0;

    r8->m1294.m0_processedQuadsForCollision++;

    // why are we computing this again? this seems wasteful :-(
    var50_transformedVertice = r12[r9_currentQuad.m0_indices[0]] - r11_pointToProject;
    fixedPoint var10_penetration = dot3_FP(&var50_transformedVertice, &var5C_transformedQuadNormal) * 16;

    // 06063BD4
    // are we colliding with the collision sphere?
    if (var10_penetration.getAbs() > r14.mC_collisionRadius)
        return 0;

    r8->m1294.m4_processedQuadsForCollision2++;
    r8->m1294.m8_processedQuadsForCollision3++;

    sVec3_FP var44_previousResolvedPosition;
    var44_previousResolvedPosition[0] = r14.m0_translationToResolve[0] + MTH_Mul(var10_penetration, var5C_transformedQuadNormal[0]) * 16;
    var44_previousResolvedPosition[1] = r14.m0_translationToResolve[1] + MTH_Mul(var10_penetration, var5C_transformedQuadNormal[1]) * 16;
    var44_previousResolvedPosition[2] = r14.m0_translationToResolve[2] + MTH_Mul(var10_penetration, var5C_transformedQuadNormal[2]) * 16;

    sVec3_FP var38_previousResolvedPositionBackup = var44_previousResolvedPosition;

    if (var10_penetration > 0)
    {
        r14.m0_translationToResolve = var44_previousResolvedPosition;
        r11_pointToProject = var44_previousResolvedPosition;
        var10_penetration = 0;
    }

    //6063C7A
    std::array<std::array<s32, 3>,2> var20;
    var20[0][0] = r9_currentQuad.m0_indices[0];
    var20[0][1] = r9_currentQuad.m0_indices[1];
    var20[0][2] = r9_currentQuad.m0_indices[2];

    s32 var8_numTrianglesToProcess;
    if (r9_currentQuad.m0_indices[2] == r9_currentQuad.m0_indices[3])
    {
        // it's a triangle!
        var8_numTrianglesToProcess = 1;
    }
    else
    {
        // it's a quad!
        var8_numTrianglesToProcess = 2;

        var20[1][0] = r9_currentQuad.m0_indices[0];
        var20[1][1] = r9_currentQuad.m0_indices[2];
        var20[1][2] = r9_currentQuad.m0_indices[3];
    }

    // start of the triangle loop
    // 06063CBA
    do 
    {
        sVec3_FP& vertice0 = r12[var20[var8_numTrianglesToProcess - 1][0]];
        sVec3_FP& vertice1 = r12[var20[var8_numTrianglesToProcess - 1][1]];
        sVec3_FP& vertice2 = r12[var20[var8_numTrianglesToProcess - 1][2]];

        std::array<s32, 2> result;
        testQuadsForCollisionsSub0(vertice0, vertice1, vertice2, var5C_transformedQuadNormal, result[0], result[1], var44_previousResolvedPosition);

        s32 r9 = 0;
        if (testQuadsForCollisionsSub4(result[1], result[0]))
        {
            r9 = 1;
        }
        else
        {
            if (testQuadsForCollisionsSub1(result[1], result[0]))
            {
                //06063D24
                r9 = testQuadsForCollisionsSub5(vertice0, vertice1, r14);
            }
            else if (testQuadsForCollisionsSub2(result[1], result[0]))
            {
                //06063D5E
                r9 = testQuadsForCollisionsSub5(vertice1, vertice2, r14);
            }
            else if (testQuadsForCollisionsSub3(result[1], result[0]))
            {
                //06063DA0
                r9 = testQuadsForCollisionsSub5(vertice2, vertice0, r14);
            }
        }

        //6063DD4
        if (r9)
        {
            arg1->m0_position = var38_previousResolvedPositionBackup;
            arg1->m0_position += getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

            arg1->mC_normal = var5C_transformedQuadNormal;

            var14_quadNormal = var44_previousResolvedPosition - r14.m0_translationToResolve;

            arg1->m18_penetrationDistance = r14.m10 - dot3_FP(&var14_quadNormal, &var14_quadNormal);

            if (var10_penetration > 0)
                return 1;

            r14.m0_translationToResolve[0] = var38_previousResolvedPositionBackup[0] + MTH_Mul(r14.mC_collisionRadius, var5C_transformedQuadNormal[0]) * 16;
            r14.m0_translationToResolve[1] = var38_previousResolvedPositionBackup[1] + MTH_Mul(r14.mC_collisionRadius, var5C_transformedQuadNormal[1]) * 16;
            r14.m0_translationToResolve[2] = var38_previousResolvedPositionBackup[2] + MTH_Mul(r14.mC_collisionRadius, var5C_transformedQuadNormal[2]) * 16;

            r11_pointToProject = r14.m0_translationToResolve;
            return 1;
        }

    } while (--var8_numTrianglesToProcess);

    return 0;
}

s32 collisionSub0(sProcessed3dModel* collisionMesh, sCollisionTempStruct& r5, sVec3_FP& r6, fixedPoint r7, std::array<s_visibilityGridWorkArea_5A8, 8>::iterator arg0)
{
    s_visibilityGridWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    transformCollisionVertices(collisionMesh->m4_numVertices, collisionMesh->m8_vertices, r13->m688_transformedCollisionVertices);

    s32 r11 = 0;

    for (int i = 0; i < collisionMesh->mC_Quads.size(); i++)
    {
        bool bCollisionHappened = false;
        if (testQuadForCollisions(collisionMesh->mC_Quads[i], r13->m688_transformedCollisionVertices, r5, r6, r7, arg0))
        {
            r11 = 1;
            r13->m1294.m10++;
            bCollisionHappened = true;
        }
        
        if(bDrawCollisions)
        {
            const sVec3_FP& position = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;
            const sVec3_FP vector0 = position + r13->m688_transformedCollisionVertices[collisionMesh->mC_Quads[i].m0_indices[0]];
            const sVec3_FP vector1 = position + r13->m688_transformedCollisionVertices[collisionMesh->mC_Quads[i].m0_indices[1]];
            const sVec3_FP vector2 = position + r13->m688_transformedCollisionVertices[collisionMesh->mC_Quads[i].m0_indices[2]];
            const sVec3_FP vector3 = position + r13->m688_transformedCollisionVertices[collisionMesh->mC_Quads[i].m0_indices[3]];

            if (bCollisionHappened)
            {
                sFColor quadColor = { 1,0,0,1 };
                drawDebugFilledQuad(vector0, vector1, vector2, vector3, quadColor);
            }
            else
            {
                sFColor quadColor = { 0,1,0,1 };
                drawDebugLine(vector0, vector1, quadColor);
                drawDebugLine(vector1, vector2, quadColor);
                drawDebugLine(vector2, vector3, quadColor);
                drawDebugLine(vector3, vector0, quadColor);
            }
        }

    }

    return r11;
}

void dragonFieldTaskUpdateSub1Sub1()
{
    if (ImGui::Begin("Collisions"))
    {
        ImGui::Checkbox("Draw Collisions", &bDrawCollisions);
    }
    ImGui::End();

    s_dragonTaskWorkArea* r14_pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    s_visibilityGridWorkArea* r12_pVisibilityGrid = getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1;

    // collisions enabled?
    if (!(r14_pDragonTask->mF8_Flags & 0x400))
        return;

    if (r14_pDragonTask->m249_noCollisionAndHideDragon)
        return;

    sCollisionTempStruct var60;
    var60.m0_translationToResolve.zero();
    fixedPoint var0 = 0x8000;

    var60.mC_collisionRadius = 0x8000;
    var60.m10 = MTH_Mul(0x8000, 0x8000);

    sVec2_FP var58;
    var58[0] = var60.mC_collisionRadius;
    var58[1] = var60.m10;
    sVec3_FP var40 = var60.m0_translationToResolve;

    sVec3_FP var4C_deltaPosition = r14_pDragonTask->m14_oldPos - r14_pDragonTask->m8_pos;

    r12_pVisibilityGrid->m48 = r12_pVisibilityGrid->m5A8.begin();
    r12_pVisibilityGrid->m40_activeCollisionEntriesCount = 0;

    if(0)
    {
        r14_pDragonTask->m8_pos[0] = 0x00601dd5;
        r14_pDragonTask->m8_pos[1] = 0x00051154;
        r14_pDragonTask->m8_pos[2] = 0xfeffc6ac;
    }

    // find all potential collision and increment m40 for each
    pushCurrentMatrix();
    for (int i = r12_pVisibilityGrid->m12E4_numCollisionGeometries - 1; i >= 0; i--)
    {
        //6070B40
        s_visibilityGridWorkArea_68* r13 = &r12_pVisibilityGrid->m68[i];
        copyToCurrentMatrix(&r13->m4_matrix);

        // put the object matrix in the dragon local space
        pCurrentMatrix->matrix[3] -= r14_pDragonTask->m8_pos[0];
        pCurrentMatrix->matrix[7] -= r14_pDragonTask->m8_pos[1];
        pCurrentMatrix->matrix[11] -= r14_pDragonTask->m8_pos[2];

        if (collisionSub0(r13->m0_model, var60, var40, r13->m34, r12_pVisibilityGrid->m48))
        {
            r12_pVisibilityGrid->m48++;
            r12_pVisibilityGrid->m40_activeCollisionEntriesCount++;
        }
    }
    popMatrix();

    if (r12_pVisibilityGrid->m40_activeCollisionEntriesCount)
    {
        //6070BB2
        if (!(r14_pDragonTask->mF8_Flags & 0x200))
        {
            r14_pDragonTask->mF8_Flags |= 0x300;
        }
        else
        {
            r14_pDragonTask->mF8_Flags &= ~0x100;
        }

        s_visibilityGridWorkArea_5A8* r13 = &r12_pVisibilityGrid->m5A8[0];
        for (int r4 = 1; r4 < r12_pVisibilityGrid->m40_activeCollisionEntriesCount; r4++)
        {
            if (r12_pVisibilityGrid->m5A8[r4].m18_penetrationDistance > r13->m18_penetrationDistance)
            {
                r13 = &r12_pVisibilityGrid->m5A8[r4];
            }
        }

        //06070C02
        r12_pVisibilityGrid->m4C = *r13;

        sVec3_FP var34;
        var34[0] = MTH_Mul(r13->m18_penetrationDistance, r13->mC_normal[0]);
        var34[1] = MTH_Mul(r13->m18_penetrationDistance, r13->mC_normal[1]);
        var34[2] = MTH_Mul(r13->m18_penetrationDistance, r13->mC_normal[2]);

        s32 var4_speedIndex;
        if (getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m235_dragonSpeedIndex >= 0)
        {
            var4_speedIndex = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m235_dragonSpeedIndex;
        }
        else
        {
            var4_speedIndex = 0;
        }

        r14_pDragonTask->m8_pos += var60.m0_translationToResolve;
        r14_pDragonTask->m160_deltaTranslation = r14_pDragonTask->m8_pos - r14_pDragonTask->m14_oldPos;

        sVec3_FP var10;
        var10[0] = -r14_pDragonTask->m88_matrix.matrix[2];
        var10[1] = r14_pDragonTask->m88_matrix.matrix[6];
        var10[2] = -r14_pDragonTask->m88_matrix.matrix[10];

        sVec3_FP var1C;
        var1C[0] = r14_pDragonTask->m160_deltaTranslation[0] << 16;
        var1C[1] = r14_pDragonTask->m160_deltaTranslation[1] << 16;
        var1C[2] = r14_pDragonTask->m160_deltaTranslation[2] << 16;

        fixedPoint varDragonSpeedValue = r14_pDragonTask->m21C_DragonSpeedValues[var4_speedIndex];
        if (r14_pDragonTask->m21C_DragonSpeedValues[var4_speedIndex] < sqrt_F(dot3_FP(&var1C, &var1C)).asS32() >> 8)
        {
            r14_pDragonTask->m154_dragonSpeed = varDragonSpeedValue;
        }
        else
        {
            r14_pDragonTask->m154_dragonSpeed = sqrt_F(dot3_FP(&var1C, &var1C)).asS32() >> 8;
        }

        //06070D98
        fixedPoint var28[2];
        if (gDragonState->mC_dragonType != DR_LEVEL_8_FLOATER)
        {
            generateCameraMatrixSub1(r14_pDragonTask->m160_deltaTranslation, var28);
            var28[0] = -var28[0];

            if (r14_pDragonTask->m154_dragonSpeed < 0x2AA)
            {
                var28[0] = 0;
                var28[1] = r14_pDragonTask->m20_angle[1];
            }
        }
        else
        {
            assert(0);
        }
        //06070E80
        r14_pDragonTask->m20_angle[0] = interpolateRotation(r14_pDragonTask->m20_angle[0], var28[0], 0x8000, 0x444444, 0);
        r14_pDragonTask->m20_angle[1] = interpolateRotation(r14_pDragonTask->m20_angle[1], var28[1], 0x2000, 0x444444, 0);
        dragonFieldTaskInitSub4Sub5(&r14_pDragonTask->m48, &r14_pDragonTask->m20_angle);
        return;
    }

    r14_pDragonTask->mF8_Flags &= ~0x300;

    if (gDragonState->mC_dragonType == DR_LEVEL_8_FLOATER)
    {
        r14_pDragonTask->m3C[0] = 0;
    }
}
