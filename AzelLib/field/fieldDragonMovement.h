#pragma once

struct s_dragonTaskWorkArea;

// Shared dragon movement functions used across all field overlays

// Dragon update modes (set on mF0)
void dragonIdleUpdate(s_dragonTaskWorkArea*);
void dragonCutsceneUpdate(s_dragonTaskWorkArea*);
void dragonFlightUpdate(s_dragonTaskWorkArea*);
void DragonUpdateCutscene(s_dragonTaskWorkArea*);
void dragonExitField(s_dragonTaskWorkArea*);
void dragonScriptMovement(s_dragonTaskWorkArea*);

// Input handlers
void updateDragonMovementDigital(s_dragonTaskWorkArea*);
void updateDragonMovementAnalog(s_dragonTaskWorkArea*);

// Movement integration
void integrateDragonMovement(s_dragonTaskWorkArea*);

// Utility
s32 isDragonInFieldBounds(s_dragonTaskWorkArea*);

// Type 8 (floater) dragon movement
void floaterFlightUpdate(s_dragonTaskWorkArea*);

// Activate dragon flight mode (selects normal or type 8 update, resets script status)
void activateDragonFlight();

