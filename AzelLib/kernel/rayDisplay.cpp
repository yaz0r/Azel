#include "PDS.h"
#include "rayDisplay.h"
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "renderer.h"
#include "3dEngine_textureCache.h"

bool gDirectRayRendering = true;

struct sRayVertex3D {
    float positions[3];
    float textures[2];
    float color[4];
};

struct sRayQuad3D {
    sRayVertex3D vertices[4];
    u16 CMDCTRL;
    u16 CMDPMOD;
    u16 CMDCOLR;
    u16 CMDSRCA;
    u16 CMDSIZE;
};

std::vector<sRayQuad3D> gPendingRayQuads;


s32 isGunShotVisible(std::array<sVec3_FP, 2>& param_1, s_graphicEngineStatus_405C& param_2)
{
    Unimplemented();
    return 1;
}


s32 rayComputeDisplayMatrix_fixedWidth(std::array<sVec3_FP, 2>& param_1, s32 param_2, s_graphicEngineStatus_405C& param_3, sScreenQuad3& param_4)
{
    if (isGunShotVisible(param_1, param_3))
    {
        sVec2_FP local_38;
        fixedPoint ratio0 = FP_Div(0x10000, param_1[0][2]);
        local_38[0] = MTH_Mul_5_6(param_3.m18_widthScale, param_1[0][0], ratio0);
        local_38[1] = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[0][1], ratio0);

        sVec2_FP local_2c;
        fixedPoint ratio1 = FP_Div(0x10000, param_1[1][2]);
        local_2c[0] = MTH_Mul_5_6(param_3.m18_widthScale, param_1[1][0], ratio1);
        local_2c[1] = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[1][1], ratio1);

        s32 angle = atan2(local_38[1] - local_2c[1], local_38[0] - local_2c[0]);

        sVec2_FP iVar9;
        iVar9[0] = MTH_Mul_5_6(param_3.m18_widthScale, MTH_Mul(param_2, getSin(angle)), ratio0);
        iVar9[1] = MTH_Mul_5_6(param_3.m1C_heightScale, MTH_Mul(param_2, getCos(angle)), ratio0);

        param_4[0][0] = local_38[0] - iVar9[0];
        param_4[0][1] = local_38[1] + iVar9[1];
        param_4[3][0] = local_38[0] + iVar9[0];
        param_4[3][1] = local_38[1] - iVar9[1];

        sVec2_FP iVar3;
        iVar3[0] = MTH_Mul_5_6(param_3.m18_widthScale, MTH_Mul(param_2, getSin(angle)), ratio1);
        iVar3[1] = MTH_Mul_5_6(param_3.m1C_heightScale, MTH_Mul(param_2, getCos(angle)), ratio1);

        param_4[1][0] = local_2c[0] - iVar3[0];
        param_4[1][1] = local_2c[1] + iVar3[1];
        param_4[2][0] = local_2c[0] + iVar3[0];
        param_4[2][1] = local_2c[1] - iVar3[1];

        return 1;
    }

    return 0;
}

s32 rayComputeDisplayMatrix_2Width(std::array<sVec3_FP, 2>& param_1, std::array<fixedPoint, 2>& param_2, s_graphicEngineStatus_405C& param_3, sScreenQuad3& param_4)
{
    if (isGunShotVisible(param_1, param_3))
    {
        fixedPoint ratio0 = FP_Div(0x10000, param_1[0][2]);
        fixedPoint iVar2 = MTH_Mul_5_6(param_3.m18_widthScale, param_1[0][0], ratio0);
        fixedPoint iVar3 = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[0][1], ratio0);

        fixedPoint ratio1 = FP_Div(0x10000, param_1[1][2]);
        fixedPoint iVar4 = MTH_Mul_5_6(param_3.m18_widthScale, param_1[1][0], ratio1);
        fixedPoint iVar5 = MTH_Mul_5_6(param_3.m1C_heightScale, param_1[1][1], ratio1);

        fixedPoint angle = atan2(iVar3 - iVar5, iVar2 - iVar4);

        {
            fixedPoint iVar6 = MTH_Mul(param_2[0], getSin(angle));
            fixedPoint iVar7 = MTH_Mul(param_2[0], getCos(angle));

            iVar6 = MTH_Mul_5_6(param_3.m18_widthScale, iVar6, ratio0);
            iVar7 = MTH_Mul_5_6(param_3.m1C_heightScale, iVar7, ratio0);

            param_4[0][0] = iVar2 - iVar6;
            param_4[0][1] = iVar3 + iVar7;
            param_4[3][0] = iVar2 + iVar6;
            param_4[3][1] = iVar3 - iVar7;
        }

        {
            fixedPoint iVar2 = MTH_Mul(param_2[1], getSin(angle));
            fixedPoint iVar3 = MTH_Mul(param_2[1], getCos(angle));

            iVar2 = MTH_Mul_5_6(param_3.m18_widthScale, iVar2, ratio1);
            iVar3 = MTH_Mul_5_6(param_3.m1C_heightScale, iVar3, ratio1);

            param_4[1][0] = iVar4 - iVar2;
            param_4[1][1] = iVar5 + iVar3;
            param_4[2][0] = iVar4 + iVar2;
            param_4[2][1] = iVar5 - iVar3;
        }

        return 1;
    }

    return 0;
}



