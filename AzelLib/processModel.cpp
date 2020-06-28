#include "PDS.h"
#include "processModel.h"
#include "3dEngine_textureCache.h"

sProcessed3dModel::sQuadExtra readExtraData(u8*& data, bool readColor)
{
    sProcessed3dModel::sQuadExtra newData;

    newData.m0_normals[0] = READ_BE_S16(data); data += 2;
    newData.m0_normals[1] = READ_BE_S16(data); data += 2;
    newData.m0_normals[2] = READ_BE_S16(data); data += 2;

    if (readColor)
    {
        newData.m6_colors[0] = READ_BE_U16(data); data += 2;
        newData.m6_colors[1] = READ_BE_U16(data); data += 2;
        newData.m6_colors[2] = READ_BE_U16(data); data += 2;
    }
    else
    {
        newData.m6_colors[0] = 0;
        newData.m6_colors[1] = 0;
        newData.m6_colors[2] = 0;
    }

    return newData;
}

sProcessed3dModel::sProcessed3dModel(u8* base, u32 offset)
{
    _base = base;

    u8* pRawModel = base + offset;

    m0_radius = READ_BE_S32(pRawModel + 0);
    m4_numVertices = READ_BE_U32(pRawModel + 4);
    u32 verticesOffset = READ_BE_U32(pRawModel + 8);
    m8_vertices.reserve(m4_numVertices);

    for (u32 i = 0; i < m4_numVertices; i++)
    {
        u8* startOfVertice = base + verticesOffset + (3 * 2 * i);

        sVec3_S16_12_4 vertice;
        vertice[0] = READ_BE_S16(startOfVertice + 0);
        vertice[1] = READ_BE_S16(startOfVertice + 2);
        vertice[2] = READ_BE_S16(startOfVertice + 4);

        m8_vertices.push_back(vertice);
    }

    u8* startOfQuad = pRawModel + 0xC;
    while (1)
    {
        sQuad newQuad;
        newQuad.m0_indices[0] = READ_BE_U16(startOfQuad + 0);
        newQuad.m0_indices[1] = READ_BE_U16(startOfQuad + 2);
        newQuad.m0_indices[2] = READ_BE_U16(startOfQuad + 4);
        newQuad.m0_indices[3] = READ_BE_U16(startOfQuad + 6);

        if ((newQuad.m0_indices[0] == 0) && (newQuad.m0_indices[1] == 0) && (newQuad.m0_indices[2] == 0) && (newQuad.m0_indices[3] == 0))
        {
            break;
        }

        startOfQuad += 8;

        newQuad.m8_lightingControl = READ_BE_U16(startOfQuad); startOfQuad += 2;
        newQuad.mA_CMDCTRL = READ_BE_U16(startOfQuad); startOfQuad += 2; // CMDCTRL (but modified)
        newQuad.mC_CMDPMOD = READ_BE_U16(startOfQuad); startOfQuad += 2; // CMDPMOD
        newQuad.mE_CMDCOLR = READ_BE_U16(startOfQuad); startOfQuad += 2; // CMDCOLR
        newQuad.m10_CMDSRCA = READ_BE_U16(startOfQuad); startOfQuad += 2; // CMDSRCA
        newQuad.m12_onCollisionScriptIndex = READ_BE_U16(startOfQuad); startOfQuad += 2; // CMDSIZEd

        u8 lightingMode = (newQuad.m8_lightingControl >> 8) & 3;

        switch (lightingMode)
        {
        case 0: // plain texture
            break;
        case 1: // texture + single normal, used for shadows
            newQuad.m14_extraData.push_back(readExtraData(startOfQuad, false));
            startOfQuad += 2; // 3 words + padding
            break;
        case 2: // texture + normal + color per vertex
            for (int i = 0; i < 4; i++)
            {
                newQuad.m14_extraData.push_back(readExtraData(startOfQuad, true));
            }
            break;
        case 3: // texture + normal per vertex
            for (int i = 0; i < 4; i++)
            {
                newQuad.m14_extraData.push_back(readExtraData(startOfQuad, false));
            }
            break;
        }

        mC_Quads.push_back(newQuad);
    }
}

