#include "PDS.h"
#include "renderer/renderer_gl.h"
#include "processModel.h"
#include "debugWindows.h"

#include <bx/math.h>

extern std::array<sMatrix4x3, 16> matrixStack;

#include "3dEngine_textureCache.h"
#include <unordered_map>

#if !defined(USE_NULL_RENDERER)

const char* gGLSLVersion = nullptr;

extern bgfx::UniformHandle u_spritePriority;

static void setSpritePriorityUniform(u16 cmdcolr, u16 cmdpmod)
{
    float priority[4] = { (float)computeSpritePriority(cmdcolr, cmdpmod), 0, 0, 0 };
    bgfx::setUniform(u_spritePriority, priority);
}

static void setSpritePriorityDefault()
{
    // 3D engine objects use priority register 0 (PRISA low)
    float priority[4] = { (float)(vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA & 7), 0, 0, 0 };
    bgfx::setUniform(u_spritePriority, priority);
}

void drawLineGL(sVec3_FP vertice1, sVec3_FP vertice2, sFColor color = { 1,0,0,1 });

struct sDebugLines
{
    sVec3_FP position1;
    sVec3_FP position2;
    sFColor m_color;
};

std::vector<sDebugLines> debugLines;

struct sDebugQuad
{
    std::array<sVec3_FP, 4> m_vertices;
    sFColor m_color;
};

std::vector<sDebugQuad> debugQuads;
void drawQuadGL(const sDebugQuad& quad);

void drawDebugFilledQuad(const sVec3_FP& position0, const sVec3_FP& position1, const sVec3_FP& position2, const sVec3_FP& position3, const sFColor& color)
{
    sDebugQuad newDebugQuad;
    newDebugQuad.m_vertices[0] = position0;
    newDebugQuad.m_vertices[1] = position1;
    newDebugQuad.m_vertices[2] = position2;
    newDebugQuad.m_vertices[3] = position3;
    newDebugQuad.m_color = color;

    debugQuads.push_back(newDebugQuad);
}

void drawDebugLine(const sVec3_FP& position1, const sVec3_FP& position2, const sFColor& color)
{
    sDebugLines newDebugLine;
    newDebugLine.position1 = position1;
    newDebugLine.position2 = position2;
    newDebugLine.m_color = color;

    debugLines.push_back(newDebugLine);

}

void drawDebugArrow(const sVec3_FP& position, const sVec3_FP& normal, const fixedPoint& magnitude)
{
    sVec3_FP position2;
    position2[0] = MTH_Mul(normal[0], magnitude) + position[0];
    position2[1] = MTH_Mul(normal[0], magnitude) + position[1];
    position2[2] = MTH_Mul(normal[0], magnitude) + position[2];

    sDebugLines newDebugLine;
    newDebugLine.position1 = position;
    newDebugLine.position2 = position2;
    newDebugLine.m_color = { 1,0,0,1 };

    debugLines.push_back(newDebugLine);
}

void transposeMatrix(float* pMatrix)
{
    float temp[4 * 4];
    memcpy(temp, pMatrix, sizeof(float) * 16);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            pMatrix[4 * i + j] = temp[4 * j + i];
        }
    }
}

struct s_objectToRender
{
    sProcessed3dModel* m_pObject;
    sMatrix4x3 m_modelMatrix;
    s16 m_lightColor[3];
    s32 m_lightVector[3]; // light direction in world space (captured at submit time)
    float m_2dOffset[2];
    bool m_isBillboard;
};

std::vector<s_objectToRender> objectRenderList;

void addObjectToDrawList(sProcessed3dModel* pObjectData)
{
    if (pObjectData == nullptr) {
        PDS_warningOnce("[ERROR] trying to render a null entity!");
        return;
    }

    s_objectToRender newObject;
    newObject.m_pObject = pObjectData;
    newObject.m_modelMatrix = *pCurrentMatrix;
    newObject.m_lightVector[0] = (s32)currentLightVector_M.m_lightVector[0];
    newObject.m_lightVector[1] = (s32)currentLightVector_M.m_lightVector[1];
    newObject.m_lightVector[2] = (s32)currentLightVector_M.m_lightVector[2];
    newObject.m_lightColor[0] = currentLightVector_M.m_color[0];
    newObject.m_lightColor[1] = currentLightVector_M.m_color[1];
    newObject.m_lightColor[2] = currentLightVector_M.m_color[2];
    newObject.m_2dOffset[0] = (graphicEngineStatus.m405C.m44_localCoordinatesX - (352.f / 2.f)) / 352.f;
    newObject.m_2dOffset[1] = (graphicEngineStatus.m405C.m46_localCoordinatesY - (224.f / 2.f)) / 224.f;
    newObject.m_isBillboard = 0;

    objectRenderList.push_back(newObject);
}

struct sKey
{
    u8* pObjectData;
    u32 offset;

    bool operator==(const sKey &other) const
    {
        return (pObjectData == other.pObjectData
            && offset == other.offset);
    }
};

struct KeyHasher
{
    std::size_t operator()(const sKey& k) const
    {
        return (std::size_t)(k.pObjectData + k.offset);
    }
};


static std::unordered_map<sKey, sProcessed3dModel*, KeyHasher> modelMap;

void addBillBoardToDrawList(sProcessed3dModel* pObjectData)
{
    cameraProperties2.m88_billboardViewMatrix.m[0][3] = pCurrentMatrix->m[0][3];
    cameraProperties2.m88_billboardViewMatrix.m[1][3] = pCurrentMatrix->m[1][3];
    cameraProperties2.m88_billboardViewMatrix.m[2][3] = pCurrentMatrix->m[2][3];

    s_objectToRender newObject;
    newObject.m_pObject = pObjectData;
    newObject.m_modelMatrix = cameraProperties2.m88_billboardViewMatrix;
    newObject.m_lightVector[0] = (s32)currentLightVector_M.m_lightVector[0];
    newObject.m_lightVector[1] = (s32)currentLightVector_M.m_lightVector[1];
    newObject.m_lightVector[2] = (s32)currentLightVector_M.m_lightVector[2];
    newObject.m_lightColor[0] = currentLightVector_M.m_color[0];
    newObject.m_lightColor[1] = currentLightVector_M.m_color[1];
    newObject.m_lightColor[2] = currentLightVector_M.m_color[2];
    newObject.m_2dOffset[0] = (graphicEngineStatus.m405C.m44_localCoordinatesX - (352.f / 2.f)) / 352.f;
    newObject.m_2dOffset[1] = (graphicEngineStatus.m405C.m46_localCoordinatesY - (224.f / 2.f)) / 224.f;
    newObject.m_isBillboard = 1;

    objectRenderList.push_back(newObject);
}

void multiplyMatrices(float* in1, float* in2, float* out)
{
    float temp[4 * 4];
    memset(temp, 0, sizeof(float) * 4 * 4);
#define	INDEX(r,c)				(((c-1)*4)+(r-1))
    for (int i = 01; i <= 4; i++)
        for (int j = 1; j <= 4; j++)
            for (int k = 1; k <= 4; k++)
                temp[INDEX(i, j)] += in1[INDEX(i, k)] * in2[INDEX(k, j)];

    memcpy(out, temp, sizeof(float) * 4 * 4);
}

