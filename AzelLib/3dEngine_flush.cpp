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

void drawObject(s_objectToRender* pObject, glm::mat4& projectionMatrix)
{
    pObject->m_pObject->generateVertexBuffer();
    //return drawObject_SingleDrawCall(pObject, projectionMatrix);

    checkGL();

    float quantisation = (float)0x10000;
    float objectMatrix[4 * 4];
    {

        memset(objectMatrix, 0, sizeof(objectMatrix));
        for (u32 i = 0; i < 4 * 3; i++)
        {
            objectMatrix[i] = pObject->m_modelMatrix.matrix[i] / quantisation;
        }
        objectMatrix[15] = 1.f;
    }

    static bgfx::ProgramHandle vdp1_program = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_2dOffset_handle = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_modelMatrix = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_modelViewProj = BGFX_INVALID_HANDLE;
    static bgfx::UniformHandle vdp1_textureSampler = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(vdp1_program))
    {
        vdp1_program = loadBgfxProgram("VDP1_vs", "VDP1_ps");
        vdp1_2dOffset_handle = bgfx::createUniform("u_2dOffset", bgfx::UniformType::Vec4);
        vdp1_modelViewProj = bgfx::createUniform("u_customModelViewProj", bgfx::UniformType::Mat4);
        vdp1_textureSampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
    }

    if(0)
    {
        float proj[16];
        bx::mtxProj(proj, 50.0f, float(352.f) / float(224.f), 0.1f, 1000.f, bgfx::getCaps()->homogeneousDepth);

        float view[16];
        bx::mtxIdentity(view);

        bgfx::setViewTransform(vdp1_gpuView, objectMatrix, proj);
    }

    //transposeMatrix(objectMatrix);
    //transposeMatrix(projectionMatrix);
    /*
    glm::mat4 tempProjection;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tempProjection[i][j] = projectionMatrix[j * 4 + i];
        }
    }
    */
    glm::mat4 tempObjectMatrix;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tempObjectMatrix[i][j] = objectMatrix[j * 4 + i];
        }
    }
    glm::mat4 mvpMatrix = projectionMatrix * tempObjectMatrix;

    //bgfx::setUniform(vdp1_modelMatrix, objectMatrix);
    float _2dOffset[4];
    _2dOffset[0] = pObject->m_2dOffset[0];
    _2dOffset[1] = pObject->m_2dOffset[1];
    bgfx::setUniform(vdp1_2dOffset_handle, _2dOffset);
    bgfx::setUniform(vdp1_modelViewProj, &mvpMatrix[0][0]);

    bgfx::setTransform(objectMatrix);

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

float* getViewMatrix()
{
    static float fViewMatrix[4 * 4];

    memset(fViewMatrix, 0, sizeof(fViewMatrix));
    for (u32 i = 0; i < 4 * 3; i++)
    {
        fViewMatrix[i] = matrixStack[0].matrix[i] / (float)0x10000;
    }
    fViewMatrix[15] = 1.f;
    transposeMatrix(fViewMatrix);

    return fViewMatrix;
}

float fov = 40.f;
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

    transposeMatrix(fEarlyProjectionMatrix);
#else

    glm::mat4 testProj = glm::perspectiveFov(glm::radians(50.f), 352.f, 224.f, zNear, zFar);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            fEarlyProjectionMatrix[i * 4 + j] = testProj[i][j];
        }
    }
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

    TracyPlot("ObjectRenderList size", (int64_t)objectRenderList.size());

    objectRenderList.clear();

    for (int i = 0; i < debugQuads.size(); i++)
    {
        drawQuadGL(debugQuads[i]);
    }
    debugQuads.clear();

    for(int i=0; i<debugLines.size(); i++)
    {
        drawLineGL(debugLines[i].position1, debugLines[i].position2, debugLines[i].m_color);
    }
    debugLines.clear();
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

bgfx::ProgramHandle GetWorldSpaceLineShaderBGFX()
{
    static bgfx::ProgramHandle programHandle = BGFX_INVALID_HANDLE;
    if (!bgfx::isValid(programHandle))
    {
        programHandle = loadBgfxProgram("VDP1_vertexColor_vs", "VDP1_vertexColor_ps");
    }

    return programHandle;
}

