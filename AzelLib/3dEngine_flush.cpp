#include "PDS.h"
#include "renderer/renderer_gl.h"

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
    newObject.m_2dOffset[0] = (graphicEngineStatus.m405C.localCoordinatesX - (352.f / 2.f)) / 352.f;
    newObject.m_2dOffset[1] = (graphicEngineStatus.m405C.localCoordinatesY - (224.f / 2.f)) / 224.f;
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
    newObject.m_2dOffset[0] = (graphicEngineStatus.m405C.localCoordinatesX - (352.f / 2.f)) / 352.f;
    newObject.m_2dOffset[1] = (graphicEngineStatus.m405C.localCoordinatesY - (224.f / 2.f)) / 224.f;
    newObject.m_isBillboard = 1;

    objectRenderList.push_back(newObject);
}

void checkGLImpl(const char* file, unsigned int line)
{
    GLenum error = glGetError();
    
    if (error != GL_NO_ERROR)
    {
        printf("GL error %d: file: %s line:%d\n", error, file, line);

        //assert(0);
    }
    
    //while (error != GL_NO_ERROR);
}

GLuint compileShader(const char* VS, const char* PS)
{
    GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertex_shader_with_version[2] = { gGLSLVersion, VS };
    glShaderSource(vertexshader, 2, vertex_shader_with_version, 0);
    glCompileShader(vertexshader);
    GLint IsCompiled_VS = 0;
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
    if (IsCompiled_VS == 0)
    {
        GLint maxLength;
        glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &maxLength);
        char* vertexInfoLog = (char *)malloc(maxLength);

        glGetShaderInfoLog(vertexshader, maxLength, &maxLength, vertexInfoLog);

        assert(false);

        free(vertexInfoLog);
    }

    GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* pixel_shader_with_version[2] = { gGLSLVersion, PS };
    glShaderSource(fragmentshader, 2, pixel_shader_with_version, 0);
    glCompileShader(fragmentshader);
    GLint IsCompiled_PS = 0;
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_PS);
    if (IsCompiled_PS == 0)
    {
        GLint maxLength;
        glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &maxLength);
        char* fragmentInfoLog = (char *)malloc(maxLength);

        glGetShaderInfoLog(fragmentshader, maxLength, &maxLength, fragmentInfoLog);

        assert(false);

        free(fragmentInfoLog);
    }

    GLuint shaderprogram = glCreateProgram();
    glAttachShader(shaderprogram, vertexshader);
    glAttachShader(shaderprogram, fragmentshader);

    glLinkProgram(shaderprogram);

    GLint IsLinked = 0;
    glGetProgramiv(shaderprogram, GL_LINK_STATUS, &IsLinked);
    if (IsLinked == 0)
    {
        GLint maxLength;
        glGetShaderiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);
        char* fragmentInfoLog = (char*)malloc(maxLength);

        glGetShaderInfoLog(shaderprogram, maxLength, &maxLength, fragmentInfoLog);

        assert(false);

        free(fragmentInfoLog);
    }

    return shaderprogram;
}

#define GL_SHADER_VERSION "#version 300 es\n"

#ifdef USE_GL_ES3
const GLchar azel_vs[] =
"#version 300 es\n"
"uniform mat4 u_mvpMatrix;    \n"
"uniform mat4 u_modelMatrix;    \n"
"uniform vec2 u_2dOffset;   \n"
"in vec3 a_position;   \n"
"in vec2 a_texcoord;   \n"
"in vec4 a_color;   \n"
"out  highp vec2 v_texcoord;     \n"
"out  highp vec4 v_color;     \n"
"void main()                  \n"
"{                            \n"
"   gl_Position = u_mvpMatrix * u_modelMatrix * vec4(a_position, 1); \n"
"   gl_Position.xy += u_2dOffset; \n"
"   v_texcoord = a_texcoord; \n"
"	v_color = a_color; \n"
"} "
;

const GLchar azel_ps[] =
"#version 300 es\n"
"precision highp float;                                    \n"
"in highp vec2 v_texcoord;                                \n"
"in highp vec4 v_color;                                \n"
"uniform sampler2D s_texture;                            \n"
"uniform float s_textureInfluence;                        \n"
"uniform float s_ambientInfluence;                        \n"
"out vec4 fragColor;                                    \n"
"void main()                                            \n"
"{                                                        \n"
"   float distanceValue = mix(0.f, 10.f, 1.f-gl_FragCoord.z); \n"
"   //distanceValue = clamp(distanceValue, 0, 1); \n"
"    vec4 txcol = texture(s_texture, v_texcoord);        \n"
"   if(txcol.a <= 0.f) discard;\n"
"   fragColor = (clamp(txcol, 0.f, 1.f) * s_textureInfluence) + v_color;                                    \n"
"   fragColor = txcol; \n"
"   fragColor.w = 1.f;                                \n"
"}                                                        \n"
;