bool fillPolys = true;
bool enableTextures = true;
bool gSmoothGouraud = true; // per-vertex depth + interpolated falloff curve

// Compute distance-based falloff color from the lightFalloffMap table
// viewDepth: view-space Z of the vertex (raw fixed-point from model-view matrix)
// Saturn: scaledDepth = MACH(viewZ * m34_oneOverFarClip256), index from scaledDepth
void GetDistanceFalloff(s32 falloutColor[3], s32 viewDepth)
{
    // Match Saturn: scaledDepth = (viewZ * oneOverFarClip256) >> 32
    s32 depthScale = (s32)graphicEngineStatus.m405C.m34_oneOverFarClip256;
    s32 scaledDepth = (s32)(((s64)viewDepth * (s64)depthScale) >> 32);
    if (scaledDepth < 0) scaledDepth = 0;

    // Saturn: shll r1 (<<1), shlr8 r1 (>>8), and #-8 (&~7) → byte offset, /8 → index
    int byteOffset = ((scaledDepth << 1) >> 8) & ~7;
    int index = byteOffset >> 3;
    if (index > 31) index = 31;

    falloutColor[0] = (s32)lightFalloffMap[index][0];
    falloutColor[1] = (s32)lightFalloffMap[index][1];
    falloutColor[2] = (s32)lightFalloffMap[index][2];
}

// Smooth variant: fractional index with linear interpolation between table entries
void GetDistanceFalloffSmooth(float falloutColor[3], s32 viewDepth)
{
    s32 depthScale = (s32)graphicEngineStatus.m405C.m34_oneOverFarClip256;
    s32 scaledDepth = (s32)(((s64)viewDepth * (s64)depthScale) >> 32);
    if (scaledDepth < 0) scaledDepth = 0;

    // Compute fractional index: same formula as Saturn but keep the fraction
    float fIndex = (float)((scaledDepth << 1) >> 8) / 8.0f;
    if (fIndex < 0.0f) fIndex = 0.0f;
    if (fIndex > 31.0f) fIndex = 31.0f;

    int idx0 = (int)fIndex;
    int idx1 = idx0 + 1;
    if (idx1 > 31) idx1 = 31;
    float frac = fIndex - (float)idx0;

    for (int c = 0; c < 3; c++)
    {
        falloutColor[c] = (float)lightFalloffMap[idx0][c] * (1.0f - frac)
                        + (float)lightFalloffMap[idx1][c] * frac;
    }
}

// Matches masterComputeLight3 / masterComputeLightDistanceFalloffAndLight3 from Ghidra
// Mode 3 (no color): accum = falloff + lightColor * max(dotProduct, 0)
// Mode 2 (withColor): accum = falloff + fixedColor + lightColor * max(dotProduct, 0)
// perVertexColor[3]: output in [-0.5, 0.5] for shader gouraud modulation
void ComputeColorFromNormal(const sProcessed3dModel::sQuadExtra& extraData, bool withColor,
    s32* lightVectorModelSpace, s16* lightColor, s32* falloutColor, float* perVertexColor)
{
    s32 dotProduct = 0;
    dotProduct += (s32)extraData.m0_normals[0] * lightVectorModelSpace[0];
    dotProduct += (s32)extraData.m0_normals[1] * lightVectorModelSpace[1];
    dotProduct += (s32)extraData.m0_normals[2] * lightVectorModelSpace[2];

    s32 accum[3];
    accum[0] = falloutColor[0];
    accum[1] = falloutColor[1];
    accum[2] = falloutColor[2];

    if (withColor)
    {
        accum[0] += (s16)extraData.m6_colors[0];
        accum[1] += (s16)extraData.m6_colors[1];
        accum[2] += (s16)extraData.m6_colors[2];
    }

    if (dotProduct > 0)
    {
        s16 dotHi = (s16)((u32)dotProduct >> 16);
        accum[0] += (s32)lightColor[0] * (s32)dotHi;
        accum[1] += (s32)lightColor[1] * (s32)dotHi;
        accum[2] += (s32)lightColor[2] * (s32)dotHi;
    }

    for (int i = 0; i < 3; i++)
    {
        if (accum[i] < 0) accum[i] = 0;
        if (accum[i] > 0x1F00) accum[i] = 0x1F00;
        int gouraud5bit = (accum[i] >> 8) & 0x1F;
        perVertexColor[i] = ((float)gouraud5bit - 16.0f) / 31.0f;
    }
}

// Smooth variant: float falloff input, no 5-bit quantization
void ComputeColorFromNormalSmooth(const sProcessed3dModel::sQuadExtra& extraData, bool withColor,
    s32* lightVectorModelSpace, s16* lightColor, float* falloutColor, float* perVertexColor)
{
    s32 dotProduct = 0;
    dotProduct += (s32)extraData.m0_normals[0] * lightVectorModelSpace[0];
    dotProduct += (s32)extraData.m0_normals[1] * lightVectorModelSpace[1];
    dotProduct += (s32)extraData.m0_normals[2] * lightVectorModelSpace[2];

    float accum[3];
    accum[0] = falloutColor[0];
    accum[1] = falloutColor[1];
    accum[2] = falloutColor[2];

    if (withColor)
    {
        accum[0] += (float)(s16)extraData.m6_colors[0];
        accum[1] += (float)(s16)extraData.m6_colors[1];
        accum[2] += (float)(s16)extraData.m6_colors[2];
    }

    if (dotProduct > 0)
    {
        float dotHi = (float)((u32)dotProduct >> 16);
        accum[0] += (float)lightColor[0] * dotHi;
        accum[1] += (float)lightColor[1] * dotHi;
        accum[2] += (float)lightColor[2] * dotHi;
    }

    for (int i = 0; i < 3; i++)
    {
        if (accum[i] < 0.0f) accum[i] = 0.0f;
        if (accum[i] > 7936.0f) accum[i] = 7936.0f; // 0x1F00
        // Smooth remap: no 5-bit quantization, continuous [-16/31, +15/31]
        perVertexColor[i] = (accum[i] / 256.0f - 16.0f) / 31.0f;
    }
}

bool enableVertexColor = true;

struct s_vertexData
{
    float positions[3];
    float textures[2];
    float color[4];
};

std::vector<s_vertexData> gVertexArray;

float textureInfluence = 0;

std::vector<s_vertexData> SingleDrawcallVertexArray;
std::vector<uint32_t> SingleDrawcallIndexArray;

bgfx::ProgramHandle loadBgfxProgram(const std::string& VSFile, const std::string& PSFile); // TODO: clean

glm::mat4 MatrixToGLM(const sMatrix4x3& inputMatrix)
{
    float quantisation = (float)0x10000;
    float objectMatrix[4 * 4];
    {

        memset(objectMatrix, 0, sizeof(objectMatrix));
        for (u32 row = 0; row < 3; row++)
            for (u32 col = 0; col < 4; col++)
                objectMatrix[row * 4 + col] = inputMatrix.m[row][col] / quantisation;
        objectMatrix[15] = 1.f;
    }

    glm::mat4 tempObjectMatrix;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tempObjectMatrix[i][j] = objectMatrix[j * 4 + i];
        }
    }

    return tempObjectMatrix;
}

