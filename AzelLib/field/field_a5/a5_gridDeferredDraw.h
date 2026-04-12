#pragma once

// A5-specific deferred-draw helpers. Most shared functions live in
// field.h / LCS.h — only truly A5-specific variants remain here.

struct s_fileBundle;

// 06077074 — variant of gridCellDraw_normalSub2 that takes an explicit
// world-space (position, rotation) pair instead of using the current
// matrix. Culls against the dragon's `m8_pos` directly (axis-aligned),
// builds a local world matrix, and enqueues the draw with depth 0x10000.
void a5GridDraw_cullAndEnqueueWithTransform_06077074(
    s_fileBundle* pBundle, s32 bundleKey, const sVec3_FP* pPos, const sVec3_FP* pRot);