#else
const GLchar azel_vs[] =
"#version 330 \n"
"uniform mat4 u_mvpMatrix;    \n"
"uniform mat4 u_modelMatrix;    \n"
"uniform vec2 u_2dOffset;   \n"
"in vec3 a_position;   \n"
"in vec2 a_texcoord;   \n"
"in vec4 a_color;   \n"
"out  highp vec2 v_texcoord;     \n"
"out  highp vec4 v_color;     \n"
"void main()                  \n"
"{                            \n"
"   gl_Position = u_mvpMatrix * u_modelMatrix * vec4(a_position, 1); \n"
"   gl_Position.xy += u_2dOffset; \n"
"   v_texcoord = a_texcoord; \n"
"	v_color = a_color; \n"
"} "
;

const GLchar azel_ps[] =
"#version 330 \n"
"precision highp float;									\n"
"in highp vec2 v_texcoord;								\n"
"in highp vec4 v_color;								\n"
"in vec4 gl_FragCoord;									\n"
"uniform sampler2D s_texture;							\n"
"uniform sampler2D s_falloff;							\n"
"uniform float s_textureInfluence;						\n"
"uniform float s_ambientInfluence;						\n"
"out vec4 fragColor;									\n"
"void main()											\n"
"{														\n"
"   float distanceValue = mix(0, 10, 1-gl_FragCoord.z); \n"
"   //distanceValue = clamp(distanceValue, 0, 1); \n"
"	vec4 fallout = texture(s_falloff, vec2(distanceValue,0)); \n"
"	vec4 txcol = texture(s_texture, v_texcoord);		\n"
"   if(txcol.a <= 0) discard;\n"
"   fragColor = (clamp(txcol, 0, 1) * s_textureInfluence) + v_color;									\n"
"   fragColor = txcol; \n"
"   fragColor.w = 1;								\n"
"}														\n"
;
#endif
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

GLuint shaderProgram = 0;
GLuint vshader = 0;
GLuint fshader = 0;
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
GLuint colorRampTexture = 0;

std::vector<s_vertexData> SingleDrawcallVertexArray;
std::vector<uint32_t> SingleDrawcallIndexArray;