void drawObject(s_objectToRender* pObject, const glm::mat4& projectionMatrix)
{
    pObject->m_pObject->generateVertexBuffer();

    static bgfx::ProgramHandle vdp1_program = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_modelViewProj = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_invModelViewProj = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_textureSampler = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_ndcCornersSampler = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_objCornersSampler = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_cornersParams = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(vdp1_program))
    {
        vdp1_program = loadBgfxProgram("VDP1_vs", "VDP1_ps");
        vdp1_modelViewProj = bgfx::createUniform("u_customModelViewProj", bgfx::UniformType::Mat4);
        vdp1_invModelViewProj = bgfx::createUniform("u_invModelViewProj", bgfx::UniformType::Mat4);
        vdp1_textureSampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
        vdp1_ndcCornersSampler = bgfx::createUniform("s_ndcCorners", bgfx::UniformType::Sampler);
        vdp1_objCornersSampler = bgfx::createUniform("s_objCorners", bgfx::UniformType::Sampler);
        vdp1_cornersParams = bgfx::createUniform("u_quadCornersParams", bgfx::UniformType::Vec4);
    }

    sProcessed3dModel* model = pObject->m_pObject;
    glm::mat4 mvpMatrix = projectionMatrix * MatrixToGLM(pObject->m_modelMatrix);
    glm::mat4 invMvpMatrix = glm::inverse(mvpMatrix);

    bgfx::setUniform(vdp1_modelViewProj, &mvpMatrix[0][0]);
    bgfx::setUniform(vdp1_invModelViewProj, &invMvpMatrix[0][0]);

    // Build per-frame texture with NDC corners + gouraud shading
    // Layout: 8 texels per quad
    //   texel 0-3: (ndcX, ndcY, u/w, v/w) per corner
    //   texel 4:   (1/w0, 1/w1, 1/w2, 1/w3) — all zeros = near-clip sentinel
    //   texel 5:   (gR0, gR1, gR2, gR3) — per-corner gouraud red [-0.5, 0.5]
    //   texel 6:   (gG0, gG1, gG2, gG3) — per-corner gouraud green
    //   texel 7:   (gB0, gB1, gB2, gB3) — per-corner gouraud blue
    size_t numQuads = model->mC_Quads.size();
    uint16_t texWidth = (uint16_t)(numQuads * 8);

    static std::vector<float> cornersData;
    cornersData.resize(texWidth * 4);

    const float nearClipThreshold = 0.01f;

    // Transform light vector into model space for gouraud computation
    // The model matrix transforms from model space to world space,
    // so we need to inverse-rotate the light vector into model space
    s32 lightVectorModelSpace[3];
    {
        // The model matrix upper 3x3 is the rotation. For an orthonormal rotation,
        // inverse = transpose. Light vector is already in camera/world space.
        const sMatrix4x3& modelMat = pObject->m_modelMatrix;
        for (int axis = 0; axis < 3; axis++)
        {
            // Transpose multiply: column 'axis' of modelMat dot lightVector
            s64 acc = 0;
            acc += (s64)modelMat.m[0][axis] * pObject->m_lightVector[0];
            acc += (s64)modelMat.m[1][axis] * pObject->m_lightVector[1];
            acc += (s64)modelMat.m[2][axis] * pObject->m_lightVector[2];
            lightVectorModelSpace[axis] = (s32)(acc >> 16);
        }
    }

    for (size_t i = 0; i < numQuads; i++)
    {
        float clipW[4];
        bool nearClip = false;

        for (int j = 0; j < 4; j++)
        {
            const auto& vtx = model->m_vertexBuffer[i * 4 + j];
            glm::vec4 objPos(vtx.position[0], vtx.position[1], vtx.position[2], 1.0f);
            glm::vec4 clipPos = mvpMatrix * objPos;
            clipW[j] = clipPos.w;
            if (clipPos.w < nearClipThreshold)
                nearClip = true;
        }

        if (!nearClip)
        {
            for (int j = 0; j < 4; j++)
            {
                const auto& vtx = model->m_vertexBuffer[i * 4 + j];
                float invW = 1.0f / clipW[j];
                glm::vec4 objPos(vtx.position[0], vtx.position[1], vtx.position[2], 1.0f);
                glm::vec4 clipPos = mvpMatrix * objPos;

                size_t idx = (i * 8 + j) * 4;
                cornersData[idx + 0] = clipPos.x * invW;
                cornersData[idx + 1] = clipPos.y * invW;
                cornersData[idx + 2] = vtx.texcoord0[0] * invW;
                cornersData[idx + 3] = vtx.texcoord0[1] * invW;
            }
            size_t idx = (i * 8 + 4) * 4;
            cornersData[idx + 0] = 1.0f / clipW[0];
            cornersData[idx + 1] = 1.0f / clipW[1];
            cornersData[idx + 2] = 1.0f / clipW[2];
            cornersData[idx + 3] = 1.0f / clipW[3];
        }
        else
        {
            for (int j = 0; j < 5; j++)
            {
                size_t idx = (i * 8 + j) * 4;
                cornersData[idx + 0] = 0.0f;
                cornersData[idx + 1] = 0.0f;
                cornersData[idx + 2] = 0.0f;
                cornersData[idx + 3] = 0.0f;
            }
        }

        // Compute per-vertex gouraud colors
        const auto& quad = model->mC_Quads[i];
        u8 lightingMode = (quad.m8_lightingControl >> 8) & 3;
        float gouraud[4][3] = {}; // [corner][RGB], default 0 = no modulation

        if (lightingMode > 0 && quad.m14_extraData.size() > 0)
        {
            const sMatrix4x3& modelMat = pObject->m_modelMatrix;
            bool withColor = (lightingMode == 2);

            // Helper: compute view-space depth for a vertex
            auto computeViewDepth = [&](int vertexIndex) -> s32 {
                const auto& vtx = model->m_vertexBuffer[i * 4 + vertexIndex];
                s32 viewZ = (s32)(
                    ((s64)modelMat.m[2][0] * (s32)(vtx.position[0] * 65536.0f) +
                     (s64)modelMat.m[2][1] * (s32)(vtx.position[1] * 65536.0f) +
                     (s64)modelMat.m[2][2] * (s32)(vtx.position[2] * 65536.0f)) >> 16)
                    + (s32)modelMat.m[2][3];
                s32 absDepth = viewZ < 0 ? -viewZ : viewZ;
                return absDepth << 8;
            };

            if (gSmoothGouraud)
            {
                // Enhanced: per-vertex depth + interpolated falloff + no quantization
                int numNormals = (lightingMode == 1) ? 1 : 4;
                for (int j = 0; j < 4; j++)
                {
                    int normalIdx = (lightingMode == 1) ? 0 : j;
                    if (normalIdx >= (int)quad.m14_extraData.size()) continue;

                    float falloutSmooth[3];
                    GetDistanceFalloffSmooth(falloutSmooth, computeViewDepth(j));
                    ComputeColorFromNormalSmooth(quad.m14_extraData[normalIdx], withColor,
                        lightVectorModelSpace, pObject->m_lightColor,
                        falloutSmooth, gouraud[j]);
                }
            }
            else
            {
                // Saturn-accurate: per-quad depth, quantized 5-bit gouraud
                s32 falloutColor[3];
                GetDistanceFalloff(falloutColor, computeViewDepth(0));

                if (lightingMode == 1)
                {
                    float perVertexColor[3];
                    ComputeColorFromNormal(quad.m14_extraData[0], false,
                        lightVectorModelSpace, pObject->m_lightColor,
                        falloutColor, perVertexColor);
                    for (int j = 0; j < 4; j++)
                        for (int c = 0; c < 3; c++)
                            gouraud[j][c] = perVertexColor[c];
                }
                else
                {
                    for (int j = 0; j < 4; j++)
                    {
                        if (j < (int)quad.m14_extraData.size())
                        {
                            ComputeColorFromNormal(quad.m14_extraData[j], withColor,
                                lightVectorModelSpace, pObject->m_lightColor,
                                falloutColor, gouraud[j]);
                        }
                    }
                }
            }
        }

        // Pack gouraud: texel 5 = R, texel 6 = G, texel 7 = B
        for (int c = 0; c < 3; c++)
        {
            size_t idx = (i * 8 + 5 + c) * 4;
            cornersData[idx + 0] = gouraud[0][c];
            cornersData[idx + 1] = gouraud[1][c];
            cornersData[idx + 2] = gouraud[2][c];
            cornersData[idx + 3] = gouraud[3][c];
        }
    }

    bgfx::TextureHandle ndcCornersTex = bgfx::createTexture2D(
        texWidth, 1, false, 1,
        bgfx::TextureFormat::RGBA32F,
        BGFX_SAMPLER_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
        bgfx::copy(cornersData.data(), (uint32_t)(texWidth * 4 * sizeof(float))));

    float params[4] = { (float)texWidth, (float)(numQuads * 5), 0.0f, 0.0f };
    bgfx::setUniform(vdp1_cornersParams, params);

    uint64_t state = 0
        | BGFX_STATE_DEPTH_TEST_LEQUAL
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO)
        ;

    bgfx::setState(state);

    bgfx::setTexture(0, vdp1_textureSampler, model->m_textureAtlas);
    bgfx::setTexture(1, vdp1_ndcCornersSampler, ndcCornersTex);
    bgfx::setTexture(2, vdp1_objCornersSampler, model->m_quadCornersTexture);
    bgfx::setVertexBuffer(0, model->m_vertexBufferHandle);
    bgfx::setIndexBuffer(model->m_indexBufferHandle);
    setSpritePriorityDefault();
    bgfx::submit(vdp1_gpuView, vdp1_program);

    bgfx::destroy(ndcCornersTex);
}

