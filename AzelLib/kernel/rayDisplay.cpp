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
};

struct sRayQuad3D {
    sRayVertex3D vertices[4];
    float gouraud[4]; // average gouraud tint (r, g, b, a)
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
    // Saturn VDP1 gouraud RGB555: 1BBBBBGGGGGRRRRR
    float r = (float)((s32)((gouraud >> 0) & 0x1F) - 16) / 31.0f;
    float g = (float)((s32)((gouraud >> 5) & 0x1F) - 16) / 31.0f;
    float b = (float)((s32)((gouraud >> 10) & 0x1F) - 16) / 31.0f;

    out[0] = r;
    out[1] = g;
    out[2] = b;
    out[3] = 1.0f;
}

static void computeAverageGouraud(const quadColor* pQuadColor, float out[4])
{
    float c0[4], c1[4], c2[4], c3[4];
    computeRayVertexColor((*pQuadColor)[0], c0);
    computeRayVertexColor((*pQuadColor)[1], c1);
    computeRayVertexColor((*pQuadColor)[2], c2);
    computeRayVertexColor((*pQuadColor)[3], c3);

    out[0] = (c0[0] + c1[0] + c2[0] + c3[0]) * 0.25f;
    out[1] = (c0[1] + c1[1] + c2[1] + c3[1]) * 0.25f;
    out[2] = (c0[2] + c1[2] + c2[2] + c3[2]) * 0.25f;
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
    quad.vertices[0] = { {v0.x, v0.y, v0.z}, {0,0} };
    quad.vertices[1] = { {v1.x, v1.y, v1.z}, {1,0} };
    quad.vertices[2] = { {v2.x, v2.y, v2.z}, {1,1} };
    quad.vertices[3] = { {v3.x, v3.y, v3.z}, {0,1} };

    computeAverageGouraud(pQuadColor, quad.gouraud);

    quad.CMDCTRL = 0x1002;
    quad.CMDPMOD = 0x484 | colorMode;
    quad.CMDCOLR = characterColor;
    quad.CMDSRCA = characterAddress;
    quad.CMDSIZE = characterSize;

    gPendingRayQuads.push_back(quad);
}

static void enqueueRaySegment3D_2Width(std::array<sVec3_FP, 2>& viewSpacePoints, std::array<fixedPoint, 2>& widths, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    glm::vec3 p0(viewSpacePoints[0][0].toFloat(), viewSpacePoints[0][1].toFloat(), viewSpacePoints[0][2].toFloat());
    glm::vec3 p1(viewSpacePoints[1][0].toFloat(), viewSpacePoints[1][1].toFloat(), viewSpacePoints[1][2].toFloat());

    glm::vec3 dir = p1 - p0;
    float len = glm::length(dir);
    if (len < 0.001f) return;
    dir /= len;

    glm::vec3 viewDir(0.0f, 0.0f, -1.0f);
    glm::vec3 perp = glm::cross(dir, viewDir);
    float perpLen = glm::length(perp);
    if (perpLen < 0.0001f) {
        perp = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));
        perpLen = glm::length(perp);
        if (perpLen < 0.0001f) return;
    }
    perp /= perpLen;

    float halfWidth0 = widths[0].m_value / (float)0x10000;
    float halfWidth1 = widths[1].m_value / (float)0x10000;

    glm::vec3 v0 = p0 - perp * halfWidth0;
    glm::vec3 v1 = p1 - perp * halfWidth1;
    glm::vec3 v2 = p1 + perp * halfWidth1;
    glm::vec3 v3 = p0 + perp * halfWidth0;

    sRayQuad3D quad;
    quad.vertices[0] = { {v0.x, v0.y, v0.z}, {0,0} };
    quad.vertices[1] = { {v1.x, v1.y, v1.z}, {1,0} };
    quad.vertices[2] = { {v2.x, v2.y, v2.z}, {1,1} };
    quad.vertices[3] = { {v3.x, v3.y, v3.z}, {0,1} };

    computeAverageGouraud(pQuadColor, quad.gouraud);

    quad.CMDCTRL = 0x1002;
    quad.CMDPMOD = 0x484 | colorMode;
    quad.CMDCOLR = characterColor;
    quad.CMDSRCA = characterAddress;
    quad.CMDSIZE = characterSize;

    gPendingRayQuads.push_back(quad);
}