void drawObject(s_objectToRender* pObject, float* projectionMatrix)
{
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

        transposeMatrix(objectMatrix);

        //glMatrixMode(GL_MODELVIEW);
        //glLoadMatrixf(objectMatrix);
    }

    float objectProjMatrix[4 * 4];
    multiplyMatrices(objectMatrix, projectionMatrix, objectProjMatrix);

    if (pObject->m_isBillboard)
    {
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
    }

    {
        checkGL();
        GLuint modelProjection = glGetUniformLocation(shaderProgram, (const GLchar *)"u_mvpMatrix");
        checkGL();
        if (modelProjection != -1)
        {
            glUniformMatrix4fv(modelProjection, 1, GL_FALSE, projectionMatrix);
        }
        checkGL();

        GLuint modelMatrixId = glGetUniformLocation(shaderProgram, (const GLchar *)"u_modelMatrix");
        checkGL();
        if (modelMatrixId != -1)
        {
            glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, objectMatrix);
        }
        checkGL();

        GLuint offsetId = glGetUniformLocation(shaderProgram, (const GLchar *)"u_2dOffset");
        checkGL();
        if (modelMatrixId != -1)
        {
            glUniform2fv(offsetId, 1, pObject->m_2dOffset);
        }
        checkGL();
    }

    s32 lightVectorModelSpace[3] = { 0,0,0 };
    {
        /*
        lightVectorModelSpace[0] = (pObject->m_lightVector[0] * pObject->m_modelMatrix[0] + pObject->m_lightVector[1] * pObject->m_modelMatrix[4] + pObject->m_lightVector[2] * pObject->m_modelMatrix[8]) >> 16;
        lightVectorModelSpace[1] = (pObject->m_lightVector[0] * pObject->m_modelMatrix[1] + pObject->m_lightVector[1] * pObject->m_modelMatrix[5] + pObject->m_lightVector[2] * pObject->m_modelMatrix[9]) >> 16;
        lightVectorModelSpace[2] = (pObject->m_lightVector[0] * pObject->m_modelMatrix[2] + pObject->m_lightVector[1] * pObject->m_modelMatrix[6] + pObject->m_lightVector[2] * pObject->m_modelMatrix[10]) >> 16;
        */
    }

    sProcessed3dModel* pModel = pObject->m_pObject;
    {
        u32 currentBlockId = 0;
        u32 currentBlockOffset = 0;

        do
        {
            currentBlockOffset = 0;
            {
                {
                    for(int i=0; i< pModel->mC_Quads.size(); i++)
                    {
                        const sProcessed3dModel::sQuad& currentQuad = pModel->mC_Quads[i];

                        char string[1024] = "";
                        sprintf(string, "Quad: CMDSRCA: %04X CMDCOLR: %04X", currentQuad.m10_CMDSRCA, currentQuad.mE_CMDCOLR);
                        /*glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, strlen(string), string);
*/
                        u8 lightingMode = (currentQuad.m8_lightingControl >> 8) & 3;

                        u32 indices[6] = { 0, 1, 2, 0, 2, 3 };

                        {
                            s_quad tempQuad;
                            tempQuad.model = pModel->_base;
                            tempQuad.CMDCTRL = currentQuad.mA_CMDCTRL;
                            tempQuad.CMDPMOD = currentQuad.mC_CMDPMOD;
                            tempQuad.CMDCOLR = currentQuad.mE_CMDCOLR;
                            tempQuad.CMDSRCA = currentQuad.m10_CMDSRCA;
                            tempQuad.CMDSIZE = currentQuad.m12_onCollisionScriptIndex;

                            for (int i = 0; i < 4; i++)
                            {
                                float fX = pModel->m8_vertices[currentQuad.m0_indices[i]][0] / quantisation;
                                float fY = pModel->m8_vertices[currentQuad.m0_indices[i]][1] / quantisation;
                                float fZ = pModel->m8_vertices[currentQuad.m0_indices[i]][2] / quantisation;

                                tempQuad.m_vertices[i].m_originalVertices[0] = fX * 16;
                                tempQuad.m_vertices[i].m_originalVertices[1] = fY * 16;
                                tempQuad.m_vertices[i].m_originalVertices[2] = fZ * 16;
                                tempQuad.m_vertices[i].m_originalVertices[3] = 1.f;
                            }

                            float perVertexColor[4][3];

                            for (int i = 0; i < 4; i++)
                            {
                                perVertexColor[i][0] = 1.f;
                                perVertexColor[i][1] = 1.f;
                                perVertexColor[i][2] = 1.f;
                            }

                            float falloutColor[4][3];
                            for (int i = 0; i < 4; i++)
                            {
                                GetDistanceFalloff(falloutColor[i], pObject, currentQuad.m0_indices[i]);
                            }

                            u8* colorOverrideEA = 0;
                            //if (instanceEA)
                            {
                                /*u32 currectSeekAddress = instanceEA;
                                while (AzelLowWramMemoryReadLong(currectSeekAddress) != 0xFFFFFFFF)
                                {
                                if (AzelLowWramMemoryReadLong(currectSeekAddress) == quadIndex)
                                {
                                colorOverrideEA = currectSeekAddress + 4;
                                }
                                currectSeekAddress += 0x10;
                                }*/
                            }

                            switch (lightingMode)
                            {
                            case 0: // plain texture
                                break;
                            case 1: // texture + single normal, used for shadows
                                ComputeColorFromNormal(currentQuad.m14_extraData[0], false, lightVectorModelSpace, pObject->m_lightColor, falloutColor[0], perVertexColor[0], colorOverrideEA);
                                break;
                            case 2: // texture + normal + color per vertex
                                for (int i = 0; i < 4; i++)
                                {
                                    ComputeColorFromNormal(currentQuad.m14_extraData[i], true, lightVectorModelSpace, pObject->m_lightColor, falloutColor[i], perVertexColor[i], colorOverrideEA);
                                }
                                break;
                            case 3: // texture + normal per vertex
                                for (int i = 0; i < 4; i++)
                                {
                                    ComputeColorFromNormal(currentQuad.m14_extraData[i], false, lightVectorModelSpace, pObject->m_lightColor, falloutColor[i], perVertexColor[i], colorOverrideEA);
                                }
                                break;
                            }

                            checkGL();

                            GLuint textureId = getTextureForQuad(tempQuad);
                            if (textureId > 0)
                            {
                                checkGL();
                                //glEnable(GL_ALPHA_TEST);
                                //glAlphaFunc(GL_GREATER, 0.1);
                                glBindTexture(GL_TEXTURE_2D, textureId);
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
                                bool bUseVBO = true;

                                if (bUseVBO)
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

                                            gVertexArray[j].color[0] = perVertexColor[j][2];
                                            gVertexArray[j].color[1] = perVertexColor[j][1];
                                            gVertexArray[j].color[2] = perVertexColor[j][0];
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

                                        {
                                            tempQuad.m_vertices[j].m_originalVertices[3] = 1;
                                            //glVertex4fv(tempQuad.m_vertices[j].m_originalVertices);
                                        }

                                        gVertexArray[j].positions[0] = tempQuad.m_vertices[j].m_originalVertices[0];
                                        gVertexArray[j].positions[1] = tempQuad.m_vertices[j].m_originalVertices[1];
                                        gVertexArray[j].positions[2] = tempQuad.m_vertices[j].m_originalVertices[2];
                                    }

                                    checkGL();

                                    GLuint vertexp = glGetAttribLocation(shaderProgram, (const GLchar *)"a_position");
                                    GLuint texcoordp = glGetAttribLocation(shaderProgram, (const GLchar *)"a_texcoord");
                                    GLuint colorp = glGetAttribLocation(shaderProgram, (const GLchar *)"a_color");
                                    GLuint texture = glGetUniformLocation(shaderProgram, (const GLchar*)"s_texture");
                                    GLuint falloff = glGetUniformLocation(shaderProgram, (const GLchar*)"s_falloff");
                                    GLuint textureInfluenceHandle = glGetUniformLocation(shaderProgram, (const GLchar*)"s_textureInfluence");

                                    checkGL();

                                    static GLuint vao = 0;
                                    if (vao == 0)
                                    {
                                        glGenVertexArrays(1, &vao);
                                    }
                                    glBindVertexArray(vao);

                                    static GLuint vbo = 0;
                                    if (vbo == 0)
                                    {
                                        glGenBuffers(1, &vbo);
                                    }
                                    checkGL();
                                    glBindBuffer(GL_ARRAY_BUFFER, vbo);
                                    checkGL();
                                    glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertexData) * 4, &gVertexArray[0], GL_STATIC_DRAW);
                                    checkGL();
                                    glVertexAttribPointer(vertexp, 3, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->positions);
                                    checkGL();
                                    glEnableVertexAttribArray(vertexp);
                                    checkGL();
                                    if (texcoordp != -1)
                                    {
                                        glVertexAttribPointer(texcoordp, 2, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->textures);
                                        glEnableVertexAttribArray(texcoordp);
                                    }
                                    checkGL();
                                    if (colorp != -1)
                                    {
                                        glVertexAttribPointer(colorp, 4, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->color);
                                        glEnableVertexAttribArray(colorp);
                                    }
                                    checkGL();
                                    if (texture != -1)
                                    {
                                        glUniform1i(texture, 0);
                                        glActiveTexture(GL_TEXTURE0);
                                        glBindTexture(GL_TEXTURE_2D, textureId);
                                    }
                                    checkGL();
                                    if (textureInfluenceHandle != -1)
                                    {
                                        glUniform1f(textureInfluenceHandle, textureInfluence);
                                    }
                                    checkGL();
                                    if (falloff != -1)
                                    {
                                        glUniform1i(falloff, 1);
                                        glActiveTexture(GL_TEXTURE1);
                                        glBindTexture(GL_TEXTURE_2D, colorRampTexture);
                                    }

                                    static GLuint indexBufferId = 0;
                                    if (indexBufferId == 0)
                                    {
                                        char triVertexOrder[6] = { 0, 1, 2, 2, 3, 0 };
                                        glGenBuffers(1, &indexBufferId);
                                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
                                        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triVertexOrder), triVertexOrder, GL_STATIC_DRAW);
                                    }
                                    else
                                    {
                                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
                                    }
                                    
                                    checkGL();
                                    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                                    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, triVertexOrder);
                                    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*)0);
                                    checkGL();
                                    glDisableVertexAttribArray(vertexp);
                                    checkGL();
                                    if (texcoordp != -1)
                                        glDisableVertexAttribArray(texcoordp);
                                    checkGL();
                                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                                    checkGL();

                                    if (texture != -1)
                                    {
                                        glUniform1i(texture, 0);
                                        glActiveTexture(GL_TEXTURE0);
                                        glBindTexture(GL_TEXTURE_2D, 0);
                                    }

                                    if (falloff != -1)
                                    {
                                        glUniform1i(falloff, 0);
                                        glActiveTexture(GL_TEXTURE1);
                                        glBindTexture(GL_TEXTURE_2D, 0);
                                    }
                                }
                                else
                                {
#if 0
                                    glBegin(GL_QUADS);
                                    for (int j = 0; j < 4; j++)
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

                                    /*
                                    glTexCoord2f(uv[vertexOrder[j]][0], uv[vertexOrder[j]][1]);
                                    if (enableVertexColor)
                                    {
                                    glColor4f(perVertexColor[j][2], perVertexColor[j][1], perVertexColor[j][0], 1);
                                    }
                                    else
                                    {
                                    glColor4f(1, 1, 1, 1);
                                    }

                                    {
                                    tempQuad.m_vertices[j].m_originalVertices[3] = 1;
                                    glVertex4fv(tempQuad.m_vertices[j].m_originalVertices);
                                    }

                                    }
                                    glEnd();
                                    */
#endif
                                }
                            }
                        }
                    }
                }
            }
        } while (0);
    }
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