glm::mat4 getViewMatrix()
{
    return MatrixToGLM(matrixStack[0]);
}

float fov = 80.f;
void RendererSetFov(float fovInDegree)
{
    fov = fovInDegree;
}

glm::mat4 getProjectionMatrix()
{
    static float fEarlyProjectionMatrix[4 * 4];

    float zNear = 0.01f;
    float zFar = 1000.f;

#if 0
    s32 const1 = graphicEngineStatus.m405C.m18_widthScale; //229;
    s32 const2 = graphicEngineStatus.m405C.m1C_heightScale; //195;

    fEarlyProjectionMatrix[0] = const1 / (352.f / 2.f);
    fEarlyProjectionMatrix[1] = 0;
    fEarlyProjectionMatrix[2] = 0;
    fEarlyProjectionMatrix[3] = 0;

    fEarlyProjectionMatrix[4] = 0;
    fEarlyProjectionMatrix[5] = const2 / (224.f / 2.f);
    fEarlyProjectionMatrix[6] = 0;
    fEarlyProjectionMatrix[7] = 0;

    fEarlyProjectionMatrix[8] = 0;
    fEarlyProjectionMatrix[9] = 0;
    fEarlyProjectionMatrix[10] = (-zNear - zFar) / (zFar - zNear);
    fEarlyProjectionMatrix[11] = 2.f * zFar * zNear / (zFar - zNear);

    fEarlyProjectionMatrix[12] = 0;
    fEarlyProjectionMatrix[13] = 0;
    fEarlyProjectionMatrix[14] = 1.f;
    fEarlyProjectionMatrix[15] = 0;

    //transposeMatrix(fEarlyProjectionMatrix);

    glm::mat4 testProj;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            testProj[i][j] = fEarlyProjectionMatrix[i * 4 + j];
        }
    }
#else

    static float currentFov = 59.f;
    glm::mat4 testProj = glm::perspectiveFov(glm::radians(currentFov), 4.f, 3.f, zNear, zFar);

    testProj[0][0] = graphicEngineStatus.m405C.m18_widthScale / (352.f / 2.f);
    testProj[1][1] = graphicEngineStatus.m405C.m1C_heightScale / (224.f / 2.f);
    testProj[2][2] = (zFar + zNear) / (zFar - zNear);
    testProj[2][3] = 1.f;
    testProj[3][2] = -2 * (zFar * zNear) / (zFar - zNear);

#endif
    return testProj;
}

void flushObjectsToDrawList()
{
    ZoneScopedN("flushObjectsToDrawList");

    gVertexArray.resize(1024 * 1024);


    if(!isShipping() && gDebugWindows.objects)
    {
        if (ImGui::Begin("Objects", &gDebugWindows.objects))
        {
            ImGui::Checkbox("Smooth Gouraud (per-vertex depth + interpolated falloff)", &gSmoothGouraud);
            ImGui::Separator();
            for (int i = 0; i < objectRenderList.size(); i++)
            {
                char buffer[256];
                sprintf(buffer, "Object %i", i);
                ImGui::PushID(buffer);
                {
                    for (int j = 0; j < 3; j++)
                    {
                        float vertex[4];
                        vertex[0] = objectRenderList[i].m_modelMatrix.m[j][0] / (float)0x10000;
                        vertex[1] = objectRenderList[i].m_modelMatrix.m[j][1] / (float)0x10000;
                        vertex[2] = objectRenderList[i].m_modelMatrix.m[j][2] / (float)0x10000;
                        vertex[3] = objectRenderList[i].m_modelMatrix.m[j][3] / (float)0x10000;

                        sprintf(buffer, "M%d", j);
                        ImGui::InputFloat4(buffer, vertex);
                    }
                }
                ImGui::PopID();
            }
        }
        ImGui::End();
    }

    bgfx::setMarker("Start object render list");
    for (int i = 0; i < objectRenderList.size(); i++)
    {
        SingleDrawcallVertexArray.clear();
        SingleDrawcallIndexArray.clear();

        char string[1024] = "";
        sprintf(string, "DrawObject %d", i);
        //glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, strlen(string), string);
        drawObject(&objectRenderList[i], getProjectionMatrix());
        //glPopDebugGroup();
    }
    bgfx::setMarker("Finish object render list");

    TracyPlot("ObjectRenderList size", (int64_t)objectRenderList.size());

    objectRenderList.clear();

    bgfx::setMarker("Start debug quads");
    for (int i = 0; i < debugQuads.size(); i++)
    {
        drawQuadGL(debugQuads[i]);
    }
    debugQuads.clear();
    bgfx::setMarker("Finish debug quads");

    bgfx::setMarker("Start debug lines");
    for(int i=0; i<debugLines.size(); i++)
    {
        drawLineGL(debugLines[i].position1, debugLines[i].position2, debugLines[i].m_color);
    }
    debugLines.clear();
    bgfx::setMarker("Finish debug lines");
}

