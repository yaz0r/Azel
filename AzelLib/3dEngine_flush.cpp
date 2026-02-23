#include "PDS.h"
#include "renderer/renderer_gl.h"
#include "processModel.h"

#include <bx/math.h>

extern std::array<sMatrix4x3, 16> matrixStack;

#include "3dEngine_textureCache.h"
#include <unordered_map>

#if !defined(USE_NULL_RENDERER)

const char* gGLSLVersion = nullptr;

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
    float m_2dOffset[2];
    bool m_isBillboard;
};

std::vector<s_objectToRender> objectRenderList;

void addObjectToDrawList(sProcessed3dModel* pObjectData)
{
    s_objectToRender newObject;
    newObject.m_pObject = pObjectData;
    newObject.m_modelMatrix = *pCurrentMatrix;
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
    cameraProperties2.m88_billboardViewMatrix.matrix[3] = pCurrentMatrix->matrix[3];
    cameraProperties2.m88_billboardViewMatrix.matrix[7] = pCurrentMatrix->matrix[7];
    cameraProperties2.m88_billboardViewMatrix.matrix[11] = pCurrentMatrix->matrix[11];

    s_objectToRender newObject;
    newObject.m_pObject = pObjectData;
    newObject.m_modelMatrix = cameraProperties2.m88_billboardViewMatrix;
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

void GetDistanceFalloff(float* falloutColor, s_objectToRender* pObject, int vertexId)
{
    float colorIndex = 0;// computeVertexCameraDistance(pObject, vertexId);

    colorIndex /= 128;

    colorIndex -= fmodf(colorIndex, 8);
    colorIndex /= 6;

    int value = (int)colorIndex;
    if (value < 0)
        value = 0;
    if (value >= 31)
        value = 31;

    //falloutColor[2] = (s16)MappedMemoryReadWord(0x0601FBDC + 8 * value + 0);
    //falloutColor[1] = (s16)MappedMemoryReadWord(0x0601FBDC + 8 * value + 2);
    //falloutColor[0] = (s16)MappedMemoryReadWord(0x0601FBDC + 8 * value + 4);

    falloutColor[0] = 0;
    falloutColor[1] = 0;
    falloutColor[2] = 0;
}

void ComputeColorFromNormal(const sProcessed3dModel::sQuadExtra& extraData, bool withColor, s32* lightVectorModelSpace, s16* lightColor, float* falloutColor, float* perVertexColor, u8* &instanceDataEA)
{
    s16 fixedNormal[3];
    fixedNormal[0] = extraData.m0_normals[0];
    fixedNormal[1] = extraData.m0_normals[1];
    fixedNormal[2] = extraData.m0_normals[2];

    float fixedColor[3];
    fixedColor[0] = 0;
    fixedColor[1] = 0;
    fixedColor[2] = 0;

    if (withColor)
    {
        fixedColor[0] = extraData.m6_colors[0];
        fixedColor[1] = extraData.m6_colors[1];
        fixedColor[2] = extraData.m6_colors[2];
    }

    if (instanceDataEA != 0)
    {
        fixedColor[0] = ((u8)READ_BE_U16(instanceDataEA)); instanceDataEA += 1;
        fixedColor[1] = ((u8)READ_BE_U16(instanceDataEA)); instanceDataEA += 1;
        fixedColor[2] = ((u8)READ_BE_U16(instanceDataEA)); instanceDataEA += 1;

        fixedColor[0] *= 256.f;
        fixedColor[1] *= 256.f;
        fixedColor[2] *= 256.f;
    }

    float accumulator = 0;
    accumulator += fixedNormal[0] * lightVectorModelSpace[0];
    accumulator += fixedNormal[1] * lightVectorModelSpace[1];
    accumulator += fixedNormal[2] * lightVectorModelSpace[2];
    accumulator /= (float)0x10000;

    if (accumulator < 0.f)
        accumulator = 0.f;

    perVertexColor[0] = (lightColor[0] * accumulator + falloutColor[0] + fixedColor[0]) / (256.f);
    perVertexColor[1] = (lightColor[1] * accumulator + falloutColor[1] + fixedColor[1]) / (256.f);
    perVertexColor[2] = (lightColor[2] * accumulator + falloutColor[2] + fixedColor[2]) / (256.f);

    //valid range for gouraud should be in the [0,0x1F] range. Clamp everything over that.

    for (int i = 0; i < 3; i++)
    {
        if (perVertexColor[i] < 0)
            perVertexColor[i] = 0;
        if (perVertexColor[i] > 31.f)
            perVertexColor[i] = 31.f;

        // remap to [-0.5, 0.5] as this is how gouraud is supposed to work
        perVertexColor[i] /= 31.f;
        perVertexColor[i] -= 0.5f;
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
        for (u32 i = 0; i < 4 * 3; i++)
        {
            objectMatrix[i] = inputMatrix.matrix[i] / quantisation;
        }
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
    //return drawObject_SingleDrawCall(pObject, projectionMatrix);

    static bgfx::ProgramHandle vdp1_program = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_modelMatrix = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_modelViewProj = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_textureSampler = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(vdp1_program))
    {
        vdp1_program = loadBgfxProgram("VDP1_vs", "VDP1_ps");
        vdp1_modelViewProj = bgfx::createUniform("u_customModelViewProj", bgfx::UniformType::Mat4);
        vdp1_textureSampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
    }

    glm::mat4 mvpMatrix = projectionMatrix * MatrixToGLM(pObject->m_modelMatrix);

    //bgfx::setUniform(vdp1_modelMatrix, objectMatrix);
    float _2dOffset[4];
    _2dOffset[0] = pObject->m_2dOffset[0];
    _2dOffset[1] = pObject->m_2dOffset[1];
    bgfx::setUniform(vdp1_modelViewProj, &mvpMatrix[0][0]);

    uint64_t state = 0
        | BGFX_STATE_DEPTH_TEST_LEQUAL
        | BGFX_STATE_WRITE_RGB
        | BGFX_STATE_WRITE_A
        | BGFX_STATE_WRITE_Z
        | BGFX_STATE_CULL_CCW
        | BGFX_STATE_MSAA
        | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
        ;

    bgfx::setState(state);

    bgfx::setTexture(0, vdp1_textureSampler, pObject->m_pObject->m_textureAtlas);
    bgfx::setVertexBuffer(0, pObject->m_pObject->m_vertexBufferHandle);
    bgfx::setIndexBuffer(pObject->m_pObject->m_indexBufferHandle);
    bgfx::submit(vdp1_gpuView, vdp1_program);
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


    if(!isShipping())
    {
        if (ImGui::Begin("Objects"))
        {
            for (int i = 0; i < objectRenderList.size(); i++)
            {
                char buffer[256];
                sprintf(buffer, "Object %i", i);
                ImGui::PushID(buffer);
                {
                    for (int j = 0; j < 3; j++)
                    {
                        float vertex[4];
                        vertex[0] = objectRenderList[i].m_modelMatrix.matrix[j * 4 + 0] / (float)0x10000;
                        vertex[1] = objectRenderList[i].m_modelMatrix.matrix[j * 4 + 1] / (float)0x10000;
                        vertex[2] = objectRenderList[i].m_modelMatrix.matrix[j * 4 + 2] / (float)0x10000;
                        vertex[3] = objectRenderList[i].m_modelMatrix.matrix[j * 4 + 3] / (float)0x10000;

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
    
