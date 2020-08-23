#pragma once

struct s_visibilityGridWorkArea : public s_workAreaTemplate<s_visibilityGridWorkArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { fieldCameraTask1Init, fieldCameraTask1Update, fieldCameraTask1Draw, NULL };
        return &taskDefinition;
    }

    static void fieldCameraTask1Init(s_visibilityGridWorkArea*);
    static void fieldCameraTask1Update(s_visibilityGridWorkArea*);
    static void fieldCameraTask1Draw(s_visibilityGridWorkArea*);

    sVec3_FP m0_position; // 0
    sVec3_FP mC;
    s32 m18_cameraGridLocation[2]; // 18 Grid location
    s32 m20_cellDimensions[2];
    u32 m28;
    std::vector<fixedPoint>* m2C_depthRangeTable;
    s_DataTable3* m30; // 30
    std::vector<std::vector<sCameraVisibility>>* m34_cameraVisibilityTable; // m34
    p_workArea m38; // 38
    s_visdibilityCellTask** m3C_cellRenderingTasks; // 3C (an array of tasks)
    s32 m40_activeCollisionEntriesCount;
    std::array<s_visibilityGridWorkArea_5A8, 8>::iterator m48;
    std::array<s_visibilityGridWorkArea_68, 24>::iterator m44;
    s_visibilityGridWorkArea_5A8 m4C;
    std::array<s_visibilityGridWorkArea_68, 24> m68;
    std::array<s_visibilityGridWorkArea_5A8, 8> m5A8;
    std::array<sVec3_FP, 256> m688_transformedCollisionVertices; // size is a guess
    u32 m128C_vdp2VramOffset2;
    u32 m1290_vdp2VramOffset;
    s_visibilityGridWorkArea_1294 m1294;
    sVec3_FP m12AC;
    sVec3_FP m12B8;
    sVec3_FP m12C4;
    sVec3_FP m12D0;
    s32 m12DC;
    u16 m12E0;
    u16 m12E2;
    u16 m12E4_numCollisionGeometries;
    u16 m12F0;
    u16 m12F2_renderMode; // 12F2
    u8 updateVisibleCells;
    u8(*m12F8_convertCameraPositionToGrid)(s_visibilityGridWorkArea* pFieldCameraTask1); // 12F8
    s32(*m12FC_isObjectClipped)(const sVec3_FP* r4, s32 r5); // 12F8
    u8 m1300;
    //size: 1304
};

u32 gridCellDraw_GetDepthRange(fixedPoint r4);
