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

    sProcessed3dModel(u8* base, u32 offset)
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

    sQuadExtra readExtraData(u8*& data, bool readColor)
    {
        sQuadExtra newData;

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
};
