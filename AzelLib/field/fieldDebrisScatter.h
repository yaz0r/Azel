#pragma once

struct sModelHierarchy;

// 0607e252 — creates a cutscene debris scatter task
// Takes a model hierarchy and scatters its bones outward with physics
struct sDebrisScatterParams
{
    fixedPoint m0_gravity;      // gravity per frame
    fixedPoint m4_bounce;       // bounce factor
    fixedPoint m8_spread;       // initial velocity spread
    fixedPoint mC_randomMask;   // random velocity mask
    sVec3_FP* m10_pPosition;    // origin position
    sVec3_FP* m14_pRotation;    // origin rotation (or nullptr)
    fixedPoint m18_velX;        // base velocity X
    fixedPoint m1C_velY;        // base velocity Y
    fixedPoint m20_velZ;        // base velocity Z
    fixedPoint m24_rotVelX;     // rotation velocity X
    fixedPoint m28_rotVelY;     // rotation velocity Y
    fixedPoint m2C_rotVelZ;     // rotation velocity Z
    fixedPoint m30_angleOffset; // initial angle offset
    u16 m34_modelOffset;        // model hierarchy offset in file bundle
    u16 m36_poseOffset;         // pose data offset in file bundle
    fixedPoint m38_groundY;     // ground plane Y (bounce height)
    fixedPoint m3C_scale;       // scale (0 or 0x10000 = no scale)
    fixedPoint m40_param;       // additional param
    u16 m44_soundEffect;        // sound to play on ground hit
    struct s_fileBundle* m_pBundle; // file bundle for model data (C++ only — on Saturn, read from parent task offset 0)
};

p_workArea createDebrisScatterTask(p_workArea parent, sDebrisScatterParams* params, bool reverseMode);
