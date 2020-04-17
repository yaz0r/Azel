#pragma once

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

#define PDS_TOOL

#ifndef _DEBUG
#define SHIPPING_BUILD
#endif

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#define NOMINMAX

#define TRACY_ENABLE
#define TRACY_CALLSTACK 20
#ifdef TRACY_ENABLE
#include "Tracy.hpp"
#endif

#include <SDL.h>
#include <SDL_syswm.h>

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
extern SoLoud::Soloud gSoloud;

#include <imgui.h>

#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <string>
#include <sstream>
#include <iostream>

#include <cmath>
#include <unordered_map>
#include <algorithm>

void checkGLImpl(const char*, unsigned int line);
//#define checkGL() checkGLImpl(__FILE__, __LINE__);
#define checkGL()

#if (defined(__ANDROID__)) || (defined(__EMSCRIPTEN__))
    #define USE_GL_ES3
    #define WITH_GL
#else
    #if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
        #define USE_GL_ES3
        #define GLES_SILENCE_DEPRECATION
        #define WITH_GL
        #define WITH_VK
    #elif (defined(__APPLE__) && TARGET_OS_WATCH)
        #define USE_NULL_RENDERER
    #else
        #define USE_GL
        #define WITH_VK
        #define WITH_GL
    #endif
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

#include "PDS_Logger.h"

struct sSaturnPtr
{
    s32 m_offset;
    struct sSaturnMemoryFile* m_file;

    sSaturnPtr operator + (unsigned int i) const
    {
        sSaturnPtr newPtr = *this;
        newPtr.m_offset += i;
        return newPtr;
    }

    sSaturnPtr operator + (int i) const
    {
        sSaturnPtr newPtr = *this;
        newPtr.m_offset += i;
        return newPtr;
    }

    sSaturnPtr& operator ++ ()
    {
        m_offset++;
        return *this;
    }

    sSaturnPtr operator ++ (int)
    {
        sSaturnPtr result(*this);
        ++(*this);
        return result;
    }

    sSaturnPtr& operator -- ()
    {
        m_offset--;
        return *this;
    }

    sSaturnPtr operator -- (int)
    {
        sSaturnPtr result(*this);
        --(*this);
        return result;
    }


    sSaturnPtr& operator += (unsigned int i)
    {
        this->m_offset += i;
        return *this;
    }
    sSaturnPtr operator - (unsigned int i) const
    {
        sSaturnPtr newPtr = *this;
        newPtr.m_offset -= i;
        return newPtr;
    }
    sSaturnPtr& operator -= (unsigned int i)
    {
        this->m_offset -= i;
        return *this;
    }

    bool operator ==(const sSaturnPtr& b) const
    {
        if (b.m_file != m_file)
            return false;
        if (b.m_offset != m_offset)
            return false;
        return true;
    }

    bool operator !=(const sSaturnPtr& b) const
    {
        if ((b.m_file != m_file) || (b.m_offset != m_offset))
            return true;
        return false;
    }

    bool isNull()
    {
        if (m_offset == 0)
            return true;
        return false;
    }

    static sSaturnPtr& getNull()
    {
        static sSaturnPtr temp;
        temp.m_offset = 0;
        temp.m_file = NULL;
        return temp;
    }

    static sSaturnPtr& createFromRaw(u32 offset, struct sSaturnMemoryFile* pFile = nullptr)
    {
        static sSaturnPtr temp;
        temp.m_offset = offset;
        temp.m_file = pFile;
        return temp;
    }

    u8* getRawPointer();
};

struct sSaturnMemoryFile
{
    std::string m_name;
    u8* m_data;
    u32 m_dataSize;
    u32 m_base;

    sSaturnPtr getSaturnPtr(u32 base)
    {
        sSaturnPtr newPtr;
        newPtr.m_file = this;
        newPtr.m_offset = base;
        return newPtr;
    }

    virtual void init() {}
};

bool findFileOnDisc(const std::string& filename);

#include "fixedPoint.h"

#include "dummy.h"

#include "heap.h"
#include "task.h"
#include "rootTask.h"

#include "VDP1.h"
#include "VDP2.h"
#include "titleScreen.h"

#include "common.h"
#include "renderer.h"
#include "3dEngine.h"
#include "3dEngine_flush.h"
#include "mainMenuDebugTasks.h"
#include "field.h"
#include "LCS.h"

#include "kernel/receiveItemTask.h"

#include "menu_dragon.h"
#include "menu_dragonMorph.h"

#include "3dModels.h"

extern u8 COMMON_DAT[0x98000];

class sCommonOverlay_data : public sSaturnMemoryFile
{
public:
    void init();
    