void sendRaySegmentToVdp1(sScreenQuad3& projectedCoordinates, fixedPoint& depth, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1002; // command 0
    vdp1WriteEA.m4_CMDPMOD = 0x484 | colorMode; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = characterColor; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = characterAddress; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = characterSize; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = projectedCoordinates[0][0]; // CMDXA
    vdp1WriteEA.mE_CMDYA = -projectedCoordinates[0][1]; // CMDYA
    vdp1WriteEA.m10_CMDXB = projectedCoordinates[1][0]; // CMDXB
    vdp1WriteEA.m12_CMDYB = -projectedCoordinates[1][1]; // CMDYB
    vdp1WriteEA.m14_CMDXC = projectedCoordinates[2][0]; // CMDXC
    vdp1WriteEA.m16_CMDYC = -projectedCoordinates[2][1]; // CMDYC
    vdp1WriteEA.m18_CMDXD = projectedCoordinates[3][0]; // CMDXD
    vdp1WriteEA.m1A_CMDYD = -projectedCoordinates[3][1]; // CMDYD

    int outputColorIndex = graphicEngineStatus.m14_vdp1Context[0].m10 - graphicEngineStatus.m14_vdp1Context[0].m14->begin();
    quadColor& outputColor = *(graphicEngineStatus.m14_vdp1Context[0].m10++);
    outputColor = *pQuadColor;
    vdp1WriteEA.m1C_CMDGRA = outputColorIndex;

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;

}


static void computeRayVertexColor(u16 gouraud, float out[4])
{
    // Saturn VDP1 gouraud: additive signed offset per channel
    // Each channel is 5 bits, 16 = neutral (no offset)
    // Offset range: -16 to +15, applied to RGB555 pixel values (0-31)
    // Normalize to 0-1 range for the shader
    float r = (float)((s32)((gouraud >> 10) & 0x1F) - 16) / 31.0f;
    float g = (float)((s32)((gouraud >> 5) & 0x1F) - 16) / 31.0f;
    float b = (float)((s32)((gouraud >> 0) & 0x1F) - 16) / 31.0f;

    out[0] = r;
    out[1] = g;
    out[2] = b;
    out[3] = 1.0f;
}

static void enqueueRaySegment3D(std::array<sVec3_FP, 2>& viewSpacePoints, s32 width, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    glm::vec3 p0(viewSpacePoints[0][0].toFloat(), viewSpacePoints[0][1].toFloat(), viewSpacePoints[0][2].toFloat());
    glm::vec3 p1(viewSpacePoints[1][0].toFloat(), viewSpacePoints[1][1].toFloat(), viewSpacePoints[1][2].toFloat());

    glm::vec3 dir = p1 - p0;
    float len = glm::length(dir);
    if (len < 0.001f) return;
    dir /= len;

    // View direction (camera looks down -Z in view space)
    glm::vec3 viewDir(0.0f, 0.0f, -1.0f);

    glm::vec3 perp = glm::cross(dir, viewDir);
    float perpLen = glm::length(perp);
    if (perpLen < 0.0001f) {
        perp = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));
        perpLen = glm::length(perp);
        if (perpLen < 0.0001f) return;
    }
    perp /= perpLen;

    float halfWidth = width / (float)0x10000;

    glm::vec3 v0 = p0 - perp * halfWidth;
    glm::vec3 v1 = p1 - perp * halfWidth;
    glm::vec3 v2 = p1 + perp * halfWidth;
    glm::vec3 v3 = p0 + perp * halfWidth;

    sRayQuad3D quad;
    float c0[4], c1[4], c2[4], c3[4];
    computeRayVertexColor((*pQuadColor)[0], c0);
    computeRayVertexColor((*pQuadColor)[1], c1);
    computeRayVertexColor((*pQuadColor)[2], c2);
    computeRayVertexColor((*pQuadColor)[3], c3);

    quad.vertices[0] = { {v0.x, v0.y, v0.z}, {0,0}, {c0[0], c0[1], c0[2], c0[3]} };
    quad.vertices[1] = { {v1.x, v1.y, v1.z}, {1,0}, {c1[0], c1[1], c1[2], c1[3]} };
    quad.vertices[2] = { {v2.x, v2.y, v2.z}, {1,1}, {c2[0], c2[1], c2[2], c2[3]} };
    quad.vertices[3] = { {v3.x, v3.y, v3.z}, {0,1}, {c3[0], c3[1], c3[2], c3[3]} };

    quad.CMDCTRL = 0x1002;
    quad.CMDPMOD = 0x484 | colorMode;
    quad.CMDCOLR = characterColor;
    quad.CMDSRCA = characterAddress;
    quad.CMDSIZE = characterSize;

    gPendingRayQuads.push_back(quad);
}

