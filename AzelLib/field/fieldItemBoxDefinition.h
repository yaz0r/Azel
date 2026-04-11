#pragma once

enum itemBoxType : s8 {
    itemBox_normal = 0,
    itemBox_map = 1,
    itemBox_save = 2,
};

struct s_itemBoxDefinition
{
    sVec3_FP m0_pos;
    sVec3_FP mC_boundingMin;
    sVec3_FP m18_boundingMax;
    sVec3_FP m24_rotation;
    fixedPoint m30_scale;
    s32 m34_bitIndex;
    s32 m38;
    eItems m3C_receivedItemId; // TODO: was 32bit in original, but no need
    s8 m40_receivedItemQuantity;
    itemBoxType m41_LCSType;
    s8 m42;
    s8 m43;
    s8 m44;
};

s_itemBoxDefinition* readItemBoxDefinition(sSaturnPtr ptr);