    struct sDragonLevelStat
    {
        std::array<s8, 3> m0;
        std::array<s8, 3> m3;
        std::array<s8, 3> m6;
        std::array<s8, 3> m9;
        std::array<s8, 3> mC;
        std::array<s8, 3> mF;
        std::array<s8, 3> m12;
        std::array<s8, 3> m15;
        std::array<s8, 3> m18;
        std::array<s8, 3> m1B;
    };
    std::vector<sDragonLevelStat> dragonLevelStats;
};

extern sCommonOverlay_data gCommonFile;

struct sFileInfoSub
{
    sFileInfoSub* pNext; // 0
    char m_fileName[32]; // 4 was file id before
    FILE* fHandle;//8
                  //u32 m_numSectors;//C
    u32 m_fileSize; //10 ?
                    //u32 m_14;//14 ?
    u16 m_18;//18 ? word
    u32 m1A_vdp1Data; //1A ? word
};

struct sFileInfo
{
    u8 m0; //0
    u8 m3; //3
    u8 displayMemoryLayout; //5
    u16 m8; //8

    sFileInfoSub* m2C_allocatedHead; //2C
    sFileInfoSub* freeHead; //30
    sFileInfoSub linkedList[15]; //34
};

extern sFileInfo fileInfoStruct;

extern bool debugEnabled;
extern int enableDebugTask;
extern u8 pauseEngine[8];
extern u32 azelCdNumber;

void WRITE_BE_U16(const void* ptr, u16 value);
void WRITE_BE_U32(const void* ptr, u32 value);

sVec3_FP READ_BE_Vec3(const void* ptr);

u32 READ_BE_U32(const void* ptr);
s32 READ_BE_S32(const void* ptr);
u16 READ_BE_U16(const void* ptr);
s16 READ_BE_S16(const void* ptr);
u8 READ_BE_U8(const void* ptr);
s8 READ_BE_S8(const void* ptr);

u32 getFileSize(const char* fileName);

//void unimplemented(const char* name);

void initVDP1Projection(fixedPoint r4, u32 mode);
void getVdp1ProjectionParams(s16* r4, s16* r5);

s32 setDividend(s32 r4, s32 r5, s32 divisor);

fixedPoint sqrt_F(fixedPoint r4);
s32 sqrt_I(s32 r4);
void initFileLayoutTable();

extern bool hasEncounterData;

#include "o_title.h"
#include "o_menuEn.h"
#include "field/field_a3/o_fld_a3.h"

// Script macro stuff

#define WRITE_SCRIPT_U16(value) value & 0xFF, (value >> 8) & 0xFF
#define WRITE_SCRIPT_U32(value) value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF
#if 0
#define WRITE_SCRIPT_POINTER(stringPtr) (u64)stringPtr & 0xFF, ((u64)stringPtr >> 8) & 0xFF, ((u64)stringPtr >> 16) & 0xFF, ((u64)stringPtr >> 24) & 0xFF, ((u64)stringPtr >> 32) & 0xFF, ((u64)stringPtr >> 40) & 0xFF, ((u64)stringPtr >> 48) & 0xFF, ((u64)stringPtr >> 56) & 0xFF
#else 
#define WRITE_SCRIPT_POINTER(stringPtr) WRITE_SCRIPT_U32(0)
#endif

#define op_END() 0x1
#define op_CALL_NATIVE_0(funcPtr) 0xE, 0x0, WRITE_SCRIPT_POINTER(funcPtr)
#define op_CALL_NATIVE_1(funcPtr, arg0) 0xE, 0x1, WRITE_SCRIPT_POINTER(funcPtr), WRITE_SCRIPT_U32(arg0)
#define op_START_CUTSCENE() 0x18
#define op_WAIT(delay) 0x02, WRITE_SCRIPT_U16(delay)
#define op_DISPLAY_DIALOG_STRING(stringPtr) 0x15, WRITE_SCRIPT_POINTER(stringPtr)
#define op_CLEAR_DIALOG_STRING() 0x16
#define op_ADD_CINEMATIC_BARS() 0x19
#define op_END_CUTSCENE() 0x1A
#define op_PLAY_PCM(stringPtr) 0x22, WRITE_SCRIPT_POINTER(stringPtr)

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

extern p_workArea(*gFieldOverlayFunction)(p_workArea workArea, u32 arg);
fixedPoint distanceSquareBetween2Points(const sVec3_FP& r4_vertice0, const sVec3_FP& r5_vertice1);

void RendererSetFov(float fovInDegree);
s32 MTH_Product2d(s32(&r4)[2], s32(&r5)[2]);
fixedPoint MulVec2(const sVec2_FP& r4, const sVec2_FP& r5);
void adjustMatrixTranslation(fixedPoint r4);

struct sF0Color
{
    std::array<u16, 4> m0_colors;
};

