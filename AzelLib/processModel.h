#pragma once

struct sProcessed3dModel
{
    struct sQuadExtra
    {
        sVec3_S16_12_4 m0_normals;
        sVec3_U16 m6_colors;
    };

    struct sQuad
    {
        std::array<u16, 4> m0_indices;
        u16 m8_lightingControl;
        u16 mA_CMDCTRL;
        u16 mC_CMDPMOD;
        u16 mE_CMDCOLR;
        u16 m10_CMDSRCA;
        u16 m12_onCollisionScriptIndex;
        std::vector<sQuadExtra> m14_extraData;
    };

    sProcessed3dModel(u8* base, u32 offset);

    // build from raw data
    sProcessed3dModel(const fixedPoint& radius, const std::vector<sVec3_S16_12_4>& vertices, const std::vector<sQuad>& quads)
    {
        _base = nullptr;
        m0_radius = radius;
        m4_numVertices = (u32)vertices.size();
        m8_vertices = vertices;
        mC_Quads = quads;
    }

    u8* _base;

    fixedPoint m0_radius;
    u32 m4_numVertices;
    std::vector<sVec3_S16_12_4> m8_vertices;
    std::vector<sQuad> mC_Quads;

    void patchFilePointers(u32 offset)
    {
        for (int i = 0; i < mC_Quads.size(); i++)
        {
            mC_Quads[i].mE_CMDCOLR += offset;
            mC_Quads[i].m10_CMDSRCA += offset;
        }
    }

    void generateVertexBuffer();

    struct sVertexBufferEntry
    {
        float position[3];
        float texcoord0[2];
    };
    std::vector<sVertexBufferEntry> m_vertexBuffer;
    std::vector<u16> m_indexBuffer;

    bgfx::VertexLayout m_vertexLayout;
    bgfx::VertexBufferHandle m_vertexBufferHandle = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle m_indexBufferHandle = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle m_textureAtlas = BGFX_INVALID_HANDLE;

    bool m_vertexBuffersDirty = true;
};