void displayRaySegment(std::array<sVec3_FP, 2>& param_1, s32 param_2, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    std::array<sVec3_FP, 2> transformedPoints;
    transformAndAddVecByCurrentMatrix(&param_1[0], &transformedPoints[0]);
    transformAndAddVecByCurrentMatrix(&param_1[1], &transformedPoints[1]);

    if (gDirectRayRendering)
    {
        enqueueRaySegment3D(transformedPoints, param_2, characterAddress, characterSize, characterColor, pQuadColor, colorMode);
        return;
    }

    sScreenQuad3 screenQuad;

    if (rayComputeDisplayMatrix_fixedWidth(transformedPoints, param_2, graphicEngineStatus.m405C, screenQuad))
    {
        //if (sGunShotTask_DrawSub1Sub1(local_50, graphicEngineStatus.m405C) && sGunShotTask_DrawSub1Sub2(local_50))
        Unimplemented();
        {
            fixedPoint depth = transformedPoints[1][2];
            sendRaySegmentToVdp1(screenQuad, depth, characterAddress, characterSize, characterColor, pQuadColor, colorMode);
        }
    }
}

void flushRayQuads3D()
{
    if (gPendingRayQuads.empty()) return;

    // Forward declarations from 3dEngine_flush.cpp
    glm::mat4 getProjectionMatrix();
    bgfx::ProgramHandle loadBgfxProgram(const std::string& VSFile, const std::string& PSFile);
    bgfx::TextureHandle getTextureForQuadBGFX(s_quad& quad);

    static bgfx::ProgramHandle worldTexturedProgram = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(worldTexturedProgram))
    {
        worldTexturedProgram = loadBgfxProgram("VDP1_worldTextured_vs", "VDP1_worldTextured_ps");
    }

    static bgfx::UniformHandle vdp1_modelViewProj = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(vdp1_modelViewProj))
    {
        vdp1_modelViewProj = bgfx::createUniform("u_customModelViewProj", bgfx::UniformType::Mat4);
    }

    static bgfx::UniformHandle u_spritePriority = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(u_spritePriority))
    {
        u_spritePriority = bgfx::createUniform("u_spritePriority", bgfx::UniformType::Vec4);
    }

    static bgfx::UniformHandle textureUniform = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(textureUniform))
    {
        textureUniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
    }

    bgfx::VertexLayout layout;
    layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();

    glm::mat4 projectionMatrix = getProjectionMatrix();

    u16 indices[6] = { 2, 1, 0, 0, 3, 2 };

    for (auto& rayQuad : gPendingRayQuads)
    {
        bgfx::TransientVertexBuffer vertexBuffer;
        bgfx::TransientIndexBuffer indexBuffer;
        if (!bgfx::allocTransientBuffers(&vertexBuffer, layout, 4, &indexBuffer, 6))
            break;

        memcpy(vertexBuffer.data, rayQuad.vertices, sizeof(rayQuad.vertices));
        memcpy(indexBuffer.data, indices, sizeof(indices));

        // Create texture from VDP1 sprite data
        s_quad tempQuad = {};
        tempQuad.CMDCTRL = rayQuad.CMDCTRL;
        tempQuad.CMDPMOD = rayQuad.CMDPMOD;
        tempQuad.CMDCOLR = rayQuad.CMDCOLR;
        tempQuad.CMDSRCA = rayQuad.CMDSRCA;
        tempQuad.CMDSIZE = rayQuad.CMDSIZE;

        bgfx::setUniform(vdp1_modelViewProj, &projectionMatrix[0][0]);

        float priority[4] = { (float)(vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA & 7), 0, 0, 0 };
        bgfx::setUniform(u_spritePriority, priority);

        bgfx::setTexture(0, textureUniform, getTextureForQuadBGFX(tempQuad));
        bgfx::setVertexBuffer(0, &vertexBuffer);
        bgfx::setIndexBuffer(&indexBuffer);

        bgfx::setState(0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_MSAA
        );

        bgfx::submit(vdp1_gpuView, worldTexturedProgram);
    }

    gPendingRayQuads.clear();
}