void sProcessed3dModel::generateVertexBuffer()
{
    if (!m_vertexBuffersDirty)
        return;

    // generate atlas mapping
    struct sUniqueTextures
    {
        //u16 mA_CMDCTRL;
        u16 mC_CMDPMOD;
        u16 mE_CMDCOLR;
        u16 m10_CMDSRCA;
        u16 CMDSIZE;

        std::vector<int> m_quadsUsingThisTexture;
    };
    std::vector<sUniqueTextures> uniqueTextures;
    for (int i = 0; i < mC_Quads.size(); i++)
    {
        bool found = false;
        for(int j=0; j<uniqueTextures.size(); j++)
        {
            if (
                (mC_Quads[i].mC_CMDPMOD == uniqueTextures[j].mC_CMDPMOD) &&
                (mC_Quads[i].mE_CMDCOLR == uniqueTextures[j].mE_CMDCOLR) &&
                (mC_Quads[i].m10_CMDSRCA == uniqueTextures[j].m10_CMDSRCA) &&
                (mC_Quads[i].m12_onCollisionScriptIndex == uniqueTextures[j].CMDSIZE))
            {
                uniqueTextures[j].m_quadsUsingThisTexture.push_back(i);
                found = true;
                break;
            }
        }

        if (!found)
        {
            sUniqueTextures newUniqueTexture;
            newUniqueTexture.mC_CMDPMOD = mC_Quads[i].mC_CMDPMOD;
            newUniqueTexture.mE_CMDCOLR = mC_Quads[i].mE_CMDCOLR;
            newUniqueTexture.m10_CMDSRCA = mC_Quads[i].m10_CMDSRCA;
            newUniqueTexture.CMDSIZE = mC_Quads[i].m12_onCollisionScriptIndex;
            newUniqueTexture.m_quadsUsingThisTexture.push_back(i);
            uniqueTextures.push_back(newUniqueTexture);
        }
    }

    // generate atlas mapping
    struct s_atlasEntry
    {
        u16 CMDPMOD;
        u16 CMDCOLR;
        u16 CMDSRCA;
        u16 CMDSIZE;

        int startX;
        int startY;
        int width;
        int height;
        int uniqueTextureId;
    };
    std::vector<s_atlasEntry> atlasDefinition;
    atlasDefinition.reserve(uniqueTextures.size());

    int atlasTextureWidth = 0;
    int atlasTextureHeight = 0;
    for (int i = 0; i < uniqueTextures.size(); i++)
    {
        int textureWidth = (uniqueTextures[i].CMDSIZE & 0x3F00) >> 5;
        int textureHeight = uniqueTextures[i].CMDSIZE & 0xFF;

        s_atlasEntry newEntry;
        newEntry.CMDPMOD = uniqueTextures[i].mC_CMDPMOD;
        newEntry.CMDCOLR = uniqueTextures[i].mE_CMDCOLR;
        newEntry.CMDSRCA = uniqueTextures[i].m10_CMDSRCA;
        newEntry.CMDSIZE = uniqueTextures[i].CMDSIZE;
        newEntry.startX = 0;
        newEntry.startY = atlasTextureHeight;
        newEntry.width = textureWidth;
        newEntry.height = textureHeight;
        newEntry.uniqueTextureId = i;
        atlasDefinition.push_back(newEntry);

        atlasTextureWidth = std::max(atlasTextureWidth, textureWidth);
        atlasTextureHeight += textureHeight;
    }

    std::vector<u8> atlasTexture;
    atlasTexture.resize(atlasTextureWidth* atlasTextureHeight * 4);
    for (int i = 0; i < atlasDefinition.size(); i++)
    {
        s_quad tempQuad;
        tempQuad.CMDPMOD = atlasDefinition[i].CMDPMOD;
        tempQuad.CMDCOLR = atlasDefinition[i].CMDCOLR;
        tempQuad.CMDSRCA = atlasDefinition[i].CMDSRCA;
        tempQuad.CMDSIZE = atlasDefinition[i].CMDSIZE;
        tempQuad.model = _base;

        u16 entryWidth = 0;
        u16 entryHeight = 0;
        u32* pDecodedTexture = decodeVdp1Quad(tempQuad, entryWidth, entryHeight);

        u8* textureStartInAtlas = &atlasTexture[(atlasTextureWidth * atlasDefinition[i].startY + atlasDefinition[i].startX) * 4];
        u8* pCurrentScanlineInAtlas = textureStartInAtlas;
        for (int y = 0; y <entryHeight; y++)
        {
            memcpy(pCurrentScanlineInAtlas, pDecodedTexture + y * entryWidth, entryWidth * 4);
            pCurrentScanlineInAtlas += atlasTextureWidth * 4;
        }

        delete[] pDecodedTexture;
    }

    if (bgfx::isValid(m_textureAtlas))
    {
        bgfx::destroy(m_textureAtlas);
    }

    m_textureAtlas = bgfx::createTexture2D(atlasTextureWidth, atlasTextureHeight, false, 1, bgfx::TextureFormat::RGBA8, 0, bgfx::copy(&atlasTexture[0], atlasTextureWidth * atlasTextureHeight * 4));

    m_vertexBuffersDirty = false;
    m_vertexBuffer.resize(mC_Quads.size() * 4);
    m_indexBuffer.resize(mC_Quads.size() * 6);
    for (int i = 0; i < mC_Quads.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            sVec3_FP fpVec = m8_vertices[mC_Quads[i].m0_indices[j]].toSVec3_FP();
            m_vertexBuffer[i * 4 + j].position[0] = fpVec[0].toFloat();
            m_vertexBuffer[i * 4 + j].position[1] = fpVec[1].toFloat();
            m_vertexBuffer[i * 4 + j].position[2] = fpVec[2].toFloat();
        }

        m_indexBuffer[i * 6 + 0] = i * 4 + 0;
        m_indexBuffer[i * 6 + 1] = i * 4 + 1;
        m_indexBuffer[i * 6 + 2] = i * 4 + 2;
        m_indexBuffer[i * 6 + 3] = i * 4 + 0;
        m_indexBuffer[i * 6 + 4] = i * 4 + 2;
        m_indexBuffer[i * 6 + 5] = i * 4 + 3;

        // setup the UVs
        //find the entry in the atlas
        int atlasEntry = -1;
        for (int j = 0; j < atlasDefinition.size(); j++)
        {
            for (int k = 0; k < uniqueTextures[atlasDefinition[j].uniqueTextureId].m_quadsUsingThisTexture.size(); k++)
            {
                if (uniqueTextures[atlasDefinition[j].uniqueTextureId].m_quadsUsingThisTexture[k] == i)
                {
                    atlasEntry = j;
                }
            }
        }
        assert(atlasEntry != -1);

        int flip = (mC_Quads[i].mA_CMDCTRL & 0x30) >> 4;
        float uv[4][2];
        uv[0][0] = atlasDefinition[atlasEntry].startX + 0.5f;
        uv[0][1] = atlasDefinition[atlasEntry].startY + 0.5f;

        uv[1][0] = atlasDefinition[atlasEntry].startX + atlasDefinition[atlasEntry].width - 0.5f;
        uv[1][1] = atlasDefinition[atlasEntry].startY + 0.5f;

        uv[2][0] = atlasDefinition[atlasEntry].startX + atlasDefinition[atlasEntry].width - 0.5f;
        uv[2][1] = atlasDefinition[atlasEntry].startY + atlasDefinition[atlasEntry].height - 0.5f;

        uv[3][0] = atlasDefinition[atlasEntry].startX + 0.5f;
        uv[3][1] = atlasDefinition[atlasEntry].startY + atlasDefinition[atlasEntry].height - 0.5f;

        char vertexOrder[4] = { 0, 1, 2, 3 };

        switch (flip & 3)
        {
        case 1:
            vertexOrder[0] = 1;
            vertexOrder[1] = 0;
            vertexOrder[2] = 3;
            vertexOrder[3] = 2;
            break;
        case 2:
            vertexOrder[0] = 3;
            vertexOrder[1] = 2;
            vertexOrder[2] = 1;
            vertexOrder[3] = 0;
            break;
        case 3:
            vertexOrder[0] = 2;
            vertexOrder[1] = 3;
            vertexOrder[2] = 0;
            vertexOrder[3] = 1;
            break;
        }

        for (int j = 0; j < 4; j++)
        {
            m_vertexBuffer[i * 4 + j].texcoord0[0] = uv[vertexOrder[j]][0]/* / atlasTextureWidth */;
            m_vertexBuffer[i * 4 + j].texcoord0[1] = uv[vertexOrder[j]][1]/* / atlasTextureHeight */;

        }
    }

    m_vertexLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    if (bgfx::isValid(m_vertexBufferHandle))
    {
        bgfx::destroy(m_vertexBufferHandle);
    }

    if (bgfx::isValid(m_indexBufferHandle))
    {
        bgfx::destroy(m_indexBufferHandle);
    }

    m_vertexBufferHandle = bgfx::createVertexBuffer(bgfx::copy(&m_vertexBuffer[0], sizeof(m_vertexBuffer[0]) * m_vertexBuffer.size()), m_vertexLayout);
    m_indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy(&m_indexBuffer[0], sizeof(m_indexBuffer[0]) * m_indexBuffer.size()));
}

