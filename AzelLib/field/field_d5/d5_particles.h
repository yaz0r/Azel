#pragma once

struct sAnimatedQuad;

struct s_nameEntryParticle
{
    sAnimatedQuad m0_quad;       // 0x00: sprite animation data (8 bytes)
    sVec3_FP m8_position;       // 0x08: current position (12 bytes)
    fixedPoint m14_velocityX;    // 0x14
    fixedPoint m18_velocityY;    // 0x18
    // size 0x1C
};

struct s_nameEntryParticleTask : public s_workAreaTemplate<s_nameEntryParticleTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(s_nameEntryParticleTask* pThis);
    static void Update(s_nameEntryParticleTask* pThis);
    static void Draw(s_nameEntryParticleTask* pThis);

    s_fileBundle* m0_bundle;                // 0x00
    u32 m4_characterArea;                   // 0x04
    u32 m8_pad[8];                          // 0x08
    u32 m28_state;                          // 0x28: 0=normal, 2=fade-out
    u32 m2C_pad;                            // 0x2C
    s_nameEntryParticle m30_particles[64];  // 0x30: 64 particles * 0x1C = 0x700
    // Saturn size 0x730
};

s_nameEntryParticleTask* createParticleSystemTask(p_workArea parent);
