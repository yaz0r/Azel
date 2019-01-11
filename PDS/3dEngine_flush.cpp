#include "PDS.h"

#include "3dEngine_textureCache.h"

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
    u8* m_pObject;
    u32 m_offset;
    sMatrix4x3 m_modelMatrix;
    s16 m_lightColor[3];
    float m_2dOffset[2];
    bool m_isBillboard;
};

std::vector<s_objectToRender> objectRenderList;

void addObjectToDrawList(u8* pObjectData, u32 offset)
{
    s_objectToRender newObject;
    newObject.m_pObject = pObjectData;
    newObject.m_offset = offset;
    newObject.m_modelMatrix = *pCurrentMatrix;
    newObject.m_2dOffset[0] = (graphicEngineStatus.m405C.localCoordinatesX - (352.f / 2.f)) / 352.f;
    newObject.m_2dOffset[1] = (graphicEngineStatus.m405C.localCoordinatesY - (224.f / 2.f)) / 224.f;
    newObject.m_isBillboard = 0;

    objectRenderList.push_back(newObject);
}

void addBillBoardToDrawList(u8* pObjectData, u32 offset)
{
    cameraProperties2.m88.matrix[3] = pCurrentMatrix->matrix[3];
    cameraProperties2.m88.matrix[7] = pCurrentMatrix->matrix[7];
    cameraProperties2.m88.matrix[11] = pCurrentMatrix->matrix[11];

    s_objectToRender newObject;
    newObject.m_pObject = pObjectData;
    newObject.m_offset = offset;
    newObject.m_modelMatrix = *pCurrentMatrix;
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
        assert(0);
    }
    
    //while (error != GL_NO_ERROR);
}

GLuint compileShader(const char* VS, const char* PS)
{
    GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, &VS, 0);
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
    glShaderSource(fragmentshader, 1, &PS, 0);
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
    assert(IsLinked);

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
"precision highp float;									\n"
"in highp vec2 v_texcoord;								\n"
"in highp vec4 v_color;								\n"
"uniform sampler2D s_texture;							\n"
"uniform float s_textureInfluence;						\n"
"uniform float s_ambientInfluence;						\n"
"out vec4 fragColor;									\n"
"void main()											\n"
"{														\n"
"   float distanceValue = mix(0.f, 10.f, 1.f-gl_FragCoord.z); \n"
"   //distanceValue = clamp(distanceValue, 0, 1); \n"
"	vec4 txcol = texture(s_texture, v_texcoord);		\n"
"   if(txcol.a <= 0.f) discard;\n"
"   fragColor = (clamp(txcol, 0.f, 1.f) * s_textureInfluence) + v_color;									\n"
"   fragColor = txcol; \n"
"   fragColor.w = 1.f;								\n"
"}														\n"
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
"	vec4 fallout = texture2D(s_falloff, vec2(distanceValue,0)); \n"
"	vec4 txcol = texture2D(s_texture, v_texcoord);		\n"
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