float* getProjectionMatrix()
{
    static float fEarlyProjectionMatrix[4 * 4];

    float zNear = 0.1f;
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
    return fEarlyProjectionMatrix;
}

void flushObjectsToDrawList()
{
    gVertexArray.resize(1024 * 1024);
    checkGL();

    static bool bInit = false;
    if (!bInit)
    {
        vshader = glCreateShader(GL_VERTEX_SHADER);
        {
            volatile int compiled = 0;
            const GLchar * pYglprg_normal_v[] = { azel_vs, NULL };
            glShaderSource(vshader, 1, pYglprg_normal_v, NULL);
            glCompileShader(vshader);
            glGetShaderiv(vshader, GL_COMPILE_STATUS, (int*)&compiled);
            if (compiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &maxLength);

                // The maxLength includes the NULL character
                std::vector<GLchar> errorLog(maxLength);
                glGetShaderInfoLog(vshader, maxLength, &maxLength, &errorLog[0]);
                PDS_unimplemented(errorLog.data());
                assert(false);
            }
        }

        fshader = glCreateShader(GL_FRAGMENT_SHADER);
        {
            volatile int compiled = 0;
            const GLchar * pYglprg_normal_f[] = { azel_ps, NULL };
            glShaderSource(fshader, 1, pYglprg_normal_f, NULL);
            glCompileShader(fshader);
            glGetShaderiv(fshader, GL_COMPILE_STATUS, (int*)&compiled);
            if (compiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(fshader, GL_INFO_LOG_LENGTH, &maxLength);
                
                // The maxLength includes the NULL character
                std::vector<GLchar> errorLog(maxLength);
                glGetShaderInfoLog(fshader, maxLength, &maxLength, &errorLog[0]);
                PDS_unimplemented(errorLog.data());
                assert(false);
            }
        }

        shaderProgram = glCreateProgram();
        {
            volatile int linked = 0;
            glAttachShader(shaderProgram, vshader);
            glAttachShader(shaderProgram, fshader);
            glLinkProgram(shaderProgram);
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, (int*)&linked);
            assert(linked == 1);
        }

        bInit = true;
    }

    if(ImGui::Begin("Objects"))
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

    checkGL();

    glUseProgram(shaderProgram);
    checkGL();

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

    checkGL();
    glUseProgram(0);

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