float invf(int i, int j, const float* m) {

    int o = 2 + (j - i);

    i += 4 + o;
    j += 4 - o;

#define eMac(a,b) m[ ((j+(b))%4)*4 + ((i+(a))%4) ]

    float inv =
        +eMac(+1, -1)*eMac(+0, +0)*eMac(-1, +1)
        + eMac(+1, +1)*eMac(+0, -1)*eMac(-1, +0)
        + eMac(-1, -1)*eMac(+1, +0)*eMac(+0, +1)
        - eMac(-1, -1)*eMac(+0, +0)*eMac(+1, +1)
        - eMac(-1, +1)*eMac(+0, -1)*eMac(+1, +0)
        - eMac(+1, -1)*eMac(-1, +0)*eMac(+0, +1);

    return (o % 2) ? inv : -inv;

#undef eMac

}

bool inverseMatrix4x4(const float *m, float *out)
{

    float inv[16];

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            inv[j * 4 + i] = invf(i, j, m);

    double D = 0;

    for (int k = 0; k < 4; k++) D += m[k] * inv[k * 4];

    if (D == 0) return false;

    D = 1.0 / D;

    for (int i = 0; i < 16; i++)
        out[i] = inv[i] * D;

    return true;

}

void tranformVec4ByfMatrix(float* inOutVec, const float* matrix)
{
    float temp[4];

    temp[0] = ((inOutVec[0] * matrix[0]) + (inOutVec[1] * matrix[4]) + (inOutVec[2] * matrix[7]) + (inOutVec[3] * matrix[10]));
    temp[1] = ((inOutVec[0] * matrix[1]) + (inOutVec[1] * matrix[5]) + (inOutVec[2] * matrix[8]) + (inOutVec[3] * matrix[11]));
    temp[2] = ((inOutVec[0] * matrix[2]) + (inOutVec[1] * matrix[6]) + (inOutVec[2] * matrix[9]) + (inOutVec[3] * matrix[12]));

    inOutVec[0] = temp[0];
    inOutVec[1] = temp[1];
    inOutVec[2] = temp[2];
}

extern s16 localCoordiantesX;
extern s16 localCoordiantesY;

bgfx::ProgramHandle Get2dUIShaderBGFX()
{
    static bgfx::ProgramHandle programHandle = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(programHandle))
    {
        programHandle = loadBgfxProgram("VDP1_2dUI_vs", "VDP1_2dUI_ps");
    }

    return programHandle;
}

bgfx::ProgramHandle Get2dUIVertexColorShaderBGFX()
{
    static bgfx::ProgramHandle programHandle = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(programHandle))
    {
        programHandle = loadBgfxProgram("VDP1_2dUIVertexColor_vs", "VDP1_2dUIVertexColor_ps");
    }

    return programHandle;
}

bgfx::ProgramHandle GetWorldSpaceVertexColorShaderBGFX()
{
    static bgfx::ProgramHandle programHandle = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(programHandle))
    {
        programHandle = loadBgfxProgram("VDP1_vertexColor_vs", "VDP1_vertexColor_ps");
    }

    return programHandle;
}

void NormalSpriteDrawGL(s_vdp1Command* vdp1EA)
{
    u16 CMDCTRL = vdp1EA->m0_CMDCTRL;
    u16 CMDPMOD = vdp1EA->m4_CMDPMOD;
    u16 CMDCOLR = vdp1EA->m6_CMDCOLR;
    u16 CMDSRCA = vdp1EA->m8_CMDSRCA;
    u16 CMDSIZE = vdp1EA->mA_CMDSIZE;
    s16 CMDXA = vdp1EA->mC_CMDXA;
    s16 CMDYA = vdp1EA->mE_CMDYA;
    u16 CMDGRDA = vdp1EA->m1C_CMDGRA;

    if (CMDSRCA)
    {
        int colorMode = (CMDPMOD >> 3) & 0x7;
        u32 characterAddress = ((u32)CMDSRCA) << 3;
        u32 colorBank = ((u32)CMDCOLR) << 1;
        s32 X = CMDXA + localCoordiantesX;
        s32 Y = CMDYA + localCoordiantesY;
        s32 Width = ((CMDSIZE >> 8) & 0x3F) * 8;
        s32 Height = CMDSIZE & 0xFF;

        float zNear = 0.1f;
        float zFar = 1000.f;

        s_quad tempQuad;
        tempQuad.model = NULL;
        tempQuad.CMDCTRL = CMDCTRL;
        tempQuad.CMDPMOD = CMDPMOD;
        tempQuad.CMDCOLR = CMDCOLR;
        tempQuad.CMDSRCA = CMDSRCA;
        tempQuad.CMDSIZE = CMDSIZE;

        float quadDepth = 0.9;

        s_vd1ExtendedCommand* pExtendedCommand = fetchVdp1ExtendedCommand(*vdp1EA);
        if (pExtendedCommand)
        {
            quadDepth = pExtendedCommand->depth;
        }

        tempQuad.m_vertices[0].m_originalVertices[0] = X;
        tempQuad.m_vertices[0].m_originalVertices[1] = Y;
        tempQuad.m_vertices[0].m_originalVertices[2] = quadDepth;

        tempQuad.m_vertices[1].m_originalVertices[0] = (X + Width);
        tempQuad.m_vertices[1].m_originalVertices[1] = Y;
        tempQuad.m_vertices[1].m_originalVertices[2] = quadDepth;

        tempQuad.m_vertices[2].m_originalVertices[0] = (X + Width);
        tempQuad.m_vertices[2].m_originalVertices[1] = (Y + Height);
        tempQuad.m_vertices[2].m_originalVertices[2] = quadDepth;

        tempQuad.m_vertices[3].m_originalVertices[0] = X;
        tempQuad.m_vertices[3].m_originalVertices[1] = (Y + Height);
        tempQuad.m_vertices[3].m_originalVertices[2] = quadDepth;

        if (1)
        {
            int flip = (tempQuad.CMDCTRL & 0x30) >> 4;

            float uv[4][2];
            u16 color[4];

            uv[0][0] = 0;
            uv[0][1] = 0;
            color[0] = 1;
            //color[0] = MappedMemoryReadWord(quads[i].gouraudPointer + 0 * 2);

            uv[1][0] = 1;
            uv[1][1] = 0;
            color[1] = 1;
            //color[1] = MappedMemoryReadWord(quads[i].gouraudPointer + 1 * 2);

            uv[2][0] = 1;
            uv[2][1] = 1;
            color[2] = 1;
            //color[2] = MappedMemoryReadWord(quads[i].gouraudPointer + 2 * 2);

            uv[3][0] = 0;
            uv[3][1] = 1;
            color[3] = 1;
            //color[3] = MappedMemoryReadWord(quads[i].gouraudPointer + 3 * 2);

            {
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
                    gVertexArray[j].textures[0] = uv[vertexOrder[j]][0];
                    gVertexArray[j].textures[1] = uv[vertexOrder[j]][1];

                    if (enableVertexColor)
                    {
                        //glColor4f(perVertexColor[j][2], perVertexColor[j][1], perVertexColor[j][0], 1);

                        gVertexArray[j].color[0] = 1;
                        gVertexArray[j].color[1] = 1;
                        gVertexArray[j].color[2] = 1;
                        gVertexArray[j].color[3] = 1;

                    }
                    else
                    {
                        //glColor4f(1, 1, 1, 1);

                        gVertexArray[j].color[0] = 1;
                        gVertexArray[j].color[1] = 1;
                        gVertexArray[j].color[2] = 1;
                        gVertexArray[j].color[3] = 1;
                    }

                    gVertexArray[j].positions[0] = tempQuad.m_vertices[j].m_originalVertices[0];
                    gVertexArray[j].positions[1] = tempQuad.m_vertices[j].m_originalVertices[1];
                    gVertexArray[j].positions[2] = tempQuad.m_vertices[j].m_originalVertices[2];
                }

                {
                    bgfx::VertexLayout layout;
                    layout
                        .begin()
                        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
                        .end();

                    bgfx::TransientVertexBuffer vertexBuffer;
                    bgfx::TransientIndexBuffer indexBuffer;
                    bgfx::allocTransientBuffers(&vertexBuffer, layout, 4, &indexBuffer, 6);

                    u16 triVertexOrder[6] = { 2, 1, 0, 0, 3, 2 };

                    memcpy(vertexBuffer.data, &gVertexArray[0], sizeof(gVertexArray[0]) * 4);
                    memcpy(indexBuffer.data, triVertexOrder, 6 * 2);

                    bgfx::setVertexBuffer(0, &vertexBuffer);
                    bgfx::setIndexBuffer(&indexBuffer);

                    static bgfx::UniformHandle textureUniform = BGFX_INVALID_HANDLE;
                    if (!bgfx::isValid(textureUniform))
                    {
                        textureUniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
                    }
                    bgfx::setTexture(0, textureUniform, getTextureForQuadBGFX(tempQuad));
                    bgfx::setState(0 | BGFX_STATE_WRITE_RGB
                        | BGFX_STATE_WRITE_A
                        | BGFX_STATE_WRITE_Z
                        | BGFX_STATE_DEPTH_TEST_LEQUAL
                        | BGFX_STATE_MSAA
                    );
                    setSpritePriorityUniform(CMDCOLR, CMDPMOD);
                    bgfx::submit(vdp1_gpuView, Get2dUIShaderBGFX());
                }
            }
        }
    }
}

