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

// Initialises a collision body's type/layer from the preset table at index presetIndex.
void setCollisionSetup(sCollisionBody* pBody, s32 presetIndex);

// Computes AABB center, half-extents and bounding sphere radius from two corner points.
void setCollisionBounds(sCollisionBody*, const sVec3_FP&, const sVec3_FP&);

// A singly-linked list node referencing an active collision body for the current frame.
struct sCollisionBodyNode
{
    sCollisionBodyNode* m0_pNext;
    sCollisionBody* m4;
};

// Per-frame registry of active collision bodies organised into 5 per-type linked lists.
// Nodes are allocated from the pool m1C (capacity 0x3F); m8_headOfLinkedList[i] is the
// head of the linked list for collision setup index i.
struct sCollisionBodyRegistry
{
    s32 m0;
    s32 m4;
    std::array<sCollisionBodyNode*, 5> m8_headOfLinkedList;
    std::array<sCollisionBodyNode, 0x3F> m1C;
};
extern sCollisionBodyRegistry gCollisionRegistry;

// Inserts the collision body into the per-type linked list for this frame and
// updates its world-space AABB center position from the owner's position/rotation.
void registerCollisionBody(sCollisionBody*);

// Runs sphere broad-phase then per-face AABB collision tests for all registered bodies,
// resolves inter-body and body-vs-environment penetrations, and fires collision scripts.
void scriptUpdateSub0();

// Resets the per-frame collision registry (clears linked list heads, resets node counter)
// and toggles the resValue0 oscillator used by AABB positioning.
void initResTable();