void displayRaySegment_2Width(std::array<sVec3_FP, 2>& param_1, std::array<fixedPoint, 2>& param_2, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    std::array<sVec3_FP, 2> transformedPoints;
    transformAndAddVecByCurrentMatrix(&param_1[0], &transformedPoints[0]);
    transformAndAddVecByCurrentMatrix(&param_1[1], &transformedPoints[1]);

    if (gDirectRayRendering)
    {
        enqueueRaySegment3D_2Width(transformedPoints, param_2, characterAddress, characterSize, characterColor, pQuadColor, colorMode);
        return;
    }

    sScreenQuad3 screenQuad;
    if (rayComputeDisplayMatrix_2Width(transformedPoints, param_2, graphicEngineStatus.m405C, screenQuad))
    {
        sendRaySegmentToVdp1(screenQuad, transformedPoints[1][2], characterAddress, characterSize, characterColor, pQuadColor, colorMode);
    }
}

void displayRaySegmentFromViewSpace(std::array<sVec3_FP, 2>& viewSpacePoints, s32 width, u16 characterAddress, s16 characterSize, u16 characterColor, const quadColor* pQuadColor, s32 colorMode)
{
    if (gDirectRayRendering)
    {
        enqueueRaySegment3D(viewSpacePoints, width, characterAddress, characterSize, characterColor, pQuadColor, colorMode);
        return;
    }

    sScreenQuad3 screenQuad;
    if (rayComputeDisplayMatrix_fixedWidth(viewSpacePoints, width, graphicEngineStatus.m405C, screenQuad))
    {
        sendRaySegmentToVdp1(screenQuad, viewSpacePoints[1][2], characterAddress, characterSize, characterColor, pQuadColor, colorMode);
    }
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

    static bgfx::UniformHandle u_gouraudTint = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(u_gouraudTint))
    {
        u_gouraudTint = bgfx::createUniform("u_gouraudTint", bgfx::UniformType::Vec4);
    }

    bgfx::VertexLayout layout;
    layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
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

        bgfx::setUniform(u_gouraudTint, rayQuad.gouraud);

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

// ---------------------------------------------------------------------------
// Colored 3D lines (per-pixel depth) used by shared VDP1 line submitters.
// ---------------------------------------------------------------------------

struct sRayLine3D
{
    float p0[3];     // view-space endpoint 0
    float c0[4];     // RGBA at endpoint 0
    float p1[3];     // view-space endpoint 1
    float c1[4];     // RGBA at endpoint 1
};

static std::vector<sRayLine3D> gPendingRayLines;

// Extract the direct-colour RGB555 base out of CMDCOLR when bit 15 is set.
// Palette-indexed CMDCOLR (bit 15 clear) falls back to white because the
// 3D path has no backing VDP1 CLUT to look up.
static void saturnDirectColorToRGB(u16 cmdcolr, float baseRGB[3])
{
    if (cmdcolr & 0x8000)
    {
        baseRGB[0] = (float)((cmdcolr >>  0) & 0x1F) / 31.0f;
        baseRGB[1] = (float)((cmdcolr >>  5) & 0x1F) / 31.0f;
        baseRGB[2] = (float)((cmdcolr >> 10) & 0x1F) / 31.0f;
    }
    else
    {
        baseRGB[0] = 1.0f;
        baseRGB[1] = 1.0f;
        baseRGB[2] = 1.0f;
    }
}

// Apply a Saturn VDP1 gouraud colour (RGB555: 1BBBBBGGGGGRRRRR, with each
// 5-bit channel representing a signed +/-16 offset around neutral = 16) as
// an additive offset on top of a direct-colour base. Output clamped to
// [0, 1] per channel so we never feed negative values through the shader.
static void applyGouraudToBase(const float baseRGB[3], u16 gouraud, float out[4])
{
    float rOffset = (float)((s32)((gouraud >>  0) & 0x1F) - 16) / 31.0f;
    float gOffset = (float)((s32)((gouraud >>  5) & 0x1F) - 16) / 31.0f;
    float bOffset = (float)((s32)((gouraud >> 10) & 0x1F) - 16) / 31.0f;

    float r = baseRGB[0] + rOffset;
    float g = baseRGB[1] + gOffset;
    float b = baseRGB[2] + bOffset;
    if (r < 0.0f) r = 0.0f; else if (r > 1.0f) r = 1.0f;
    if (g < 0.0f) g = 0.0f; else if (g > 1.0f) g = 1.0f;
    if (b < 0.0f) b = 0.0f; else if (b > 1.0f) b = 1.0f;

    out[0] = r;
    out[1] = g;
    out[2] = b;
    out[3] = 1.0f;
}