void ScaledSpriteDrawGL(s_vdp1Command* vdp1EA)
{
    u16 CMDCTRL = vdp1EA->m0_CMDCTRL;
    u16 CMDPMOD = vdp1EA->m4_CMDPMOD;
    u16 CMDCOLR = vdp1EA->m6_CMDCOLR;
    u16 CMDSRCA = vdp1EA->m8_CMDSRCA;
    u16 CMDSIZE = vdp1EA->mA_CMDSIZE;
    s16 CMDXA = vdp1EA->mC_CMDXA;
    s16 CMDYA = vdp1EA->mE_CMDYA;
    u16 CMDGRDA = vdp1EA->m1C_CMDGRA;

    if (CMDSRCA)
    {
        int colorMode = (CMDPMOD >> 3) & 0x7;
        u32 characterAddress = ((u32)CMDSRCA) << 3;
        u32 colorBank = ((u32)CMDCOLR) << 1;
        s32 X0 = CMDXA + localCoordiantesX;
        s32 Y0 = CMDYA + localCoordiantesY;
        s32 Width = ((CMDSIZE >> 8) & 0x3F) * 8;
        s32 Height = CMDSIZE & 0xFF;

        s32 X1;
        s32 Y1;

        switch ((CMDCTRL >> 8) & 0xF)
        {
        case 0:
            X1 = vdp1EA->m14_CMDXC + localCoordiantesX + 1;
            Y1 = vdp1EA->m16_CMDYC + localCoordiantesY + 1;
            break;
        default:
            assert(0);
            break;
        }

        float zNear = 0.1f;
        float zFar = 1000.f;

        s_quad tempQuad;
        tempQuad.model = NULL;
        tempQuad.CMDCTRL = CMDCTRL;
        tempQuad.CMDPMOD = CMDPMOD;
        tempQuad.CMDCOLR = CMDCOLR;
        tempQuad.CMDSRCA = CMDSRCA;
        tempQuad.CMDSIZE = CMDSIZE;

        float quadDepth = 0.9;

        s_vd1ExtendedCommand* pExtendedCommand = fetchVdp1ExtendedCommand(*vdp1EA);
        if (pExtendedCommand)
        {
            quadDepth = pExtendedCommand->depth;
        }

        tempQuad.m_vertices[0].m_originalVertices[0] = X0;
        tempQuad.m_vertices[0].m_originalVertices[1] = Y0;
        tempQuad.m_vertices[0].m_originalVertices[2] = quadDepth;

        tempQuad.m_vertices[1].m_originalVertices[0] = X1;
        tempQuad.m_vertices[1].m_originalVertices[1] = Y0;
        tempQuad.m_vertices[1].m_originalVertices[2] = quadDepth;

        tempQuad.m_vertices[2].m_originalVertices[0] = X1;
        tempQuad.m_vertices[2].m_originalVertices[1] = Y1;
        tempQuad.m_vertices[2].m_originalVertices[2] = quadDepth;

        tempQuad.m_vertices[3].m_originalVertices[0] = X0;
        tempQuad.m_vertices[3].m_originalVertices[1] = Y1;
        tempQuad.m_vertices[3].m_originalVertices[2] = quadDepth;

        if (1)
        {
            int flip = (tempQuad.CMDCTRL & 0x30) >> 4;

            float uv[4][2];
            u16 color[4];

            uv[0][0] = 0;
            uv[0][1] = 0;
            color[0] = 1;
            //color[0] = MappedMemoryReadWord(quads[i].gouraudPointer + 0 * 2);

            uv[1][0] = 1;
            uv[1][1] = 0;
            color[1] = 1;
            //color[1] = MappedMemoryReadWord(quads[i].gouraudPointer + 1 * 2);

            uv[2][0] = 1;
            uv[2][1] = 1;
            color[2] = 1;
            //color[2] = MappedMemoryReadWord(quads[i].gouraudPointer + 2 * 2);

            uv[3][0] = 0;
            uv[3][1] = 1;
            color[3] = 1;
            //color[3] = MappedMemoryReadWord(quads[i].gouraudPointer + 3 * 2);

            {
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
                    gVertexArray[j].textures[0] = uv[vertexOrder[j]][0];
                    gVertexArray[j].textures[1] = uv[vertexOrder[j]][1];

                    if (enableVertexColor)
                    {
                        //glColor4f(perVertexColor[j][2], perVertexColor[j][1], perVertexColor[j][0], 1);

                        gVertexArray[j].color[0] = 1;
                        gVertexArray[j].color[1] = 1;
                        gVertexArray[j].color[2] = 1;
                        gVertexArray[j].color[3] = 1;

                    }
                    else
                    {
                        //glColor4f(1, 1, 1, 1);

                        gVertexArray[j].color[0] = 1;
                        gVertexArray[j].color[1] = 1;
                        gVertexArray[j].color[2] = 1;
                        gVertexArray[j].color[3] = 1;
                    }

                    gVertexArray[j].positions[0] = tempQuad.m_vertices[j].m_originalVertices[0];
                    gVertexArray[j].positions[1] = tempQuad.m_vertices[j].m_originalVertices[1];
                    gVertexArray[j].positions[2] = tempQuad.m_vertices[j].m_originalVertices[2];
                }

                {
                    bgfx::VertexLayout layout;
                    layout
                        .begin()
                        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
                        .end();

                    bgfx::TransientVertexBuffer vertexBuffer;
                    bgfx::TransientIndexBuffer indexBuffer;
                    bgfx::allocTransientBuffers(&vertexBuffer, layout, 4, &indexBuffer, 6);

                    u16 triVertexOrder[6] = { 2, 1, 0, 0, 3, 2 };

                    memcpy(vertexBuffer.data, &gVertexArray[0], sizeof(gVertexArray[0]) * 4);
                    memcpy(indexBuffer.data, triVertexOrder, 6 * 2);

                    bgfx::setVertexBuffer(0, &vertexBuffer);
                    bgfx::setIndexBuffer(&indexBuffer);

                    static bgfx::UniformHandle textureUniform = BGFX_INVALID_HANDLE;
                    if (!bgfx::isValid(textureUniform))
                    {
                        textureUniform = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
                    }
                    bgfx::setTexture(0, textureUniform, getTextureForQuadBGFX(tempQuad));

                    bgfx::setState(0 | BGFX_STATE_WRITE_RGB
                        | BGFX_STATE_WRITE_A
                        | BGFX_STATE_WRITE_Z
                        | BGFX_STATE_DEPTH_TEST_LEQUAL
                        | BGFX_STATE_MSAA
                    );
                    setSpritePriorityUniform(CMDCOLR, CMDPMOD);
                    bgfx::submit(vdp1_gpuView, Get2dUIShaderBGFX());
                }
            }
        }
    }
}

