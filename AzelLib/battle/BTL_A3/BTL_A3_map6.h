#pragma once

struct s_BTL_A3_Env : public s_workAreaTemplate<s_BTL_A3_Env>
{
    sVec3_FP mC_cameraPosition;
    sVec3_FP m18_cameraRotation;
    std::array<s16, 4> m24_vdp1Clipping;
    std::array<s16, 2> m2C_vdp1LocalCoordinates;
    std::array<s16, 2> m30_vdp1ProjectionParam;
    s32 m34;
    s32 m38;
    s32 m3C;
    sSaturnPtr m40;
    u32 m44;
    u32 m48;
    s32 m4C;
    s8 m50;
    s8 m51;
    s8 m52;
    s8 m55;
    npcFileDeleter* m58;
    // 0x9C
};

//TODO: move to kernel
void setupRotationMapPlanes(int rotationMapIndex, const std::array<u32, 16>& planes);
void setupRotationMapPlanes(int rotationMapIndex, sSaturnPtr inPlanes);
void setupScrollAndRotation(int p1, void* p2, void* p3, u8* coefficientTableAddress, u8 p5);
void setupVdp2Table(int p1, std::vector<fixedPoint>& p2, std::vector<fixedPoint>& p3, u8* coefficientTableAddress, u8 p5);
void s_BTL_A3_Env_InitVdp2Sub3(int layerIndex, u8* table);
void s_BTL_A3_Env_InitVdp2Sub4(sSaturnPtr);
extern tCoefficientTable coefficientA0;
extern tCoefficientTable coefficientA1;
extern tCoefficientTable coefficientB0;
extern tCoefficientTable coefficientB1;

void initGridForBattle(npcFileDeleter* pFile, const struct sGrid* pGrid, s32 r6_sizeX, s32 r7_sizeY, s32 r8_cellSize);
void BTL_A3_Env_InitVdp2(s_BTL_A3_Env* pThis);
void BTL_A3_Env_Update(s_BTL_A3_Env* pThis);
void BTL_A3_Env_Draw(s_BTL_A3_Env* pThis);

p_workArea Create_BTL_A3_map6(p_workArea);

struct sCoefficientTableData
{
    s32 m0;     // Xst
    s32 m4;     // Yst
    s32 m8_Zst;     // Zst
    s32 mC;     // DXx
    s32 m10;    // DXy
    s32 m14;    // DYx
    s32 m18;    // DYy
    s32 m1C;    // A (rotation matrix element)
    s32 m20;    // B
    s32 m24;    // C
    s32 m28;    // D
    s32 m2C;    // E
    s32 m30;    // F
    s16 m34;    // Px (pivot point X)
    s16 m36;    // Py (pivot point Y)
    s16 m38;    // Pz (focal length)
    s16 m3A;    // (pad)
    s16 m3C;    // Cx (center X)
    s16 m3E;    // Cy (center Y)
    s16 m40;    // Cz (center Z)
    s16 m42;    // (pad)
    s32 m44;    // Mx
    s32 m48;    // My
    s32 m4C;
    s32 m50;
    s32 m54;    // KAst (coefficient table start)
    s32 m58;    // DKAx (coefficient X increment)
    s32 m5C;    // DKAy (coefficient Y increment)
    s32 m60;
    s32 m64;
    s32 m68;
    s32 m6C;
    // size 0x70
};
extern std::array<std::array<sCoefficientTableData, 2>, 2> gCoefficientTables;
extern std::array<std::array<std::vector<fixedPoint>*, 2>, 2> gVdp2CoefficientTables;

enum eRotationMatrixMode : s8
{
    eRMM_roll     = 0,
    eRMM_pitchYaw = 1,
    eRMM_params   = 2,
};

struct sRotationPassState
{
    s8 m0_planeIndex;       // which rotation pass (0 or 1)
    s8 m1_scrollMode;      // scroll orientation (0=default, 1=alternate)
    eRotationMatrixMode m2_matrixMode;  // last matrix-build call
    fixedPoint m4_focalLength;
    // size 0x8
};
extern sRotationPassState gRotationPassState;

struct sVdp2RotationData
{
    fixedPoint m[3][3];     // rotation matrix
    fixedPoint Mx, My, Mz;  // map translation
};
extern sVdp2RotationData gVdp2RotationMatrix;

void beginRotationPass(int passIndex, fixedPoint focalLength);
void beginRotationPass_resetMatrix();
void commitRotationPass();
void buildRotationMatrixRoll(fixedPoint rollAngle);
void scaleRotationMatrix(fixedPoint scale);
void setRotationScrollOffset(s32 scrollX, s32 scrollY);
void buildRotationMatrixPitchYaw(fixedPoint pitchAngle, fixedPoint yawAngle);
void writeRotationParams(fixedPoint rollAngle);
s32 computeRotationScrollOffset();
void buildGroundRotation(s_BTL_A3_Env* pThis);