GLuint Get2dUIShader()
{
    static const GLchar UI_vs[] =
        "in vec3 a_position;   \n"
        "in vec2 a_texcoord;   \n"
        "out  highp vec2 v_texcoord;     \n"
        "out  highp float v_depth;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vec4(a_position, 1); \n"
        "   gl_Position.x = (a_position.x / (352.f/2.f)) - 1.f;"
        "   gl_Position.y = 1.f - (a_position.y / (224.f/2.f));"
        "   v_depth = a_position.z;"
        "   v_texcoord = a_texcoord; \n"
        "} "
        ;

    const GLchar UI_ps[] =
        "precision highp float;									\n"
        "in highp vec2 v_texcoord;								\n"
        "in  highp float v_depth;\n"
        "uniform sampler2D s_texture;							\n"
        "out vec4 fragColor;									\n"
        "void main()											\n"
        "{														\n"
        "	vec4 txcol = texture(s_texture, v_texcoord);		\n"
        "   if(txcol.a <= 0.f) discard;\n"
        "   fragColor = txcol; \n"
        "   fragColor.w = 1.f;								\n"
        "   gl_FragDepth = v_depth;\n"
        "}														\n"
        ;

    static GLuint UIShaderIndex = 0;
    if (UIShaderIndex == 0)
    {
        UIShaderIndex = compileShader(UI_vs, UI_ps);
        assert(UIShaderIndex);
    }

    return UIShaderIndex;
}

