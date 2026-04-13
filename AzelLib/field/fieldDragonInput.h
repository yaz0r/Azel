#pragma once

// Shared dragon input/physics helper functions.
// These are compiled into every field overlay as identical copies.
// Moved from overlay-local code (A5/A7) to shared field code.

struct s_dragonTaskWorkArea;

// Dragon speed computation
void computeDragonSpeed(s_dragonTaskWorkArea* pDragon);

// 28-bit angle interpolation (15/16 smoothing toward target)
fixedPoint interpolateAngle28(fixedPoint current, fixedPoint target);

// Script/state queries
s32 isDragonScriptBlocked();
s32 isDragonFreeFlying();
s32 canAcceptDragonInput();

// Dragon velocity/state management
void clearDragonVelocity();
void clearDragonScriptFlag();
bool isBerserkUnlocked();
bool isLightWingUnlocked();

// Speed control
void resetSpeedIndex(s_dragonTaskWorkArea* pDragon);
void dragonSpeedControl(s_dragonTaskWorkArea* pDragon);

// Input state management
void resetDragonInput(s_dragonTaskWorkArea* pDragon);
void dragonInput_default(s_dragonTaskWorkArea* pDragon);

// Animation from input
void dragonAnimFromDPad(s_dragonTaskWorkArea* pDragon);
void dragonAnimFromAnalog(s_dragonTaskWorkArea* pDragon);

// Yaw interpolation / banking
void setYawInterpMode(s_dragonTaskWorkArea* pDragon);
void interpYawWithBanking(s_dragonTaskWorkArea* pDragon);

// Special moves
void dragonSpecialInput(s_dragonTaskWorkArea* pDragon);
void triggerBarrelRoll(s_dragonTaskWorkArea* pDragon);
void dragonAnalogSpecialInput(s_dragonTaskWorkArea* pDragon);

// Pitch/roll/yaw processing
void dragonDPadPitchRoll(s_dragonTaskWorkArea* pDragon);
void dragonAnalogPitchRoll(s_dragonTaskWorkArea* pDragon);

// Dragon update shared tail
void dragonUpdate_normalTail(s_dragonTaskWorkArea* pDragon);
void setDragonAnimationFromSpeed(s_dragonTaskWorkArea* pDragon);

// Dragon update state machine
void dragonUpdate_idle(s_dragonTaskWorkArea* pDragon);
void dragonUpdate_normal(s_dragonTaskWorkArea* pDragon);
void dragonUpdate_normal_type8(s_dragonTaskWorkArea* pDragon);
void dragonUpdate_cameraScript(s_dragonTaskWorkArea* pDragon);
void dragonUpdate_cutscene(s_dragonTaskWorkArea* pDragon);
void processCameraScript(s_dragonTaskWorkArea* pDragon, s_cameraScript* pScript);
void processCutscene(s_dragonTaskWorkArea* pDragon);
void dragonTransitionToNormal();
void dragonTransitionFromScript();

// Dragon input dispatchers
void dragonInput_digital(s_dragonTaskWorkArea* pDragon);
void dragonInput_analog(s_dragonTaskWorkArea* pDragon);
void dragonInput_digital_type8(s_dragonTaskWorkArea* pDragon);
void dragonInput_analog_type8(s_dragonTaskWorkArea* pDragon);

// Corridor autopilot
s32 checkDragonTransition(s_dragonTaskWorkArea* pDragon);
void computeBoundsPushback(s_dragonTaskWorkArea* pDragon);
void dragonUpdate_corridorAutopilot(s_dragonTaskWorkArea* pDragon);
void corridorEntryScript();
extern sSaturnPtr gCorridorEntryScriptEA; // set by each overlay during init
// initDragonMovementMode — declared in o_fld_a3.h

// Cutscene keyframe management
void setCutsceneKeyFrame(s_scriptData3* pKeyFrame);
void clearCutsceneKeyFrame();

// Script skipping + field exit combo
void enableScriptSkippingAndExit(s32 fieldIndex, s32 param, s32 exitNumber, s16 r7);