void drawQuadGL(const sDebugQuad& quad)
{
    for (int i = 0; i < 4; i++)
    {
        gVertexArray[i].positions[0] = quad.m_vertices[i][0].toFloat();
        gVertexArray[i].positions[1] = quad.m_vertices[i][1].toFloat();
        gVertexArray[i].positions[2] = quad.m_vertices[i][2].toFloat();

        gVertexArray[i].color[0] = quad.m_color.R;
        gVertexArray[i].color[1] = quad.m_color.G;
        gVertexArray[i].color[2] = quad.m_color.B;
        gVertexArray[i].color[3] = quad.m_color.A;
    }

    {
        bgfx::VertexLayout layout;
        layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();

        bgfx::TransientVertexBuffer vertexBuffer;
        bgfx::TransientIndexBuffer indexBuffer;
        bgfx::allocTransientBuffers(&vertexBuffer, layout, 4, &indexBuffer, 6);

        u16 triVertexOrder[6] = { 2, 1, 0, 0, 3, 2 };

        memcpy(vertexBuffer.data, &gVertexArray[0], sizeof(gVertexArray[0]) * 4);
        memcpy(indexBuffer.data, triVertexOrder, 6 * 2);

        bgfx::setVertexBuffer(0, &vertexBuffer);
        bgfx::setIndexBuffer(&indexBuffer);

        bgfx::setState(0 | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
        );

        setSpritePriorityDefault();
        bgfx::submit(vdp1_gpuView, Get2dUIVertexColorShaderBGFX());
    }
}

void drawLineGL(sVec3_FP vertice1, sVec3_FP vertice2, sFColor color)
{
    gVertexArray[0].positions[0] = vertice1[0].toFloat();
    gVertexArray[0].positions[1] = vertice1[1].toFloat();
    gVertexArray[0].positions[2] = vertice1[2].toFloat();

    gVertexArray[1].positions[0] = vertice2[0].toFloat();
    gVertexArray[1].positions[1] = vertice2[1].toFloat();
    gVertexArray[1].positions[2] = vertice2[2].toFloat();

    gVertexArray[0].color[0] = gVertexArray[1].color[0] = color.R;
    gVertexArray[0].color[1] = gVertexArray[1].color[1] = color.G;
    gVertexArray[0].color[2] = gVertexArray[1].color[2] = color.B;
    gVertexArray[0].color[3] = gVertexArray[1].color[3] = color.A;

    {
        bgfx::VertexLayout layout;
        layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();

        bgfx::TransientVertexBuffer vertexBuffer;
        bgfx::TransientIndexBuffer indexBuffer;
        bgfx::allocTransientBuffers(&vertexBuffer, layout, 2, &indexBuffer, 2);

        u16 triVertexOrder[6] = { 0, 1 };

        memcpy(vertexBuffer.data, &gVertexArray[0], sizeof(gVertexArray[0]) * 2);
        memcpy(indexBuffer.data, triVertexOrder, 2 * 2);

        bgfx::setState(0 | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
            | BGFX_STATE_PT_LINES
        );

        static bgfx::UniformHandle vdp1_modelViewProj = BGFX_INVALID_HANDLE;
        if (!isValid(vdp1_modelViewProj))
        {
            vdp1_modelViewProj = bgfx::createUniform("u_customModelViewProj", bgfx::UniformType::Mat4);
        }
        
        glm::mat4 projectionViewMatrix = getProjectionMatrix() * getViewMatrix();

        bgfx::setUniform(vdp1_modelViewProj, &projectionViewMatrix[0][0]);

        bgfx::setVertexBuffer(0, &vertexBuffer);
        bgfx::setIndexBuffer(&indexBuffer);

        setSpritePriorityDefault();
        bgfx::submit(vdp1_gpuView, GetWorldSpaceVertexColorShaderBGFX());
    }
}