void enqueueRayLine3D(const sVec3_FP& viewSpaceP0, const sVec3_FP& viewSpaceP1,
                      u16 cmdcolr, u16 gouraud0, u16 gouraud1)
{
    float baseRGB[3];
    saturnDirectColorToRGB(cmdcolr, baseRGB);

    sRayLine3D line;
    line.p0[0] = viewSpaceP0.m0_X.toFloat();
    line.p0[1] = viewSpaceP0.m4_Y.toFloat();
    line.p0[2] = viewSpaceP0.m8_Z.toFloat();
    line.p1[0] = viewSpaceP1.m0_X.toFloat();
    line.p1[1] = viewSpaceP1.m4_Y.toFloat();
    line.p1[2] = viewSpaceP1.m8_Z.toFloat();
    applyGouraudToBase(baseRGB, gouraud0, line.c0);
    applyGouraudToBase(baseRGB, gouraud1, line.c1);
    gPendingRayLines.push_back(line);
}

void flushRayLines3D()
{
    if (gPendingRayLines.empty()) return;

    // Forward declarations from 3dEngine_flush.cpp — same pattern as
    // flushRayQuads3D.
    glm::mat4 getProjectionMatrix();
    bgfx::ProgramHandle GetWorldSpaceVertexColorShaderBGFX();

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

    bgfx::VertexLayout layout;
    layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Float)
        .end();

    struct LineVert
    {
        float pos[3];
        float tex[2];
        float color[4];
    };

    const u32 numLines = (u32)gPendingRayLines.size();
    const u32 numVerts = numLines * 2;

    if (bgfx::getAvailTransientVertexBuffer(numVerts, layout) < numVerts)
    {
        gPendingRayLines.clear();
        return;
    }

    bgfx::TransientVertexBuffer vertexBuffer;
    bgfx::allocTransientVertexBuffer(&vertexBuffer, numVerts, layout);

    LineVert* pOut = (LineVert*)vertexBuffer.data;
    for (u32 i = 0; i < numLines; i++)
    {
        const sRayLine3D& line = gPendingRayLines[i];

        pOut[0].pos[0] = line.p0[0];
        pOut[0].pos[1] = line.p0[1];
        pOut[0].pos[2] = line.p0[2];
        pOut[0].tex[0] = 0.f;
        pOut[0].tex[1] = 0.f;
        pOut[0].color[0] = line.c0[0];
        pOut[0].color[1] = line.c0[1];
        pOut[0].color[2] = line.c0[2];
        pOut[0].color[3] = line.c0[3];

        pOut[1].pos[0] = line.p1[0];
        pOut[1].pos[1] = line.p1[1];
        pOut[1].pos[2] = line.p1[2];
        pOut[1].tex[0] = 1.f;
        pOut[1].tex[1] = 0.f;
        pOut[1].color[0] = line.c1[0];
        pOut[1].color[1] = line.c1[1];
        pOut[1].color[2] = line.c1[2];
        pOut[1].color[3] = line.c1[3];

        pOut += 2;
    }

    glm::mat4 projectionMatrix = getProjectionMatrix();
    bgfx::setUniform(vdp1_modelViewProj, &projectionMatrix[0][0]);

    // Match the 3D engine's "regular object" priority bucket so these lines
    // are depth-sorted against the main 3D scene via the VDP1 GPU view.
    float priority[4] = { (float)(vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA & 7), 0, 0, 0 };
    bgfx::setUniform(u_spritePriority, priority);

    bgfx::setVertexBuffer(0, &vertexBuffer);
    bgfx::setState(0
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LEQUAL
        | BGFX_STATE_MSAA
        | BGFX_STATE_PT_LINES
    );

    bgfx::submit(vdp1_gpuView, GetWorldSpaceVertexColorShaderBGFX());

    gPendingRayLines.clear();
}