void NormalSpriteDrawGL(u32 vdp1EA)
{
    u16 CMDCTRL = getVdp1VramU16(vdp1EA + 0);
    u16 CMDPMOD = getVdp1VramU16(vdp1EA + 4);
    u16 CMDCOLR = getVdp1VramU16(vdp1EA + 6);
    u16 CMDSRCA = getVdp1VramU16(vdp1EA + 8);
    u16 CMDSIZE = getVdp1VramU16(vdp1EA + 0xA);
    s16 CMDXA = getVdp1VramU16(vdp1EA + 0xC);
    s16 CMDYA = getVdp1VramU16(vdp1EA + 0xE);
    u16 CMDGRDA = getVdp1VramU16(vdp1EA + 0x1C);

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

        s_vd1ExtendedCommand* pExtendedCommand = fetchVdp1ExtendedCommand(vdp1EA);
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
            checkGL();
            //glEnable(GL_ALPHA_TEST);
            //glAlphaFunc(GL_GREATER, 0.1);
            checkGL();

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

            //glColor3f(1, 1, 1);
            checkGL();
            //                                 if (enableTextures)
            //                                 {
            //                                     glEnable(GL_TEXTURE_2D);
            //                                 }
            //                                 else
            //                                 {
            //                                     glDisable(GL_TEXTURE_2D);
            //                                 }
            checkGL();
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
                        | BGFX_STATE_CULL_CW
                        | BGFX_STATE_MSAA
                    );
                    bgfx::submit(vdp1_gpuView, Get2dUIShaderBGFX());
                }
            }
        }
    }
}

void ScaledSpriteDrawGL(u32 vdp1EA)
{
    u16 CMDCTRL = getVdp1VramU16(vdp1EA + 0);
    u16 CMDPMOD = getVdp1VramU16(vdp1EA + 4);
    u16 CMDCOLR = getVdp1VramU16(vdp1EA + 6);
    u16 CMDSRCA = getVdp1VramU16(vdp1EA + 8);
    u16 CMDSIZE = getVdp1VramU16(vdp1EA + 0xA);
    s16 CMDXA = getVdp1VramU16(vdp1EA + 0xC);
    s16 CMDYA = getVdp1VramU16(vdp1EA + 0xE);
    u16 CMDGRDA = getVdp1VramU16(vdp1EA + 0x1C);

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

        switch ((getVdp1VramU16(vdp1EA + 0) >> 8) & 0xF)
        {
        case 0:
            X1 = getVdp1VramS16(vdp1EA + 0x14) + localCoordiantesX + 1;
            Y1 = getVdp1VramS16(vdp1EA + 0x16) + localCoordiantesY + 1;
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

        s_vd1ExtendedCommand* pExtendedCommand = fetchVdp1ExtendedCommand(vdp1EA);
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

            //glColor3f(1, 1, 1);
            checkGL();
            //                                 if (enableTextures)
            //                                 {
            //                                     glEnable(GL_TEXTURE_2D);
            //                                 }
            //                                 else
            //                                 {
            //                                     glDisable(GL_TEXTURE_2D);
            //                                 }
            checkGL();
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
                        | BGFX_STATE_CULL_CW
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
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .end();

        bgfx::TransientVertexBuffer vertexBuffer;
        bgfx::TransientIndexBuffer indexBuffer;
        bgfx::allocTransientBuffers(&vertexBuffer, layout, 4, &indexBuffer, 6);
        bgfx::setState(0 | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
        );

        bgfx::submit(vdp1_gpuView, GetWorldSpaceLineShaderBGFX());
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
}

void PolyDrawGL(u32 vdp1EA)
{
    u16 CMDPMOD = getVdp1VramU16(vdp1EA + 4);
    u16 CMDCOLR = getVdp1VramU16(vdp1EA + 6);
    s16 CMDXA = getVdp1VramS16(vdp1EA + 0xC);
    s16 CMDYA = getVdp1VramS16(vdp1EA + 0xE);
    s16 CMDXB = getVdp1VramS16(vdp1EA + 0x10);
    s16 CMDYB = getVdp1VramS16(vdp1EA + 0x12);
    s16 CMDXC = getVdp1VramS16(vdp1EA + 0x14);
    s16 CMDYC = getVdp1VramS16(vdp1EA + 0x16);
    s16 CMDXD = getVdp1VramS16(vdp1EA + 0x18);
    s16 CMDYD = getVdp1VramS16(vdp1EA + 0x1A);
    u16 CMDGRDA = getVdp1VramU16(vdp1EA + 0x1C);

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

void PolyLineDrawGL(u32 vdp1EA)
{
    u16 CMDPMOD = getVdp1VramU16(vdp1EA + 4);
    u16 CMDCOLR = getVdp1VramU16(vdp1EA + 6);
    s16 CMDXA = getVdp1VramS16(vdp1EA + 0xC);
    s16 CMDYA = getVdp1VramS16(vdp1EA + 0xE);
    s16 CMDXB = getVdp1VramS16(vdp1EA + 0x10);
    s16 CMDYB = getVdp1VramS16(vdp1EA + 0x12);
    s16 CMDXC = getVdp1VramS16(vdp1EA + 0x14);
    s16 CMDYC = getVdp1VramS16(vdp1EA + 0x16);
    s16 CMDXD = getVdp1VramS16(vdp1EA + 0x18);
    s16 CMDYD = getVdp1VramS16(vdp1EA + 0x1A);
    u16 CMDGRDA = getVdp1VramU16(vdp1EA + 0x1C);

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
    