void drawLineGL(s16 X1, s16 Y1, s16 X2, s16 Y2, u32 finalColor)
{
    gVertexArray[0].positions[0] = X1;
    gVertexArray[0].positions[1] = Y1;
    gVertexArray[0].positions[2] = 0.f;

    gVertexArray[1].positions[0] = X2;
    gVertexArray[1].positions[1] = Y2;
    gVertexArray[1].positions[2] = 0.f;

    gVertexArray[0].color[0] = gVertexArray[1].color[0] = (finalColor & 0xFF) / 256.f;
    gVertexArray[0].color[1] = gVertexArray[1].color[1] = ((finalColor >> 8) & 0xFF) / 256.f;
    gVertexArray[0].color[2] = gVertexArray[1].color[2] = ((finalColor >> 16) & 0xFF) / 256.f;
    gVertexArray[0].color[3] = gVertexArray[1].color[3] = ((finalColor >> 24) & 0xFF) / 256.f;

    {
        bgfx::VertexLayout layout;
        layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();

        bgfx::TransientVertexBuffer vertexBuffer;
        bgfx::TransientIndexBuffer indexBuffer;
        bgfx::allocTransientBuffers(&vertexBuffer, layout, 2, &indexBuffer, 2);

        u16 triVertexOrder[6] = { 0, 1 };

        memcpy(vertexBuffer.data, &gVertexArray[0], sizeof(gVertexArray[0]) * 2);
        memcpy(indexBuffer.data, triVertexOrder, 2 * 2);

        bgfx::setState(0 | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
            | BGFX_STATE_PT_LINES
        );

        bgfx::setVertexBuffer(0, &vertexBuffer);
        bgfx::setIndexBuffer(&indexBuffer);

        setSpritePriorityDefault();
        bgfx::submit(vdp1_gpuView, Get2dUIVertexColorShaderBGFX());
    }
}

void PolyDrawGL(s_vdp1Command* vdp1EA)
{
    u16 CMDPMOD = vdp1EA->m4_CMDPMOD;
    u16 CMDCOLR = vdp1EA->m6_CMDCOLR;
    s16 CMDXA = vdp1EA->mC_CMDXA;
    s16 CMDYA = vdp1EA->mE_CMDYA;
    s16 CMDXB = vdp1EA->m10_CMDXB;
    s16 CMDYB = vdp1EA->m12_CMDYB;
    s16 CMDXC = vdp1EA->m14_CMDXC;
    s16 CMDYC = vdp1EA->m16_CMDYC;
    s16 CMDXD = vdp1EA->m18_CMDXD;
    s16 CMDYD = vdp1EA->m1A_CMDYD;
    u16 CMDGRDA = vdp1EA->m1C_CMDGRA;

    u32 finalColor;
    if (CMDCOLR & 0x8000)
    {
        finalColor = 0xFF000000 | (((CMDCOLR & 0x1F) << 3) | ((CMDCOLR & 0x03E0) << 6) | ((CMDCOLR & 0x7C00) << 9));
    }
    else
    {
        finalColor = 0xFF0000FF;
    }

    float quadDepth = 0.9;

    s_vd1ExtendedCommand* pExtendedCommand = fetchVdp1ExtendedCommand(*vdp1EA);
    if (pExtendedCommand)
    {
        quadDepth = pExtendedCommand->depth;
    }

    gVertexArray[0].positions[0] = CMDXA + localCoordiantesX;
    gVertexArray[0].positions[1] = CMDYA + localCoordiantesY;
    gVertexArray[0].positions[2] = quadDepth;

    gVertexArray[1].positions[0] = CMDXB + localCoordiantesX;
    gVertexArray[1].positions[1] = CMDYB + localCoordiantesY;
    gVertexArray[1].positions[2] = quadDepth;

    gVertexArray[2].positions[0] = CMDXC + localCoordiantesX;
    gVertexArray[2].positions[1] = CMDYC + localCoordiantesY;
    gVertexArray[2].positions[2] = quadDepth;

    gVertexArray[3].positions[0] = CMDXD + localCoordiantesX;
    gVertexArray[3].positions[1] = CMDYD + localCoordiantesY;
    gVertexArray[3].positions[2] = quadDepth;

    for (int i = 0; i < 4; i++)
    {
        gVertexArray[i].color[0] = (finalColor & 0xFF) / 256.f;
        gVertexArray[i].color[1] = ((finalColor >> 8) & 0xFF) / 256.f;
        gVertexArray[i].color[2] = ((finalColor >> 16) & 0xFF) / 256.f;
        gVertexArray[i].color[3] = ((finalColor >> 24) & 0xFF) / 256.f;
    }

    {
        bgfx::VertexLayout layout;
        layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();

        bgfx::TransientVertexBuffer vertexBuffer;
        bgfx::TransientIndexBuffer indexBuffer;
        bgfx::allocTransientBuffers(&vertexBuffer, layout, 4, &indexBuffer, 6);

        u16 triVertexOrder[6] = { 2, 1, 0, 0, 3, 2 };

        memcpy(vertexBuffer.data, &gVertexArray[0], sizeof(gVertexArray[0]) * 4);
        memcpy(indexBuffer.data, triVertexOrder, 6 * 2);

        bgfx::setState(0 | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
        );

        bgfx::setVertexBuffer(0, &vertexBuffer);
        bgfx::setIndexBuffer(&indexBuffer);

        setSpritePriorityUniform(CMDCOLR, CMDPMOD);
        bgfx::submit(vdp1_gpuView, Get2dUIVertexColorShaderBGFX());
    }
}

void PolyLineDrawGL(s_vdp1Command* vdp1EA)
{
    u16 CMDPMOD = vdp1EA->m4_CMDPMOD;
    u16 CMDCOLR = vdp1EA->m6_CMDCOLR;
    s16 CMDXA = vdp1EA->mC_CMDXA;
    s16 CMDYA = vdp1EA->mE_CMDYA;
    s16 CMDXB = vdp1EA->m10_CMDXB;
    s16 CMDYB = vdp1EA->m12_CMDYB;
    s16 CMDXC = vdp1EA->m14_CMDXC;
    s16 CMDYC = vdp1EA->m16_CMDYC;
    s16 CMDXD = vdp1EA->m18_CMDXD;
    s16 CMDYD = vdp1EA->m1A_CMDYD;
    u16 CMDGRDA = vdp1EA->m1C_CMDGRA;

    u32 finalColor;
    if (CMDCOLR & 0x8000)
    {
        finalColor = 0xFF000000 | (((CMDCOLR & 0x1F) << 3) | ((CMDCOLR & 0x03E0) << 6) | ((CMDCOLR & 0x7C00) << 9));
    }
    else
    {
        finalColor = 0xFF0000FF;
    }

    drawLineGL(CMDXA + localCoordiantesX, CMDYA + localCoordiantesY, CMDXB + localCoordiantesX, CMDYB + localCoordiantesY, finalColor);
    drawLineGL(CMDXB + localCoordiantesX, CMDYB + localCoordiantesY, CMDXC + localCoordiantesX, CMDYC + localCoordiantesY, finalColor);
    drawLineGL(CMDXC + localCoordiantesX, CMDYC + localCoordiantesY, CMDXD + localCoordiantesX, CMDYD + localCoordiantesY, finalColor);
    drawLineGL(CMDXD + localCoordiantesX, CMDYD + localCoordiantesY, CMDXA + localCoordiantesX, CMDYA + localCoordiantesY, finalColor);
}
#endif
    