void ComputeColorFromNormal(u8* &pointsEA, bool withColor, s32* lightVectorModelSpace, s16* lightColor, float* falloutColor, float* perVertexColor, u8* &instanceDataEA)
{
    s16 fixedNormal[3];
    fixedNormal[0] = ((s16)READ_BE_U16(pointsEA)); pointsEA += 2;
    fixedNormal[1] = ((s16)READ_BE_U16(pointsEA)); pointsEA += 2;
    fixedNormal[2] = ((s16)READ_BE_U16(pointsEA)); pointsEA += 2;

    float fixedColor[3];
    fixedColor[0] = 0;
    fixedColor[1] = 0;
    fixedColor[2] = 0;

    if (withColor)
    {
        fixedColor[0] = ((s16)READ_BE_U16(pointsEA)); pointsEA += 2;
        fixedColor[1] = ((s16)READ_BE_U16(pointsEA)); pointsEA += 2;
        fixedColor[2] = ((s16)READ_BE_U16(pointsEA)); pointsEA += 2;
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

void drawObject_SingleDrawCall(s_objectToRender* pObject, float* projectionMatrix)
{
    checkGL();

    float quantisation = (float)0x10000;
    float objectMatrix[4 * 4];
    {

        memset(objectMatrix, 0, sizeof(objectMatrix));
        for (u32 i = 0; i < 4 * 3; i++)
        {
            objectMatrix[i] = pObject->m_modelMatrix.matrix[i] / quantisation;
        }

        objectMatrix[9] += 5000;

        transposeMatrix(objectMatrix);

        //glMatrixMode(GL_MODELVIEW);
        //glLoadMatrixf(objectMatrix);
    }

    float objectProjMatrix[4 * 4];
    multiplyMatrices(objectMatrix, projectionMatrix, objectProjMatrix);

    s32 lightVectorModelSpace[3] = { 0,0,0 };
    {
        /*
        lightVectorModelSpace[0] = (pObject->m_lightVector[0] * pObject->m_modelMatrix[0] + pObject->m_lightVector[1] * pObject->m_modelMatrix[4] + pObject->m_lightVector[2] * pObject->m_modelMatrix[8]) >> 16;
        lightVectorModelSpace[1] = (pObject->m_lightVector[0] * pObject->m_modelMatrix[1] + pObject->m_lightVector[1] * pObject->m_modelMatrix[5] + pObject->m_lightVector[2] * pObject->m_modelMatrix[9]) >> 16;
        lightVectorModelSpace[2] = (pObject->m_lightVector[0] * pObject->m_modelMatrix[2] + pObject->m_lightVector[1] * pObject->m_modelMatrix[6] + pObject->m_lightVector[2] * pObject->m_modelMatrix[10]) >> 16;
        */
    }

    u8* objectHeader = pObject->m_pObject + pObject->m_offset;

    {
        u32 currentBlockId = 0;
        u32 currentBlockOffset = 0;

        do
        {
            currentBlockOffset = 0;
            int blockType = 1;
            {
                if (blockType == 0xFF)
                    break;

                if ((blockType == 0) || (blockType == 1))
                {
                    //glColor3f(1, 0, 0);
                    //glDisable(GL_TEXTURE_2D);
                    //glDisable(GL_TEXTURE);
                    //glDisable(GL_DEPTH_TEST);

                    if (blockType == 0)
                        objectHeader = pObject->m_pObject + READ_BE_U32(objectHeader);

                    u32 headerUnk = READ_BE_U32(objectHeader + 0); // bounding size?
                    u32 numUniqueVertices = READ_BE_U32(objectHeader + 4); // r13
                    u8* verticesOffset = pObject->m_pObject + READ_BE_U32(objectHeader + 8); // r12
                    u8* startOfQuad = objectHeader + currentBlockOffset + 12;
                    u8* pointsEA = startOfQuad;
                    u32 instanceEA = 0;// pObject->m_instanceDataEA;

                    int pointIds[4];
                    int quadIndex = 0;

                    while (1)
                    {
                        startOfQuad = pointsEA;

                        pointIds[0] = READ_BE_U16(pointsEA + 0);
                        pointIds[1] = READ_BE_U16(pointsEA + 2);
                        pointIds[2] = READ_BE_U16(pointsEA + 4);
                        pointIds[3] = READ_BE_U16(pointsEA + 6);
                        pointsEA += 8;

                        bool breakOut = false;

                        if ((pointIds[0] == 0) && (pointIds[1] == 0) && (pointIds[2] == 0) && (pointIds[3] == 0))
                            breakOut = true;

                        for (u32 i = 0; i < 4; i++)
                        {
                            assert(pointIds[i] < numUniqueVertices);
                            if (pointIds[i] > 0x100)
                                breakOut = true;
                        }

                        if (breakOut)
                            break;

                        u16 lightingControl = READ_BE_U16(pointsEA); pointsEA += 2;

                        u16 CMDCTRL = READ_BE_U16(pointsEA); pointsEA += 2; // CMDCTRL (but modified)
                        u16 CMDPMOD = READ_BE_U16(pointsEA); pointsEA += 2; // CMDPMOD
                        u16 CMDCOLR = READ_BE_U16(pointsEA); pointsEA += 2; // CMDCOLR
                        u16 CMDSRCA = READ_BE_U16(pointsEA); pointsEA += 2; // CMDSRCA
                        u16 CMDSIZE = READ_BE_U16(pointsEA); pointsEA += 2; // CMDSIZEd

                        u8 lightingMode = (lightingControl >> 8) & 3;

                        {
                            s_quad tempQuad;
                            tempQuad.model = pObject->m_pObject;
                            tempQuad.CMDCTRL = CMDCTRL;
                            tempQuad.CMDPMOD = CMDPMOD;
                            tempQuad.CMDCOLR = CMDCOLR;
                            tempQuad.CMDSRCA = CMDSRCA;
                            tempQuad.CMDSIZE = CMDSIZE;

                            for (int i = 0; i < 4; i++)
                            {
                                u8* vertexEA = verticesOffset + 6 * pointIds[i];
                                float fX = ((s16)READ_BE_U16(vertexEA + 0)) / quantisation;
                                float fY = ((s16)READ_BE_U16(vertexEA + 2)) / quantisation;
                                float fZ = ((s16)READ_BE_U16(vertexEA + 4)) / quantisation;

                                tempQuad.m_vertices[i].m_originalVertices[0] = fX;
                                tempQuad.m_vertices[i].m_originalVertices[1] = fY;
                                tempQuad.m_vertices[i].m_originalVertices[2] = fZ;
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
                                GetDistanceFalloff(falloutColor[i], pObject, pointIds[i]);
                            }

                            u8* colorOverrideEA = 0;
                            if (instanceEA)
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
                                ComputeColorFromNormal(pointsEA, false, lightVectorModelSpace, pObject->m_lightColor, falloutColor[0], perVertexColor[0], colorOverrideEA);
                                pointsEA += 2; // 3 words + padding
                                break;
                            case 2: // texture + normal + color per vertex
                                for (int i = 0; i < 4; i++)
                                {
                                    ComputeColorFromNormal(pointsEA, true, lightVectorModelSpace, pObject->m_lightColor, falloutColor[i], perVertexColor[i], colorOverrideEA);
                                }
                                break;
                            case 3: // texture + normal per vertex
                                for (int i = 0; i < 4; i++)
                                {
                                    ComputeColorFromNormal(pointsEA, false, lightVectorModelSpace, pObject->m_lightColor, falloutColor[i], perVertexColor[i], colorOverrideEA);
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
                                //color[0] = MappedMemoryReadWord(quads[i].gouraudPointer + 0 * 2);

                                uv[1][0] = 1;
                                uv[1][1] = 0;
                                //color[1] = MappedMemoryReadWord(quads[i].gouraudPointer + 1 * 2);

                                uv[2][0] = 1;
                                uv[2][1] = 1;
                                //color[2] = MappedMemoryReadWord(quads[i].gouraudPointer + 2 * 2);

                                uv[3][0] = 0;
                                uv[3][1] = 1;
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

                                    char triVertexOrder[6] = { 0, 1, 2, 2, 3, 0 };
                                    for (int i = 0; i < 6; i++)
                                    {
                                        SingleDrawcallIndexArray.push_back(SingleDrawcallVertexArray.size() + triVertexOrder[i]);
                                    }

                                    for (int i = 0; i < 4; i++)
                                    {
                                        SingleDrawcallVertexArray.push_back(gVertexArray[i]);
                                    }
                                }
                            }
                        }
                        quadIndex++;
                    }
                }
            }
        } while (0);
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

    {
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_vertexData) * SingleDrawcallVertexArray.size(), &SingleDrawcallVertexArray[0], GL_STATIC_DRAW);
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
            glBindTexture(GL_TEXTURE_2D, 0);
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

        checkGL();
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, SingleDrawcallIndexArray.size(), GL_UNSIGNED_INT, &SingleDrawcallIndexArray[0]);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
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
}

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

    u8* objectHeader = pObject->m_pObject + pObject->m_offset;

    {
        u32 currentBlockId = 0;
        u32 currentBlockOffset = 0;

        do
        {
            currentBlockOffset = 0;
            int blockType = 1;
            {
                if (blockType == 0xFF)
                    break;

                if ((blockType == 0) || (blockType == 1))
                {
                    //glColor3f(1, 0, 0);
                    //glDisable(GL_TEXTURE_2D);
                    //glDisable(GL_TEXTURE);
                    //glDisable(GL_DEPTH_TEST);

                    if (blockType == 0)
                        objectHeader = pObject->m_pObject + READ_BE_U32(objectHeader);

                    u32 headerUnk = READ_BE_U32(objectHeader + 0); // bounding size?
                    u32 numUniqueVertices = READ_BE_U32(objectHeader + 4); // r13
                    u8* verticesOffset = pObject->m_pObject + READ_BE_U32(objectHeader + 8); // r12
                    u8* startOfQuad = objectHeader + currentBlockOffset + 12;
                    u8* pointsEA = startOfQuad;
                    u32 instanceEA = 0;// pObject->m_instanceDataEA;

                    int pointIds[4];
                    int quadIndex = 0;

                    while (1)
                    {
                        startOfQuad = pointsEA;

                        pointIds[0] = READ_BE_U16(pointsEA + 0);
                        pointIds[1] = READ_BE_U16(pointsEA + 2);
                        pointIds[2] = READ_BE_U16(pointsEA + 4);
                        pointIds[3] = READ_BE_U16(pointsEA + 6);
                        pointsEA += 8;

                        bool breakOut = false;

                        if ((pointIds[0] == 0) && (pointIds[1] == 0) && (pointIds[2] == 0) && (pointIds[3] == 0))
                            breakOut = true;

                        for (u32 i = 0; i < 4; i++)
                        {
                            assert(pointIds[i] < numUniqueVertices);
                            if (pointIds[i] > 0x100)
                                breakOut = true;
                        }

                        if (breakOut)
                            break;

                        u16 lightingControl = READ_BE_U16(pointsEA); pointsEA += 2;

                        u16 CMDCTRL = READ_BE_U16(pointsEA); pointsEA += 2; // CMDCTRL (but modified)
                        u16 CMDPMOD = READ_BE_U16(pointsEA); pointsEA += 2; // CMDPMOD
                        u16 CMDCOLR = READ_BE_U16(pointsEA); pointsEA += 2; // CMDCOLR
                        u16 CMDSRCA = READ_BE_U16(pointsEA); pointsEA += 2; // CMDSRCA
                        u16 CMDSIZE = READ_BE_U16(pointsEA); pointsEA += 2; // CMDSIZEd

                        u8 lightingMode = (lightingControl >> 8) & 3;

                        u32 indices[6] = { 0, 1, 2, 0, 2, 3 };

                        if (!fillPolys)
                        {
                            /*    glDisable(GL_TEXTURE_2D);
                            glDisable(GL_TEXTURE);
                            glColor4f(1, 0, 0, 1);
                            glBegin(GL_LINE_LOOP);
                            for (u32 vertexId = 0; vertexId < 4; vertexId++)
                            {
                            u32 vertexEA = verticesOffset + 6 * pointIds[vertexId];

                            float fX = ((s16)AzelLowWramMemoryReadWord(vertexEA + 0)) / quantisation;
                            float fY = ((s16)AzelLowWramMemoryReadWord(vertexEA + 2)) / quantisation;
                            float fZ = ((s16)AzelLowWramMemoryReadWord(vertexEA + 4)) / quantisation;

                            glVertex3f(fX, fY, fZ);

                            }
                            glEnd();
                            */
                        }
                        else
                        {
                            s_quad tempQuad;
                            tempQuad.model = pObject->m_pObject;
                            tempQuad.CMDCTRL = CMDCTRL;
                            tempQuad.CMDPMOD = CMDPMOD;
                            tempQuad.CMDCOLR = CMDCOLR;
                            tempQuad.CMDSRCA = CMDSRCA;
                            tempQuad.CMDSIZE = CMDSIZE;

                            for (int i = 0; i < 4; i++)
                            {
                                u8* vertexEA = verticesOffset + 6 * pointIds[i];
                                float fX = ((s16)READ_BE_U16(vertexEA + 0)) / quantisation;
                                float fY = ((s16)READ_BE_U16(vertexEA + 2)) / quantisation;
                                float fZ = ((s16)READ_BE_U16(vertexEA + 4)) / quantisation;

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
                                GetDistanceFalloff(falloutColor[i], pObject, pointIds[i]);
                            }

                            u8* colorOverrideEA = 0;
                            if (instanceEA)
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
                                ComputeColorFromNormal(pointsEA, false, lightVectorModelSpace, pObject->m_lightColor, falloutColor[0], perVertexColor[0], colorOverrideEA);
                                pointsEA += 2; // 3 words + padding
                                break;
                            case 2: // texture + normal + color per vertex
                                for (int i = 0; i < 4; i++)
                                {
                                    ComputeColorFromNormal(pointsEA, true, lightVectorModelSpace, pObject->m_lightColor, falloutColor[i], perVertexColor[i], colorOverrideEA);
                                }
                                break;
                            case 3: // texture + normal per vertex
                                for (int i = 0; i < 4; i++)
                                {
                                    ComputeColorFromNormal(pointsEA, false, lightVectorModelSpace, pObject->m_lightColor, falloutColor[i], perVertexColor[i], colorOverrideEA);
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
                        quadIndex++;
                    }
                }
            }
        } while (0);
    }
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

    float zNear = 0.1f;
    float zFar = 1000.f;

    s32 const1 = 229;
    s32 const2 = -195;

    float fEarlyProjectionMatrix[4 * 4];

    fEarlyProjectionMatrix[0] = const1 / (352.f / 2.f);
    fEarlyProjectionMatrix[1] = 0;
    fEarlyProjectionMatrix[2] = 0;
    fEarlyProjectionMatrix[3] = 0;

    fEarlyProjectionMatrix[4] = 0;
    fEarlyProjectionMatrix[5] = -const2 / (224.f / 2.f);
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

    ImGui::Begin("Objects");
    for(int i=0; i<objectRenderList.size(); i++)
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
    ImGui::End();

    checkGL();

    glUseProgram(shaderProgram);
    checkGL();

    for (int i = 0; i < objectRenderList.size(); i++)
    {
        SingleDrawcallVertexArray.clear();
        SingleDrawcallIndexArray.clear();
        drawObject(&objectRenderList[i], fEarlyProjectionMatrix);
    }

    checkGL();
    glUseProgram(0);

    objectRenderList.clear();
}
