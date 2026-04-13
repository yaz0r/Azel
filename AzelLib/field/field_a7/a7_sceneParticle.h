#pragma once

#include "field/fieldSceneManager.h"

// sA7SceneParticleDesc is now sSceneParticleDesc in fieldSceneManager.h
#define sA7SceneParticleDesc sSceneParticleDesc

// Shared functions — now in fieldSceneManager.cpp, aliased for existing A7 callers
#define a7SceneParticle_UpdatePhysics_06076f28 sceneParticle_updatePhysics
#define a7SceneParticle_allocate sceneParticle_allocate
#define a7SceneParticle_spawnProjected sceneParticle_spawnProjected
