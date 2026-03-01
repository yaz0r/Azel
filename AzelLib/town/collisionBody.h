#pragma once

struct sCollisionSetup
{
    s8 m0_collisionType;
    s8 m1;
    s16 m2_collisionLayersBitField;
};

struct sCollisionBody
{
    sCollisionSetup m0_collisionSetup;
    fixedPoint m4_sphereRadius;
    sVec3_FP m8_position;
    sVec3_FP m14_halfAABB;
    sVec3_FP m20_AABBCenter;
    s32 m2C_collisionSetupIndex;
    sVec3_FP* m30_pPosition;
    sVec3_FP* m34_pRotation;
    p_workArea m38_pOwner;
    sSaturnPtr m3C_scriptEA;
    u8* m40;
    s32 m44;
    sCollisionBody* m48;
    sVec3_FP m4C;
    sVec3_FP m58_collisionSolveTranslation;
};

struct sCollisionBodyNode
{
    sCollisionBodyNode* m0_pNext;
    sCollisionBody* m4;
};

struct sCollisionBodyRegistry
{
    s32 m0{};
    s32 m4{};
    std::array<sCollisionBodyNode*, 5>m8_headOfLinkedList{};
    std::array<sCollisionBodyNode, 0x3F>m1C{};
};
extern sCollisionBodyRegistry gCollisionRegistry;


// Initialises a collision body's type/layer from the preset table.
void setCollisionSetup(sCollisionBody* pBody, s32 presetIndex);

// Computes AABB center, half-extents and bounding sphere radius from two corner points.
void setCollisionBounds(sCollisionBody*, const sVec3_FP&, const sVec3_FP&);

// Inserts the collision body into the per-type linked list for this frame and
// updates its world-space AABB center position from the owner's position/rotation.
void registerCollisionBody(sCollisionBody*);