// 0602e136
void displayRaySegmentFromWorldSpace(sVec3_FP* pVerts,
                                     s32 width,
                                     u16 cmdsrca,
                                     u16 cmdsize,
                                     u16 cmdcolr,
                                     u16 colorMode)
{
    if (cmdsrca != 0)
    {
        // Real Saturn path: textured ray quad via the shared rayDisplay
        // infrastructure. The 6-arg Saturn original has no per-vertex
        // gouraud (CMDGRA is never written), so we hand in a neutral
        // gouraud quadColor to get the same visual result on both the
        // direct-3D and VDP1 branches of displayRaySegment.
        static const quadColor neutralGouraud = { 0x4210, 0x4210, 0x4210, 0x4210 };
        std::array<sVec3_FP, 2> line = { pVerts[0], pVerts[1] };
        displayRaySegment(line, width, cmdsrca, cmdsize, cmdcolr, &neutralGouraud, colorMode);
        return;
    }

    // --- Colour-only fallback (no sprite state available) -----------------

    // World-space -> view-space transform for the two endpoints.
    sVec3_FP viewA;
    sVec3_FP viewB;
    transformAndAddVecByCurrentMatrix(&pVerts[0], &viewA);
    transformAndAddVecByCurrentMatrix(&pVerts[1], &viewB);

    // Behind-camera rejection — if either vertex sits at or behind the near
    // plane, drop the whole segment rather than producing garbage coordinates.
    if (viewA.m8_Z.m_value <= 0 || viewB.m8_Z.m_value <= 0)
    {
        return;
    }

    if (gDirectRayRendering)
    {
        // Per-pixel-depth BGFX line primitive with solid cmdcolr at both
        // endpoints (neutral gouraud = no tint offset).
        enqueueRayLine3D(viewA, viewB, cmdcolr, 0x4210, 0x4210);
        return;
    }

    // Perspective divide, matches the pattern used by battleEnemyLifeMeter.
    fixedPoint invZA = FP_Div(0x10000, viewA.m8_Z);
    fixedPoint invZB = FP_Div(0x10000, viewB.m8_Z);
    fixedPoint projAX = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale,  viewA.m0_X, invZA);
    fixedPoint projAY = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, viewA.m4_Y, invZA);
    fixedPoint projBX = MTH_Mul_5_6(graphicEngineStatus.m405C.m18_widthScale,  viewB.m0_X, invZB);
    fixedPoint projBY = MTH_Mul_5_6(graphicEngineStatus.m405C.m1C_heightScale, viewB.m4_Y, invZB);

    // Average view-space Z for the bucket-sort key.
    fixedPoint avgZ(
        (viewA.m8_Z.m_value + viewB.m8_Z.m_value) / 2);
    fixedPoint depth = avgZ * graphicEngineStatus.m405C.m38_oneOverFarClip;

    s_vdp1Context& ctx = graphicEngineStatus.m14_vdp1Context[0];
    s_vdp1Command& cmd = *ctx.m0_currentVdp1WriteEA;
    cmd.m0_CMDCTRL = 0x1006; // distorted line
    cmd.m4_CMDPMOD = (u16)(colorMode | 0x480); // match FUN_0602d9bc: colorMode | 0x480
    cmd.m6_CMDCOLR = cmdcolr;
    cmd.mC_CMDXA  =  (s16)projAX.getInteger();
    cmd.mE_CMDYA  = -(s16)projAY.getInteger();
    cmd.m10_CMDXB =  (s16)projBX.getInteger();
    cmd.m12_CMDYB = -(s16)projBY.getInteger();
    cmd.m14_CMDXC =  (s16)projBX.getInteger();
    cmd.m16_CMDYC = -(s16)projBY.getInteger();
    cmd.m18_CMDXD =  (s16)projAX.getInteger();
    cmd.m1A_CMDYD = -(s16)projAY.getInteger();
    cmd.m1C_CMDGRA = 0;

    ctx.m20_pCurrentVdp1Packet->m4_bucketTypes = depth.getInteger();
    ctx.m20_pCurrentVdp1Packet->m6_vdp1EA = &cmd;
    ctx.m20_pCurrentVdp1Packet++;
    ctx.m0_currentVdp1WriteEA++;
    ctx.m1C += 1;
    ctx.mC  += 1;
}
