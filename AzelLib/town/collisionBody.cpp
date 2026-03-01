#include "PDS.h"
#include "collisionBody.h"

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

// Prepends the body into resData's per-type linked list (keyed by m2C_collisionSetupIndex),
// then computes its world-space AABB center by rotating m20_AABBCenter by the owner's
// rotation and adding the owner's world position (*m30_pPosition).
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