GLuint GetWorldSpaceLineShader()
{
    static const GLchar UI_vs[] =
        "uniform mat4 u_projectionMatrix;    \n"
        "uniform mat4 u_viewMatrix;    \n"
        "in vec3 a_position;   \n"
        "in vec3 a_color;   \n"
        "out  highp vec3 v_color;     \n"
        "out  highp float v_depth;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = u_projectionMatrix * u_viewMatrix * vec4(a_position, 1); \n"
        "   v_color = a_color; \n"
        "} "
        ;

    const GLchar UI_ps[] =
        "precision highp float;									\n"
        "in highp vec3 v_color;								\n"
        "in  highp float v_depth;\n"
        "uniform sampler2D s_texture;							\n"
        "out vec4 fragColor;									\n"
        "void main()											\n"
        "{														\n"
        "   fragColor.xyz = v_color; \n"
        "   fragColor.w = 1;								\n"
        "   gl_FragDepth = v_depth;\n"
        "}														\n"
        ;

    static GLuint UIShaderIndex = 0;
    if (UIShaderIndex == 0)
    {
        UIShaderIndex = compileShader(UI_vs, UI_ps);
        assert(UIShaderIndex);
    }

    return UIShaderIndex;
}

GLuint GetLineShader()
{
    static const GLchar UI_vs[] =
        "in vec3 a_position;   \n"
        "in vec3 a_color;   \n"
        "out  highp vec3 v_color;     \n"
        "out  highp float v_depth;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vec4(a_position, 1); \n"
        "   gl_Position.x = (a_position.x / (352.f/2.f)) - 1.f;"
        "   gl_Position.y = 1.f - (a_position.y / (224.f/2.f));"
        "   v_depth = a_position.z;"
        "   v_color = a_color; \n"
        "} "
        ;

    const GLchar UI_ps[] =
        "precision highp float;									\n"
        "in highp vec3 v_color;								\n"
        "in  highp float v_depth;\n"
        "uniform sampler2D s_texture;							\n"
        "out vec4 fragColor;									\n"
        "out highp float gl_FragDepth ; \n"
        "void main()											\n"
        "{														\n"
        "   fragColor.xyz = v_color; \n"
        "   fragColor.w = 1;								\n"
        "   gl_FragDepth = v_depth;\n"
        "}														\n"
        ;

    static GLuint UIShaderIndex = 0;
    if (UIShaderIndex == 0)
    {
        UIShaderIndex = compileShader(UI_vs, UI_ps);
        assert(UIShaderIndex);
    }

    return UIShaderIndex;
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

        glUseProgram(Get2dUIShader());
        checkGL();

        float zNear = 0.1f;
        float zFar = 1000.f;

        s_quad tempQuad;
        tempQuad.model = NULL;
        tempQuad.CMDCTRL = CMDCTRL;
        tempQuad.CMDPMOD = CMDPMOD;
        tempQuad.CMDCOLR = CMDCOLR;
        tempQuad.CMDSRCA = CMDSRCA;
        tempQuad.CMDSIZE = CMDSIZE;

        GLuint textureId = getTextureForQuad(tempQuad);

        float quadDepth = 0.9;

        s_vd1ExtendedCommand* pExtendedCommand = fetchVdp1ExtendedCommand(vdp1EA);
        if (pExtendedCommand)
        {
            quadDepth = pExtendedCommand->depth;
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
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

        if (textureId > 0)
        {
            checkGL();
            //glEnable(GL_ALPHA_TEST);
            //glAlphaFunc(GL_GREATER, 0.1);
            glBindTexture(GL_TEXTURE_2D, textureId);
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

                checkGL();

                GLuint vertexp = glGetAttribLocation(shaderProgram, (const GLchar *)"a_position");
                GLuint texcoordp = glGetAttribLocation(shaderProgram, (const GLchar *)"a_texcoord");
                GLuint texture = glGetUniformLocation(shaderProgram, (const GLchar*)"s_texture");

                checkGL();

                static GLuint vao = 0;
                if (vao == 0)
                {
                    glGenVertexArrays(1, &vao);
                }
                glBindVertexArray(vao);

                static GLuint vbo = 0;
                if (vbo == 0)
                {
                    glGenBuffers(1, &vbo);
                }
                checkGL();
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                checkGL();
                glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertexData) * 4, &gVertexArray[0], GL_STATIC_DRAW);
                checkGL();
                glVertexAttribPointer(vertexp, 3, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->positions);
                checkGL();
                glEnableVertexAttribArray(vertexp);
                checkGL();
                if (texcoordp != -1)
                {
                    glVertexAttribPointer(texcoordp, 2, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->textures);
                    glEnableVertexAttribArray(texcoordp);
                }
                checkGL();
                if (texture != -1)
                {
                    glUniform1i(texture, 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, textureId);
                }
                checkGL();

                static GLuint indexBufferId = 0;
                if (indexBufferId == 0)
                {
                    char triVertexOrder[6] = { 0, 1, 2, 2, 3, 0 };
                    glGenBuffers(1, &indexBufferId);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triVertexOrder), triVertexOrder, GL_STATIC_DRAW);
                }
                else
                {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
                }

                checkGL();
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, triVertexOrder);
                //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*)0);
                checkGL();
                glDisableVertexAttribArray(vertexp);
                checkGL();
                if (texcoordp != -1)
                    glDisableVertexAttribArray(texcoordp);
                checkGL();
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                checkGL();

                if (texture != -1)
                {
                    glUniform1i(texture, 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
        }
    }

    checkGL();
    glUseProgram(0);
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

        glUseProgram(Get2dUIShader());
        checkGL();

        float zNear = 0.1f;
        float zFar = 1000.f;

        s_quad tempQuad;
        tempQuad.model = NULL;
        tempQuad.CMDCTRL = CMDCTRL;
        tempQuad.CMDPMOD = CMDPMOD;
        tempQuad.CMDCOLR = CMDCOLR;
        tempQuad.CMDSRCA = CMDSRCA;
        tempQuad.CMDSIZE = CMDSIZE;

        GLuint textureId = getTextureForQuad(tempQuad);

        float quadDepth = 0.9;

        s_vd1ExtendedCommand* pExtendedCommand = fetchVdp1ExtendedCommand(vdp1EA);
        if (pExtendedCommand)
        {
            quadDepth = pExtendedCommand->depth;
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
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

        if (textureId > 0)
        {
            checkGL();
            //glEnable(GL_ALPHA_TEST);
            //glAlphaFunc(GL_GREATER, 0.1);
            glBindTexture(GL_TEXTURE_2D, textureId);
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

                checkGL();

                GLuint vertexp = glGetAttribLocation(shaderProgram, (const GLchar *)"a_position");
                GLuint texcoordp = glGetAttribLocation(shaderProgram, (const GLchar *)"a_texcoord");
                GLuint texture = glGetUniformLocation(shaderProgram, (const GLchar*)"s_texture");

                checkGL();

                static GLuint vao = 0;
                if (vao == 0)
                {
                    glGenVertexArrays(1, &vao);
                }
                glBindVertexArray(vao);

                static GLuint vbo = 0;
                if (vbo == 0)
                {
                    glGenBuffers(1, &vbo);
                }
                checkGL();
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                checkGL();
                glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertexData) * 4, &gVertexArray[0], GL_STATIC_DRAW);
                checkGL();
                glVertexAttribPointer(vertexp, 3, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->positions);
                checkGL();
                glEnableVertexAttribArray(vertexp);
                checkGL();
                if (texcoordp != -1)
                {
                    glVertexAttribPointer(texcoordp, 2, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->textures);
                    glEnableVertexAttribArray(texcoordp);
                }
                checkGL();
                if (texture != -1)
                {
                    glUniform1i(texture, 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, textureId);
                }
                checkGL();

                static GLuint indexBufferId = 0;
                if (indexBufferId == 0)
                {
                    char triVertexOrder[6] = { 0, 1, 2, 2, 3, 0 };
                    glGenBuffers(1, &indexBufferId);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triVertexOrder), triVertexOrder, GL_STATIC_DRAW);
                }
                else
                {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
                }

                checkGL();
                //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, triVertexOrder);
                //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*)0);
                checkGL();
                glDisableVertexAttribArray(vertexp);
                checkGL();
                if (texcoordp != -1)
                    glDisableVertexAttribArray(texcoordp);
                checkGL();
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                checkGL();

                if (texture != -1)
                {
                    glUniform1i(texture, 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
        }
    }

    checkGL();
    glUseProgram(0);
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

    GLuint currentShader = GetWorldSpaceLineShader();
    glUseProgram(currentShader);
    checkGL();

    GLuint vertexp = glGetAttribLocation(currentShader, (const GLchar*)"a_position");
    GLuint colorp = glGetAttribLocation(currentShader, (const GLchar*)"a_color");

    checkGL();

    static GLuint vao = 0;
    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
    }
    glBindVertexArray(vao);

    static GLuint vbo = 0;
    if (vbo == 0)
    {
        glGenBuffers(1, &vbo);
    }
    checkGL();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    checkGL();
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertexData) * 4, &gVertexArray[0], GL_STATIC_DRAW);
    checkGL();
    glVertexAttribPointer(vertexp, 3, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*) & ((s_vertexData*)NULL)->positions);
    checkGL();
    glEnableVertexAttribArray(vertexp);
    checkGL();
    if (colorp != -1)
    {
        glVertexAttribPointer(colorp, 4, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*) & ((s_vertexData*)NULL)->color);
        glEnableVertexAttribArray(colorp);
    }
    checkGL();
    static GLuint indexBufferId = 0;
    if (indexBufferId == 0)
    {
        char triVertexOrder[6] = { 0, 1, 2, 2, 3, 0 };
        glGenBuffers(1, &indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triVertexOrder), triVertexOrder, GL_STATIC_DRAW);
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    }

    glDisable(GL_DEPTH_TEST);
    checkGL();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, triVertexOrder);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GLuint modelProjection = glGetUniformLocation(currentShader, (const GLchar*)"u_projectionMatrix");
    assert(modelProjection != -1);
    glUniformMatrix4fv(modelProjection, 1, GL_FALSE, getProjectionMatrix());

    GLuint viewMatrix = glGetUniformLocation(currentShader, (const GLchar*)"u_viewMatrix");
    assert(viewMatrix != -1);
    glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, getViewMatrix());

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, (void*)0);
    checkGL();
    glDisableVertexAttribArray(vertexp);
    checkGL();
    if (colorp != -1)
        glDisableVertexAttribArray(colorp);
    checkGL();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGL();

    checkGL();
    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);
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

    GLuint currentShader = GetWorldSpaceLineShader();
    glUseProgram(currentShader);
    checkGL();

    GLuint vertexp = glGetAttribLocation(currentShader, (const GLchar *)"a_position");
    GLuint colorp = glGetAttribLocation(currentShader, (const GLchar *)"a_color");

    checkGL();

    static GLuint vao = 0;
    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
    }
    glBindVertexArray(vao);

    static GLuint vbo = 0;
    if (vbo == 0)
    {
        glGenBuffers(1, &vbo);
    }
    checkGL();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    checkGL();
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertexData) * 2, &gVertexArray[0], GL_STATIC_DRAW);
    checkGL();
    glVertexAttribPointer(vertexp, 3, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->positions);
    checkGL();
    glEnableVertexAttribArray(vertexp);
    checkGL();
    if (colorp != -1)
    {
        glVertexAttribPointer(colorp, 4, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->color);
        glEnableVertexAttribArray(colorp);
    }
    checkGL();
    static GLuint indexBufferId = 0;
    if (indexBufferId == 0)
    {
        char triVertexOrder[2] = { 0, 1 };
        glGenBuffers(1, &indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triVertexOrder), triVertexOrder, GL_STATIC_DRAW);
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    }

    glDisable(GL_DEPTH_TEST);
    checkGL();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, triVertexOrder);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GLuint modelProjection = glGetUniformLocation(currentShader, (const GLchar *)"u_projectionMatrix");
    assert(modelProjection != -1);
    glUniformMatrix4fv(modelProjection, 1, GL_FALSE, getProjectionMatrix());

    GLuint viewMatrix = glGetUniformLocation(currentShader, (const GLchar *)"u_viewMatrix");
    assert(viewMatrix != -1);
    glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, getViewMatrix());

    glDrawElements(GL_LINES, 2, GL_UNSIGNED_BYTE, (void*)0);
    checkGL();
    glDisableVertexAttribArray(vertexp);
    checkGL();
    if (colorp != -1)
        glDisableVertexAttribArray(colorp);
    checkGL();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGL();

    checkGL();
    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);
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

    GLuint currentShader = GetLineShader();
    glUseProgram(currentShader);
    checkGL();

    checkGL();

    GLuint vertexp = glGetAttribLocation(currentShader, (const GLchar *)"a_position");
    GLuint colorp = glGetAttribLocation(currentShader, (const GLchar *)"a_color");

    checkGL();

    static GLuint vao = 0;
    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
    }
    glBindVertexArray(vao);

    static GLuint vbo = 0;
    if (vbo == 0)
    {
        glGenBuffers(1, &vbo);
    }
    checkGL();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    checkGL();
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertexData) * 2, &gVertexArray[0], GL_STATIC_DRAW);
    checkGL();
    glVertexAttribPointer(vertexp, 3, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->positions);
    checkGL();
    glEnableVertexAttribArray(vertexp);
    checkGL();
    if (colorp != -1)
    {
        glVertexAttribPointer(colorp, 4, GL_FLOAT, GL_FALSE, sizeof(s_vertexData), (void*)&((s_vertexData*)NULL)->color);
        glEnableVertexAttribArray(colorp);
    }
    checkGL();
    static GLuint indexBufferId = 0;
    if (indexBufferId == 0)
    {
        char triVertexOrder[2] = { 0, 1 };
        glGenBuffers(1, &indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triVertexOrder), triVertexOrder, GL_STATIC_DRAW);
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    }

    checkGL();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, triVertexOrder);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_BYTE, (void*)0);
    checkGL();
    glDisableVertexAttribArray(vertexp);
    checkGL();
    if (colorp != -1)
        glDisableVertexAttribArray(colorp);
    checkGL();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGL();

    checkGL();
    glUseProgram(0);
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
    
