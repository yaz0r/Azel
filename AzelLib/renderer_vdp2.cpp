#include "PDS.h"
#include "renderer_vdp2.h"
#include "renderer/renderer.h"
#include "battle/BTL_A3/BTL_A3_map6.h"

extern backend* gBackend;
extern bool useCombinedDualPlane;
extern bool debugFlatPlaneA;

bgfx::ProgramHandle loadBgfxProgram(const std::string& VSFile, const std::string& PSFile);

std::array<s_NBG_data, 6> NBG_data;

bgfx::TextureHandle bgfx_vdp2_ram_texture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle bgfx_vdp2_cram_texture = BGFX_INVALID_HANDLE;

bgfx::ProgramHandle bgfx_vdp2_program = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle bgfx_vdp2_rbg0_program = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle bgfx_vdp2_rbg0_dual_program = BGFX_INVALID_HANDLE;

struct s_layerData
{
    s_layerData()
    {
        lineScrollEA = 0;
        mapwh = 0;
        memset(planeOffsets, 0, sizeof(planeOffsets));
    }

    u32 CHSZ;
    u32 CHCN;
    u32 PNB;
    u32 CNSM;
    u32 CAOS;
    u32 PLSZ;
    u32 SCN;

    u32 planeOffsets[16];
    u32 mapwh; // 0 = use PLSZ-based addressing (NBG), 4 = 4x4 rotation scroll (RBG0)

    s32 scrollX;
    s32 scrollY;

    s32 lineScrollEA;
};

// Packed struct matching the VDP2_ps.sc shader layout (14 u32s)
struct s_layerDataGPU
{
    u32 CHSZ;           // 0
    u32 CHCN;           // 1
    u32 PNB;            // 2
    u32 CNSM;           // 3
    u32 CAOS;           // 4
    u32 PLSZ;           // 5
    u32 SCN;            // 6
    u32 planeOffsets[4]; // 7-10
    s32 scrollX;        // 11
    s32 scrollY;        // 12
    s32 lineScrollEA;   // 13
};

static s_layerDataGPU packForGPU(const s_layerData& ld)
{
    s_layerDataGPU g;
    g.CHSZ = ld.CHSZ;
    g.CHCN = ld.CHCN;
    g.PNB = ld.PNB;
    g.CNSM = ld.CNSM;
    g.CAOS = ld.CAOS;
    g.PLSZ = ld.PLSZ;
    g.SCN = ld.SCN;
    g.planeOffsets[0] = ld.planeOffsets[0];
    g.planeOffsets[1] = ld.planeOffsets[1];
    g.planeOffsets[2] = ld.planeOffsets[2];
    g.planeOffsets[3] = ld.planeOffsets[3];
    g.scrollX = ld.scrollX;
    g.scrollY = ld.scrollY;
    g.lineScrollEA = ld.lineScrollEA;
    return g;
}

void renderLayerGPU(s_layerData& layerData, u32 textureWidth, u32 textureHeight, s_NBG_data& NBGData)
{
    // BGFX update texture size if needed
    if ((NBGData.m_currentWidth != textureWidth) || (NBGData.m_currentHeight != textureHeight))
    {
        if (isValid(NBGData.BGFXFB))
        {
            bgfx::destroy(NBGData.BGFXFB);
        }

        if (isValid(NBGData.BGFXTexture))
        {
            bgfx::destroy(NBGData.BGFXTexture);
        }


        const uint64_t tsFlags = 0
            //| BGFX_SAMPLER_MIN_POINT
            //| BGFX_SAMPLER_MAG_POINT
            //| BGFX_SAMPLER_MIP_POINT
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            ;

        NBGData.BGFXFB = bgfx::createFrameBuffer(textureWidth, textureHeight, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
        NBGData.BGFXTexture = bgfx::getTexture(NBGData.BGFXFB);

        NBGData.bgfx_vdp2_planeDataBuffer = bgfx::createTexture2D(64, 1, 0, 0, bgfx::TextureFormat::R32U);

        NBGData.m_currentWidth = textureWidth;
        NBGData.m_currentHeight = textureHeight;
    }

    bgfx::setViewFrameBuffer(NBGData.viewId, NBGData.BGFXFB);
    {
        // BGFX version
        static bgfx::VertexBufferHandle quad_vertexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::IndexBufferHandle quad_indexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_RAM = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_CRAM = BGFX_INVALID_HANDLE;
        static bgfx::VertexLayout ms_layout;
        static bgfx::UniformHandle planeDataBuffer = BGFX_INVALID_HANDLE;

        static bool initialized = false;
        if (!initialized)
        {
            ms_layout
                .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .end();

            static const float g_quad_vertex_buffer_data[] = {
                -1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                1.0f,  1.0f, 0.0f,
            };

            static const short int g_quad_index_buffer_data[] = {
                0,1,2,3,4,5
            };

            quad_vertexbuffer = bgfx::createVertexBuffer(bgfx::copy(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data)), ms_layout);
            quad_indexbuffer = bgfx::createIndexBuffer(bgfx::copy(g_quad_index_buffer_data, sizeof(g_quad_index_buffer_data)));

            texID_VDP2_RAM = bgfx::createUniform("s_VDP2_RAM", bgfx::UniformType::Sampler);
            texID_VDP2_CRAM = bgfx::createUniform("s_VDP2_CRAM", bgfx::UniformType::Sampler);
            planeDataBuffer = bgfx::createUniform("s_planeConfig", bgfx::UniformType::Sampler);

            initialized = true;
        }

        bgfx::setState(0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
        );

        bgfx::setViewRect(NBGData.viewId, 0, 0, textureWidth, textureHeight);
        bgfx::setViewClear(NBGData.viewId, BGFX_CLEAR_COLOR);

        s_layerDataGPU gpuData = packForGPU(layerData);
        bgfx::updateTexture2D(NBGData.bgfx_vdp2_planeDataBuffer, 0, 0, 0, 0, sizeof(gpuData)/4, 1, bgfx::copy(&gpuData, sizeof(gpuData)));

        bgfx::setTexture(0, texID_VDP2_RAM, bgfx_vdp2_ram_texture);
        bgfx::setTexture(1, texID_VDP2_CRAM, bgfx_vdp2_cram_texture);
        bgfx::setTexture(2, planeDataBuffer, NBGData.bgfx_vdp2_planeDataBuffer);

        bgfx::setVertexBuffer(0, quad_vertexbuffer);
        bgfx::setIndexBuffer(quad_indexbuffer);
        bgfx::submit(NBGData.viewId, bgfx_vdp2_program);

    }

    gBackend->bindBackBuffer();
}

struct s_layerDataRBG0
{
    // Base layer params (indices 0-13, same layout as s_layerData)
    u32 CHSZ;       // 0
    u32 CHCN;       // 1
    u32 PNB;        // 2
    u32 CNSM;       // 3
    u32 CAOS;       // 4
    u32 PLSZ;       // 5
    u32 SCN;        // 6
    u32 planeOffsets[4]; // 7-10
    s32 scrollX;    // 11
    s32 scrollY;    // 12
    s32 outputHeight; // 13

    // Rotation parameters (indices 14-25, 16.16 fixed-point)
    s32 DXx;        // 14
    s32 DXy;        // 15
    s32 DYx;        // 16
    s32 DYy;        // 17
    s32 Xst;        // 18
    s32 Yst;        // 19
    s32 Mx;         // 20
    s32 My;         // 21
    s32 Cx;         // 22
    s32 Cy;         // 23
    s32 coeffCount; // 24
    s32 scrollMode; // 25

    // Rotation matrix elements (indices 26-31, for full matrix mode)
    s32 A;          // 26 (t.m1C)
    s32 B;          // 27 (t.m20)
    s32 D;          // 28 (t.m28)
    s32 E;          // 29 (t.m2C)
    s32 Px;         // 30 (t.m34, pivot X)
    s32 Py;         // 31 (t.m36, pivot Y)
};

void renderLayerGPU_RBG0(s_layerDataRBG0& layerData, u32 textureWidth, u32 textureHeight,
    s_NBG_data& NBGData, const std::vector<fixedPoint>& coefficients, bool clearView = true, bool alphaBlend = false)
{
    // BGFX update texture size if needed
    if ((NBGData.m_currentWidth != textureWidth) || (NBGData.m_currentHeight != textureHeight))
    {
        if (isValid(NBGData.BGFXFB))
        {
            bgfx::destroy(NBGData.BGFXFB);
        }

        if (isValid(NBGData.BGFXTexture))
        {
            bgfx::destroy(NBGData.BGFXTexture);
        }

        const uint64_t tsFlags = 0
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            ;

        NBGData.BGFXFB = bgfx::createFrameBuffer(textureWidth, textureHeight, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
        NBGData.BGFXTexture = bgfx::getTexture(NBGData.BGFXFB);

        NBGData.bgfx_vdp2_planeDataBuffer = bgfx::createTexture2D(64, 1, 0, 0, bgfx::TextureFormat::R32U);

        NBGData.m_currentWidth = textureWidth;
        NBGData.m_currentHeight = textureHeight;
    }

    bgfx::setViewFrameBuffer(NBGData.viewId, NBGData.BGFXFB);
    {
        static bgfx::VertexBufferHandle quad_vertexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::IndexBufferHandle quad_indexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_RAM = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_CRAM = BGFX_INVALID_HANDLE;
        static bgfx::VertexLayout ms_layout;
        static bgfx::UniformHandle planeDataBuffer = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle coeffTableUniform = BGFX_INVALID_HANDLE;

        static bool initialized = false;
        if (!initialized)
        {
            ms_layout
                .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .end();

            static const float g_quad_vertex_buffer_data[] = {
                -1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                1.0f,  1.0f, 0.0f,
            };

            static const short int g_quad_index_buffer_data[] = {
                0,1,2,3,4,5
            };

            quad_vertexbuffer = bgfx::createVertexBuffer(bgfx::copy(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data)), ms_layout);
            quad_indexbuffer = bgfx::createIndexBuffer(bgfx::copy(g_quad_index_buffer_data, sizeof(g_quad_index_buffer_data)));

            texID_VDP2_RAM = bgfx::createUniform("s_VDP2_RAM", bgfx::UniformType::Sampler);
            texID_VDP2_CRAM = bgfx::createUniform("s_VDP2_CRAM", bgfx::UniformType::Sampler);
            planeDataBuffer = bgfx::createUniform("s_planeConfig", bgfx::UniformType::Sampler);
            coeffTableUniform = bgfx::createUniform("s_coefficientTable", bgfx::UniformType::Sampler);

            initialized = true;
        }

        // Upload extended config (base + rotation params)
        bgfx::updateTexture2D(NBGData.bgfx_vdp2_planeDataBuffer, 0, 0, 0, 0, sizeof(layerData) / 4, 1, bgfx::copy(&layerData, sizeof(layerData)));

        // Create and upload coefficient texture (R32I, 256-wide)
        int coeffCount = layerData.coeffCount;
        int texWidth = 256;
        int texHeight = (coeffCount + texWidth - 1) / texWidth;
        if (texHeight < 1) texHeight = 1;

        // Pack coefficients into u32 array (reinterpreted as signed in shader)
        std::vector<u32> coeffData(texWidth * texHeight, 0);
        for (int i = 0; i < coeffCount && i < (int)coefficients.size(); i++) {
            coeffData[i] = (u32)(s32)coefficients[i];
        }

        bgfx::TextureHandle coeffTex = bgfx::createTexture2D(
            texWidth, texHeight, false, 1, bgfx::TextureFormat::R32U,
            BGFX_TEXTURE_NONE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
            bgfx::copy(coeffData.data(), texWidth * texHeight * sizeof(u32))
        );

        uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
        if (alphaBlend) {
            state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
        }
        bgfx::setState(state);

        bgfx::setViewRect(NBGData.viewId, 0, 0, textureWidth, textureHeight);
        if (clearView) {
            bgfx::setViewClear(NBGData.viewId, BGFX_CLEAR_COLOR);
        } else {
            bgfx::setViewClear(NBGData.viewId, BGFX_CLEAR_NONE);
        }

        bgfx::setTexture(0, texID_VDP2_RAM, bgfx_vdp2_ram_texture);
        bgfx::setTexture(1, texID_VDP2_CRAM, bgfx_vdp2_cram_texture);
        bgfx::setTexture(2, planeDataBuffer, NBGData.bgfx_vdp2_planeDataBuffer);
        bgfx::setTexture(3, coeffTableUniform, coeffTex);

        bgfx::setVertexBuffer(0, quad_vertexbuffer);
        bgfx::setIndexBuffer(quad_indexbuffer);
        bgfx::submit(NBGData.viewId, bgfx_vdp2_rbg0_program);

        // Destroy the transient coefficient texture after submit
        bgfx::destroy(coeffTex);
    }

    gBackend->bindBackBuffer();
}

// Combined dual-plane struct: plane A (0-25) + plane B (26-46)
struct s_layerDataRBG0_dual
{
    // Plane A (indices 0-25, identical to s_layerDataRBG0)
    u32 CHSZ;       // 0
    u32 CHCN;       // 1
    u32 PNB;        // 2
    u32 CNSM;       // 3
    u32 CAOS;       // 4
    u32 PLSZ;       // 5
    u32 SCN;        // 6
    u32 planeOffsets[4]; // 7-10
    s32 scrollX;    // 11
    s32 scrollY;    // 12
    s32 outputHeight; // 13
    s32 DXx;        // 14
    s32 DXy;        // 15
    s32 DYx;        // 16
    s32 DYy;        // 17
    s32 Xst;        // 18
    s32 Yst;        // 19
    s32 Mx;         // 20
    s32 My;         // 21
    s32 Cx;         // 22
    s32 Cy;         // 23
    s32 coeffCount; // 24
    s32 scrollMode; // 25

    // Plane B (indices 26-52)
    u32 B_CHSZ;     // 26
    u32 B_CHCN;     // 27
    u32 B_PNB;      // 28
    u32 B_CNSM;     // 29
    u32 B_CAOS;     // 30
    u32 B_PLSZ;     // 31
    u32 B_SCN;      // 32
    u32 B_planeOffsets[4]; // 33-36
    s32 B_DXx;      // 37
    s32 B_DXy;      // 38
    s32 B_DYx;      // 39
    s32 B_DYy;      // 40
    s32 B_Xst;      // 41
    s32 B_Yst;      // 42
    s32 B_Mx;       // 43
    s32 B_My;       // 44
    s32 B_Cx;       // 45
    s32 B_Cy;       // 46
    // Rotation matrix elements (16.16 fixed-point)
    s32 B_A;        // 47 (t.m1C)
    s32 B_B;        // 48 (t.m20)
    s32 B_D;        // 49 (t.m28)
    s32 B_E;        // 50 (t.m2C)
    s32 B_Px;       // 51 (t.m34, pivot X)
    s32 B_Py;       // 52 (t.m36, pivot Y)

    // Plane A rotation matrix (indices 53-58)
    s32 A_A;        // 53 (t.m1C)
    s32 A_B;        // 54 (t.m20)
    s32 A_D;        // 55 (t.m28)
    s32 A_E;        // 56 (t.m2C)
    s32 A_Px;       // 57 (t.m34, pivot X)
    s32 A_Py;       // 58 (t.m36, pivot Y)
};

void renderLayerGPU_RBG0_dual(s_layerDataRBG0_dual& layerData, u32 textureWidth, u32 textureHeight,
    s_NBG_data& NBGData, const std::vector<fixedPoint>& coefficients)
{
    // BGFX update texture size if needed
    if ((NBGData.m_currentWidth != textureWidth) || (NBGData.m_currentHeight != textureHeight))
    {
        if (isValid(NBGData.BGFXFB))
        {
            bgfx::destroy(NBGData.BGFXFB);
        }

        if (isValid(NBGData.BGFXTexture))
        {
            bgfx::destroy(NBGData.BGFXTexture);
        }

        const uint64_t tsFlags = 0
            | BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            ;

        NBGData.BGFXFB = bgfx::createFrameBuffer(textureWidth, textureHeight, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | tsFlags);
        NBGData.BGFXTexture = bgfx::getTexture(NBGData.BGFXFB);

        // 47 u32s needs 64-wide texture
        NBGData.bgfx_vdp2_planeDataBuffer = bgfx::createTexture2D(64, 1, 0, 0, bgfx::TextureFormat::R32U);

        NBGData.m_currentWidth = textureWidth;
        NBGData.m_currentHeight = textureHeight;
    }

    bgfx::setViewFrameBuffer(NBGData.viewId, NBGData.BGFXFB);
    {
        static bgfx::VertexBufferHandle quad_vertexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::IndexBufferHandle quad_indexbuffer = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_RAM = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle texID_VDP2_CRAM = BGFX_INVALID_HANDLE;
        static bgfx::VertexLayout ms_layout;
        static bgfx::UniformHandle planeDataBuffer = BGFX_INVALID_HANDLE;
        static bgfx::UniformHandle coeffTableUniform = BGFX_INVALID_HANDLE;

        static bool initialized = false;
        if (!initialized)
        {
            ms_layout
                .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .end();

            static const float g_quad_vertex_buffer_data[] = {
                -1.0f, -1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f,
                1.0f, -1.0f, 0.0f,
                1.0f,  1.0f, 0.0f,
            };

            static const short int g_quad_index_buffer_data[] = {
                0,1,2,3,4,5
            };

            quad_vertexbuffer = bgfx::createVertexBuffer(bgfx::copy(g_quad_vertex_buffer_data, sizeof(g_quad_vertex_buffer_data)), ms_layout);
            quad_indexbuffer = bgfx::createIndexBuffer(bgfx::copy(g_quad_index_buffer_data, sizeof(g_quad_index_buffer_data)));

            texID_VDP2_RAM = bgfx::createUniform("s_VDP2_RAM", bgfx::UniformType::Sampler);
            texID_VDP2_CRAM = bgfx::createUniform("s_VDP2_CRAM", bgfx::UniformType::Sampler);
            planeDataBuffer = bgfx::createUniform("s_planeConfig", bgfx::UniformType::Sampler);
            coeffTableUniform = bgfx::createUniform("s_coefficientTable", bgfx::UniformType::Sampler);

            initialized = true;
        }

        // Upload extended config (plane A + plane B params)
        bgfx::updateTexture2D(NBGData.bgfx_vdp2_planeDataBuffer, 0, 0, 0, 0, sizeof(layerData) / 4, 1, bgfx::copy(&layerData, sizeof(layerData)));

        // Create and upload coefficient texture (R32U, 256-wide)
        int coeffCount = layerData.coeffCount;
        int texWidth = 256;
        int texHeight = (coeffCount + texWidth - 1) / texWidth;
        if (texHeight < 1) texHeight = 1;

        std::vector<u32> coeffData(texWidth * texHeight, 0);
        for (int i = 0; i < coeffCount && i < (int)coefficients.size(); i++) {
            coeffData[i] = (u32)(s32)coefficients[i];
        }

        bgfx::TextureHandle coeffTex = bgfx::createTexture2D(
            texWidth, texHeight, false, 1, bgfx::TextureFormat::R32U,
            BGFX_TEXTURE_NONE | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
            bgfx::copy(coeffData.data(), texWidth * texHeight * sizeof(u32))
        );

        uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;
        bgfx::setState(state);

        bgfx::setViewRect(NBGData.viewId, 0, 0, textureWidth, textureHeight);
        bgfx::setViewClear(NBGData.viewId, BGFX_CLEAR_COLOR);

        bgfx::setTexture(0, texID_VDP2_RAM, bgfx_vdp2_ram_texture);
        bgfx::setTexture(1, texID_VDP2_CRAM, bgfx_vdp2_cram_texture);
        bgfx::setTexture(2, planeDataBuffer, NBGData.bgfx_vdp2_planeDataBuffer);
        bgfx::setTexture(3, coeffTableUniform, coeffTex);

        bgfx::setVertexBuffer(0, quad_vertexbuffer);
        bgfx::setIndexBuffer(quad_indexbuffer);
        bgfx::submit(NBGData.viewId, bgfx_vdp2_rbg0_dual_program);

        bgfx::destroy(coeffTex);
    }

    gBackend->bindBackBuffer();
}

struct s_tileDimensions
{
    u32 cellDotDimension;
    u32 characterPatternDotDimension;
    u32 pageDotDimension;
    u32 planeDotWidth;
    u32 planeDotHeight;
    u32 pageDimension;
    u32 patternSize;
    u32 pageSize;
    u32 cellSizeInByte;
};

static s_tileDimensions computeTileDimensions(const s_layerData& layerData)
{
    s_tileDimensions d;
    d.cellDotDimension = 8;
    d.characterPatternDotDimension = d.cellDotDimension * ((layerData.CHSZ == 0) ? 1 : 2);
    d.pageDotDimension = 8 * 64;
    d.planeDotWidth = d.pageDotDimension * ((layerData.PLSZ & 1) ? 2 : 1);
    d.planeDotHeight = d.pageDotDimension * ((layerData.PLSZ & 2) ? 2 : 1);
    d.pageDimension = (layerData.CHSZ == 0) ? 64 : 32;
    d.patternSize = (layerData.PNB == 0) ? 4 : 2;
    d.pageSize = d.pageDimension * d.pageDimension * d.patternSize;
    d.cellSizeInByte = 8 * 8;
    switch (layerData.CHCN)
    {
    case 0: d.cellSizeInByte /= 2; break;
    case 1: d.cellSizeInByte *= 1; break;
    }
    return d;
}

static u32 sampleTileAtCoordinate(u32 outputX, u32 outputY, const s_layerData& layerData, const s_tileDimensions& d)
{
    u32 planeX, planeY;
    if (layerData.mapwh > 0)
    {
        // RBG0: mapwh x mapwh grid (e.g. 4x4 = 16 planes)
        planeX = (outputX / d.planeDotWidth) % layerData.mapwh;
        planeY = (outputY / d.planeDotHeight) % layerData.mapwh;
    }
    else
    {
        // NBG: PLSZ-based (1x1, 2x1, 1x2, 2x2)
        planeX = (outputX / d.planeDotWidth) % ((layerData.PLSZ & 1) ? 2 : 1);
        planeY = (outputY / d.planeDotHeight) % ((layerData.PLSZ & 2) ? 2 : 1);
    }
    u32 dotInPlaneX = outputX % d.planeDotWidth;
    u32 dotInPlaneY = outputY % d.planeDotHeight;

    u32 pageX = dotInPlaneX / d.pageDotDimension;
    u32 pageY = dotInPlaneY / d.pageDotDimension;
    u32 dotInPageX = dotInPlaneX % d.pageDotDimension;
    u32 dotInPageY = dotInPlaneY % d.pageDotDimension;

    u32 characterPatternX = dotInPageX / d.characterPatternDotDimension;
    u32 characterPatternY = dotInPageY / d.characterPatternDotDimension;
    u32 dotInCharacterPatternX = dotInPageX % d.characterPatternDotDimension;
    u32 dotInCharacterPatternY = dotInPageY % d.characterPatternDotDimension;

    u32 cellX = dotInCharacterPatternX / d.cellDotDimension;
    u32 cellY = dotInCharacterPatternY / d.cellDotDimension;
    u32 dotInCellX = dotInCharacterPatternX % d.cellDotDimension;
    u32 dotInCellY = dotInCharacterPatternY % d.cellDotDimension;

    u32 planeGridWidth = (layerData.mapwh > 0) ? layerData.mapwh : ((layerData.PLSZ & 1) ? 2 : 1);
    u32 planeNumber = planeY * planeGridWidth + planeX;
    u32 startOfPlane = layerData.planeOffsets[planeNumber];

    u32 pageNumber = pageY * d.pageDimension + pageX;
    u32 startOfPage = startOfPlane + pageNumber * d.pageSize;

    u32 patternNumber = characterPatternY * d.pageDimension + characterPatternX;
    u32 startOfPattern = startOfPage + patternNumber * d.patternSize;

    u32 characterNumber;
    u32 paladdr; // pre-shifted palette address: paladdr | dot = final palette entry
    u32 characterOffset;
    u32 flipfunction = 0;

    switch (d.patternSize)
    {
    case 2:
    {
        u16 patternName = getVdp2VramU16(startOfPattern);
        u16 supplementalCharacterName = layerData.SCN;

        switch (layerData.CHCN)
        {
        case 0: // 4bpp: palette from bits 12-15, supplemental from SCN bits 5-7
            paladdr = ((patternName & 0xF000) >> 8) | ((supplementalCharacterName & 0xE0) << 3);
            break;
        case 1: // 8bpp: palette from bits 12-14
            paladdr = (patternName & 0x7000) >> 4;
            break;
        default:
            assert(0);
        }

        switch (layerData.CNSM)
        {
        case 0:
            flipfunction = (patternName & 0xC00) >> 10;
            switch (layerData.CHSZ)
            {
            case 0:
                characterNumber = patternName & 0x3FF;
                characterNumber |= (supplementalCharacterName & 0x1F) << 10;
                break;
            case 1:
                characterNumber = (patternName & 0x3FF) << 2;
                characterNumber |= supplementalCharacterName & 3;
                characterNumber |= (supplementalCharacterName & 0x1C) << 10;
                break;
            }
            break;
        case 1:
            // auxmode=1: no flip
            flipfunction = 0;
            switch (layerData.CHSZ)
            {
            case 0:
                characterNumber = patternName & 0xFFF;
                characterNumber |= (supplementalCharacterName & 0x1C) << 10;
                break;
            case 1:
                characterNumber = (patternName & 0xFFF) << 2;
                characterNumber |= supplementalCharacterName & 3;
                characterNumber |= (supplementalCharacterName & 0x10) << 10;
                break;
            }
            break;
        }

        characterOffset = (characterNumber) * 0x20;
        break;
    }
    case 4:
    {
        u16 data1 = getVdp2VramU16(startOfPattern);
        u16 data2 = getVdp2VramU16(startOfPattern + 2);

        characterNumber = data2 & 0x7FFF;
        flipfunction = (data1 & 0xC000) >> 14;
        switch (layerData.CHCN)
        {
        case 0: // 4bpp
            paladdr = (data1 & 0x7F) << 4;
            break;
        default: // 8bpp+
            paladdr = (data1 & 0x70) << 4;
            break;
        }

        characterOffset = characterNumber;
        break;
    }
    default:
        assert(0);
        return 0;
    }

    // Apply flip function to coordinates within the character pattern
    // Vdp2MapCalcXY flip logic
    if (d.characterPatternDotDimension == 8)
    {
        // 8x8 character pattern
        switch (flipfunction & 0x3)
        {
        case 1: // horizontal flip
            dotInCharacterPatternX = 7 - dotInCharacterPatternX;
            break;
        case 2: // vertical flip
            dotInCharacterPatternY = 7 - dotInCharacterPatternY;
            break;
        case 3: // both
            dotInCharacterPatternX = 7 - dotInCharacterPatternX;
            dotInCharacterPatternY = 7 - dotInCharacterPatternY;
            break;
        }
        // Recompute cell and dot positions after flip
        cellX = dotInCharacterPatternX / d.cellDotDimension;
        cellY = dotInCharacterPatternY / d.cellDotDimension;
        dotInCellX = dotInCharacterPatternX % d.cellDotDimension;
        dotInCellY = dotInCharacterPatternY % d.cellDotDimension;
    }
    else
    {
        // 16x16 character pattern (4 cells of 8x8)
        if (flipfunction)
        {
            dotInCharacterPatternY &= 15;
            if (flipfunction & 0x2) // vertical flip
            {
                if (!(dotInCharacterPatternY & 8))
                    dotInCharacterPatternY = 7 - dotInCharacterPatternY + 16;
                else
                    dotInCharacterPatternY = 15 - dotInCharacterPatternY;
            }
            else if (dotInCharacterPatternY & 8)
                dotInCharacterPatternY += 8;

            if (flipfunction & 0x1) // horizontal flip
            {
                if (!(dotInCharacterPatternX & 8))
                    dotInCharacterPatternY += 8;

                dotInCharacterPatternX &= 7;
                dotInCharacterPatternX = 7 - dotInCharacterPatternX;
            }
            else if (dotInCharacterPatternX & 8)
            {
                dotInCharacterPatternY += 8;
                dotInCharacterPatternX &= 7;
            }
            else
                dotInCharacterPatternX &= 7;
        }
        else
        {
            dotInCharacterPatternY &= 15;
            if (dotInCharacterPatternY & 8)
                dotInCharacterPatternY += 8;
            if (dotInCharacterPatternX & 8)
                dotInCharacterPatternY += 8;
            dotInCharacterPatternX &= 7;
        }

        // For 16x16 patterns, y encodes the cell offset:
        // y = cellRow*8 + dotInCellY (+ extra 8 for column), x = dotInCellX (0-7)
        // Use FetchPixel-style direct addressing: charaddr + y * 8 + x
        u8 dotColor16 = 0;
        switch (layerData.CHCN)
        {
        case 0: // 4bpp
        {
            u32 dotOffset = characterOffset + ((dotInCharacterPatternY * 8 + dotInCharacterPatternX) / 2);
            dotColor16 = getVdp2VramU8(dotOffset);
            if (!(dotInCharacterPatternX & 1))
                dotColor16 >>= 4;
            else
                dotColor16 &= 0xF;
            break;
        }
        case 1: // 8bpp
        {
            u32 dotOffset = characterOffset + dotInCharacterPatternY * 8 + dotInCharacterPatternX;
            dotColor16 = getVdp2VramU8(dotOffset);
            break;
        }
        default:
            assert(0);
        }

        if (dotColor16)
        {
            u32 paletteEntry = layerData.CAOS * 0x100 + (paladdr | dotColor16);
            u16 color = getVdp2CramU16(paletteEntry * 2);
            u8 r = (color & 0x1F) << 3;
            u8 g = (color >> 5 & 0x1F) << 3;
            u8 b = (color >> 10 & 0x1F) << 3;
            return 0xFF000000 | (r << 16) | (g << 8) | b;
        }
        return 0;
    }

    // 8x8 path: use cell indexing
    u32 cellIndex = cellX + cellY * 2;
    u32 cellOffset = characterOffset + cellIndex * d.cellSizeInByte;

    u8 dotColor = 0;

    switch (layerData.CHCN)
    {
    case 0:
    {
        u32 dotOffset = cellOffset + dotInCellY * 4 + dotInCellX / 2;
        dotColor = getVdp2VramU8(dotOffset);
        if (dotInCellX & 1)
            dotColor &= 0xF;
        else
            dotColor >>= 4;
        break;
    }
    case 1:
    {
        u32 dotOffset = cellOffset + dotInCellY * 8 + dotInCellX;
        dotColor = getVdp2VramU8(dotOffset);
        break;
    }
    default:
        assert(0);
    }

    if (dotColor)
    {
        u32 paletteEntry = layerData.CAOS * 0x100 + (paladdr | dotColor);
        u16 color = getVdp2CramU16(paletteEntry * 2);
        // Saturn RGB555: bits 0-4=R, 5-9=G, 10-14=B
        // BGRA8 u32 on LE: 0xAARRGGBB
        u8 r = (color & 0x1F) << 3;
        u8 g = (color >> 5 & 0x1F) << 3;
        u8 b = (color >> 10 & 0x1F) << 3;
        return 0xFF000000 | (r << 16) | (g << 8) | b;
    }
    return 0;
}


// Fixed-point multiply matching Saturn 16.16: (a * b) >> 16
static s32 fpMul(s32 a, s32 b)
{
    return (s32)(((s64)a * (s64)b) >> 16);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// RBG0 CPU renderer
//----------------------------------------------------------------------
static void renderLayerCPU_RBG0(
    const sCoefficientTableData& tA,
    const std::vector<fixedPoint>* pCoeffsA,
    const sCoefficientTableData* pTB,
    u32 textureWidth, u32 textureHeight,
    const s_layerData& layerDataA, const s_layerData* pLayerDataB,
    u32* textureOutput)
{
    s_tileDimensions dA = computeTileDimensions(layerDataA);
    s_tileDimensions dB;
    if (pLayerDataB)
        dB = computeTileDimensions(*pLayerDataB);

    // Saturn hardware truncates bottom 6 fractional bits
    // of all rotation parameter fields when read from VRAM. Apply same truncation for bit-exact match.
    // Px/Cx are 14-bit sign-extended then tofixed; Mx/My are 30-bit sign-extended (all & 0xFFFFFFC0).
    auto truncFP = [](s32 val) -> s32 { return val & (s32)0xFFFFFFC0; };
    auto signExt14fp = [](s16 val) -> s32 {
        s32 v = val & 0x3FFF;
        return ((v & 0x2000) ? (v | (s32)0xFFFFC000) : v) << 16;
    };
    auto truncMx = [](s32 val) -> s32 {
        return (val & 0x3FFFFFC0) | ((val & 0x20000000) ? (s32)0xE0000000 : 0);
    };

    // Read param A fields with Saturn VRAM precision
    s32 A_A = truncFP(tA.m1C), B_A = truncFP(tA.m20), C_A = truncFP(tA.m24);
    s32 D_A = truncFP(tA.m28), E_A = truncFP(tA.m2C), F_A = truncFP(tA.m30);
    s32 Px_A = signExt14fp(tA.m34), Py_A = signExt14fp(tA.m36), Pz_A = signExt14fp(tA.m38);
    s32 Cx_A = signExt14fp(tA.m3C), Cy_A = signExt14fp(tA.m3E), Cz_A = signExt14fp(tA.m40);
    s32 Mx_A = truncMx(tA.m44), My_A = truncMx(tA.m48);
    s32 Xst_A = truncFP(tA.m0), Yst_A = truncFP(tA.m4), Zst_A = truncFP(tA.m8_Zst);
    s32 deltaX_A = truncFP(tA.m14), deltaY_A = truncFP(tA.m18);
    s32 deltaXst_A = truncFP(tA.mC), deltaYst_A = truncFP(tA.m10);

    // Precompute rotation values for param A
    s32 Xp_A = fpMul(A_A, Px_A - Cx_A) + fpMul(B_A, Py_A - Cy_A) + fpMul(C_A, Pz_A - Cz_A) + Cx_A + Mx_A;
    s32 Yp_A = fpMul(D_A, Px_A - Cx_A) + fpMul(E_A, Py_A - Cy_A) + fpMul(F_A, Pz_A - Cz_A) + Cy_A + My_A;
    // GenerateRotatedVarFP
    s32 xmul_A = Xst_A - Px_A;
    s32 ymul_A = Yst_A - Py_A;
    s32 C_val_A = fpMul(C_A, Zst_A - Pz_A);
    s32 F_val_A = fpMul(F_A, Zst_A - Pz_A);
    // dX = A*deltaX + B*deltaY (rotation matrix applied to delta)
    s32 dX_A = fpMul(A_A, deltaX_A) + fpMul(B_A, deltaY_A);
    s32 dY_A = fpMul(D_A, deltaX_A) + fpMul(E_A, deltaY_A);
    s32 deltaKAst_A = truncFP(tA.m58);

    // Coordinate wrapping masks: mapwh * planeDotWidth/Height - 1
    u32 xmask_A = 4 * dA.planeDotWidth - 1;
    u32 ymask_A = 4 * dA.planeDotHeight - 1;

    s32 Xp_B = 0, Yp_B = 0, xmul_B = 0, ymul_B = 0;
    s32 C_val_B = 0, F_val_B = 0;
    u32 xmask_B = 0, ymask_B = 0;
    s32 dX_B = 0, dY_B = 0;
    s32 A_B = 0, B_B = 0, C_B = 0, D_B = 0, E_B = 0, F_B = 0;
    if (pTB)
    {
        A_B = truncFP(pTB->m1C); B_B = truncFP(pTB->m20); C_B = truncFP(pTB->m24);
        D_B = truncFP(pTB->m28); E_B = truncFP(pTB->m2C); F_B = truncFP(pTB->m30);
        s32 Px_B = signExt14fp(pTB->m34), Py_B = signExt14fp(pTB->m36), Pz_B = signExt14fp(pTB->m38);
        s32 Cx_B = signExt14fp(pTB->m3C), Cy_B = signExt14fp(pTB->m3E), Cz_B = signExt14fp(pTB->m40);
        s32 Mx_B = truncMx(pTB->m44), My_B = truncMx(pTB->m48);
        s32 Xst_B = truncFP(pTB->m0), Yst_B = truncFP(pTB->m4), Zst_B = truncFP(pTB->m8_Zst);
        s32 deltaX_B = truncFP(pTB->m14), deltaY_B = truncFP(pTB->m18);
        Xp_B = fpMul(A_B, Px_B - Cx_B) + fpMul(B_B, Py_B - Cy_B) + fpMul(C_B, Pz_B - Cz_B) + Cx_B + Mx_B;
        Yp_B = fpMul(D_B, Px_B - Cx_B) + fpMul(E_B, Py_B - Cy_B) + fpMul(F_B, Pz_B - Cz_B) + Cy_B + My_B;
        xmul_B = Xst_B - Px_B;
        ymul_B = Yst_B - Py_B;
        C_val_B = fpMul(C_B, Zst_B - Pz_B);
        F_val_B = fpMul(F_B, Zst_B - Pz_B);
        dX_B = fpMul(A_B, deltaX_B) + fpMul(B_B, deltaY_B);
        dY_B = fpMul(D_B, deltaX_B) + fpMul(E_B, deltaY_B);
        if (pLayerDataB)
        {
            s_tileDimensions dBtmp = computeTileDimensions(*pLayerDataB);
            xmask_B = 4 * dBtmp.planeDotWidth - 1;
            ymask_B = 4 * dBtmp.planeDotHeight - 1;
        }
    }

    // Coefficient iteration: integer + fractional parts (coefy/rcoefy split)
    u32 coefy_A = 0, rcoefy_A = 0;

    for (u32 j = 0; j < textureHeight; j++)
    {
        // Read per-scanline coefficient (deltaKAx==0 means per-scanline only)
        s32 kx_A = 0x10000;
        bool msb_A = false;
        if (pCoeffsA && !pCoeffsA->empty())
        {
            // Read from VRAM at: coeftbladdr + (coefy + touint(rcoefy)) * coefdatasize
            // Our vector index = coefy + touint(rcoefy) + KAst_integer
            s32 idx = (s32)(coefy_A + (rcoefy_A >> 16)) + (truncFP(tA.m54) >> 16);
            if (idx >= 0 && idx < (s32)pCoeffsA->size())
            {
                s32 coefVal = (s32)(*pCoeffsA)[idx];
                msb_A = (coefVal >> 31) & 1;
                // Coefficient vector stores fixedPoint (16.16), originally 32-bit BE in VRAM,
                // then reads lower 24 bits with sign extension (Vdp2ReadCoefficientMode0_2FP, coefdatasize==4).
                // This truncates the top 8 integer bits but preserves the sign.
                kx_A = (coefVal & 0x00FFFFFF) | ((coefVal & 0x00800000) ? 0xFF000000 : 0);
            }
            // Accumulate: coefy += toint(deltaKAst), rcoefy += decipart(deltaKAst)
            coefy_A += (u32)(deltaKAst_A >> 16);      // integer part
            rcoefy_A += (u32)(deltaKAst_A & 0xFFFF);   // fractional part (wraps naturally)
        }

        // GenerateRotatedXPosFP/YPosFP:
        // Xsp = A*xmul + B*ymul + C*(Zst-Pz)   (scanline-constant part)
        // result = kx * (Xsp + dX * pixel_x) + Xp   (per-pixel, dX = A*deltaX + B*deltaY)
        s32 Xsp_A = fpMul(A_A, xmul_A) + fpMul(B_A, ymul_A) + C_val_A;
        s32 Ysp_A = fpMul(D_A, xmul_A) + fpMul(E_A, ymul_A) + F_val_A;

        s32 Xsp_B = 0, Ysp_B = 0;
        if (pTB)
        {
            Xsp_B = fpMul(A_B, xmul_B) + fpMul(B_B, ymul_B) + C_val_B;
            Ysp_B = fpMul(D_B, xmul_B) + fpMul(E_B, ymul_B) + F_val_B;
        }

        for (u32 i = 0; i < textureWidth; i++)
        {
            bool useB = pTB && msb_A;

            s32 mapX, mapY;
            if (!useB)
            {
                // kx * (Xsp + dX * i) + Xp, where dX = A*deltaX + B*deltaY
                s32 Xsp_pixel = Xsp_A + fpMul(dX_A, (s32)i << 16);
                s32 Ysp_pixel = Ysp_A + fpMul(dY_A, (s32)i << 16);
                s32 rawX = fpMul(kx_A, Xsp_pixel) + Xp_A;
                s32 rawY = fpMul(kx_A, Ysp_pixel) + Yp_A;
                mapX = (rawX >> 16) & (s32)xmask_A;
                mapY = (rawY >> 16) & (s32)ymask_A;
            }
            else
            {
                s32 Xsp_pixel = Xsp_B + fpMul(dX_B, (s32)i << 16);
                s32 Ysp_pixel = Ysp_B + fpMul(dY_B, (s32)i << 16);
                // Param B: kx from param B coefficients (or 1.0 default)
                s32 rawX = fpMul(0x10000, Xsp_pixel) + Xp_B;
                s32 rawY = fpMul(0x10000, Ysp_pixel) + Yp_B;
                mapX = (rawX >> 16) & (s32)xmask_B;
                mapY = (rawY >> 16) & (s32)ymask_B;
            }

            u32 color;
            if (!useB)
                color = sampleTileAtCoordinate((u32)mapX, (u32)mapY, layerDataA, dA);
            else
                color = sampleTileAtCoordinate((u32)mapX, (u32)mapY, *pLayerDataB, dB);

            if (color)
                textureOutput[j * textureWidth + i] = color;
        }

        xmul_A += deltaXst_A;
        ymul_A += deltaYst_A;
        if (pTB)
        {
            xmul_B += truncFP(pTB->mC);
            ymul_B += truncFP(pTB->m10);
        }
    }
}

static void renderLayerCPU_RBG0_wrapper(
    const sCoefficientTableData& tA,
    const std::vector<fixedPoint>* pCoeffsA,
    const sCoefficientTableData* pTB,
    u32 textureWidth, u32 textureHeight,
    const s_layerData& layerDataA, const s_layerData* pLayerDataB,
    s_NBG_data& NBGData)
{
    u32 totalPixels = textureWidth * textureHeight;
    u32* output = new u32[totalPixels];
    memset(output, 0, totalPixels * sizeof(u32));

    renderLayerCPU_RBG0(tA, pCoeffsA, pTB, textureWidth, textureHeight,
        layerDataA, pLayerDataB, output);

    // Upload to bgfx texture
    if (!isValid(NBGData.BGFXTexture) || NBGData.m_currentWidth != (int)textureWidth || NBGData.m_currentHeight != (int)textureHeight)
    {
        if (isValid(NBGData.BGFXTexture))
        {
            bgfx::destroy(NBGData.BGFXTexture);
        }
        if (isValid(NBGData.BGFXFB))
        {
            bgfx::destroy(NBGData.BGFXFB);
            NBGData.BGFXFB = BGFX_INVALID_HANDLE;
        }
        NBGData.BGFXTexture = bgfx::createTexture2D(textureWidth, textureHeight, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_NONE);
        NBGData.m_currentWidth = textureWidth;
        NBGData.m_currentHeight = textureHeight;
    }

    bgfx::updateTexture2D(NBGData.BGFXTexture, 0, 0, 0, 0, textureWidth, textureHeight,
        bgfx::copy(output, textureWidth * textureHeight * 4));

    delete[] output;
}

//----------------------------------------------------------------------
// CPU tile renderer (for NBG layers)
//----------------------------------------------------------------------
void renderLayer(s_layerData& layerData, u32 textureWidth, u32 textureHeight, u32* textureOutput)
{
    for (int i = 0; i < textureWidth * textureHeight; i++)
    {
        textureOutput[i] = 0x00FF0000;
    }

    u32 cellDotDimension = 8;
    u32 characterPatternDotDimension = cellDotDimension * ((layerData.CHSZ == 0) ? 1 : 2);
    u32 pageDotDimension = 8 * 64; /* A page is always 64x64 cells, so 512 * 512 dots*/
    u32 planeDotWidth = pageDotDimension * ((layerData.PLSZ & 1) ? 2 : 1);
    u32 planeDotHeight = pageDotDimension * ((layerData.PLSZ & 2) ? 2 : 1);
    u32 mapDotWidth = planeDotWidth * 2; // because scrollScreen
    u32 mapDotHeight = planeDotHeight * 2; // because scrollScreen

    u32 pageDimension = (layerData.CHSZ == 0) ? 64 : 32;
    u32 patternSize = (layerData.PNB == 0) ? 4 : 2;
    u32 pageSize = pageDimension * pageDimension * patternSize;

    u32 cellSizeInByte = 8 * 8;
    switch (layerData.CHCN)
    {
        // 4bpp
    case 0:
        cellSizeInByte /= 2;
        break;
        // 8bpp
    case 1:
        cellSizeInByte *= 1;
        break;
    }

    for (u32 rawOutputY = 0; rawOutputY < textureHeight; rawOutputY++)
    {
        for (u32 rawOutputX = 0; rawOutputX < textureWidth; rawOutputX++)
        {
            s32 outputX = rawOutputX;
            s32 outputY = rawOutputY;

            if (layerData.lineScrollEA)
            {
                outputY = getVdp2VramU32(layerData.lineScrollEA + rawOutputY * 4) >> 16;
            }

            outputX += layerData.scrollX;
            outputY += layerData.scrollY;

            if (outputX < 0)
                continue;
            if (outputY < 0)
                continue;

            u32 planeX = outputX / planeDotWidth;
            u32 planeY = outputY / planeDotHeight;
            u32 dotInPlaneX = outputX % planeDotWidth;
            u32 dotInPlaneY = outputY % planeDotWidth;

            u32 pageX = dotInPlaneX / pageDotDimension;
            u32 pageY = dotInPlaneY / pageDotDimension;
            u32 dotInPageX = dotInPlaneX % pageDotDimension;
            u32 dotInPageY = dotInPlaneY % pageDotDimension;

            u32 characterPatternX = dotInPageX / characterPatternDotDimension;
            u32 characterPatternY = dotInPageY / characterPatternDotDimension;
            u32 dotInCharacterPatternX = dotInPageX % characterPatternDotDimension;
            u32 dotInCharacterPatternY = dotInPageY % characterPatternDotDimension;

            u32 cellX = dotInCharacterPatternX / cellDotDimension;
            u32 cellY = dotInCharacterPatternY / cellDotDimension;
            u32 dotInCellX = dotInCharacterPatternX % cellDotDimension;
            u32 dotInCellY = dotInCharacterPatternY % cellDotDimension;

            u32 planeGridWidth = (layerData.mapwh > 0) ? layerData.mapwh : ((layerData.PLSZ & 1) ? 2 : 1);
            u32 planeNumber = planeY * planeGridWidth + planeX;
            u32 startOfPlane = layerData.planeOffsets[planeNumber];

            u32 pageNumber = pageY * pageDimension + pageX;
            u32 startOfPage = startOfPlane + pageNumber * pageSize;

            u32 patternNumber = characterPatternY * pageDimension + characterPatternX;
            u32 startOfPattern = startOfPage + patternNumber * patternSize;

            u32 characterNumber;
            u32 paletteNumber;
            u32 characterOffset;

            switch (patternSize)
            {
            case 2:
            {
                u16 patternName = getVdp2VramU16(startOfPattern);
                u16 supplementalCharacterName = layerData.SCN;

                switch (layerData.CHCN)
                {
                case 0:
                    // assuming supplement mode 0 with no data
                    paletteNumber = (patternName & 0xF000) >> 12;
                    break;
                case 1:
                    paletteNumber = (patternName & 0x7000) >> 8;
                    break;
                default:
                    assert(0);
                }

                switch (layerData.CNSM)
                {
                case 0:
                    switch (layerData.CHSZ)
                    {
                    case 0:
                        characterNumber = patternName & 0x3FF;
                        characterNumber |= (supplementalCharacterName & 0x1F) << 10;
                        break;
                    case 1:
                        characterNumber = (patternName & 0x3FF) << 2;
                        characterNumber |= supplementalCharacterName & 3;
                        characterNumber |= (supplementalCharacterName & 0x1C) << 10;
                        break;
                    }
                    break;
                case 1:
                    switch (layerData.CHSZ)
                    {
                    case 0:
                        characterNumber = patternName & 0xFFF;
                        characterNumber |= (supplementalCharacterName & 0x1C) << 10;
                        break;
                    case 1:
                        characterNumber = (patternName & 0xFFF) << 2;
                        characterNumber |= supplementalCharacterName & 3;
                        characterNumber |= (supplementalCharacterName & 0x10) << 10;
                        break;
                    }
                    break;
                }

                characterOffset = (characterNumber) * 0x20;

                if (characterNumber)
                {
                    characterNumber = characterNumber;
                }
                break;
            }
            case 4:
            {
                u16 data1 = getVdp2VramU16(startOfPattern);
                u16 data2 = getVdp2VramU16(startOfPattern + 2);

                // assuming supplement mode 0 with no data
                characterNumber = data2 & 0x7FFF;
                paletteNumber = data1 & 0x7F;

                characterOffset = characterNumber;
                break;
            }
            default:
                assert(0);
            }

            u32 cellIndex = cellX + cellY * 2;
            u32 cellOffset = characterOffset + cellIndex * cellSizeInByte;

            u8 dotColor = 0;
            u32 paletteOffset = 0;

            switch (layerData.CHCN)
            {
                // 16 colors, 4bits
                case 0:
                {
                    u32 dotOffset = cellOffset + dotInCellY * 4 + dotInCellX / 2;
                    dotColor = getVdp2VramU8(dotOffset);

                    if (dotInCellX & 1)
                    {
                        dotColor &= 0xF;
                    }
                    else
                    {
                        dotColor >>= 4;
                    }
                    break;
                }
                // 256 colors, 8bits
                case 1:
                {
                    u32 dotOffset = cellOffset + dotInCellY * 8 + dotInCellX;
                    dotColor = getVdp2VramU8(dotOffset);
                    break;
                }
                default:
                    assert(0);
            }

            if(dotColor)
            {
                paletteOffset = ((paletteNumber << 4) + dotColor) * 2 + layerData.CAOS * 0x200;
                u16 color = getVdp2CramU16(paletteOffset);
                u32 finalColor = 0xFF000000 | (((color & 0x1F) << 3) | ((color & 0x03E0) << 6) | ((color & 0x7C00) << 9));

                textureOutput[(textureHeight - 1 - rawOutputY) * textureWidth + rawOutputX] = finalColor;
            }
        }
    }
}

//----------------------------------------------------------------------
// Public renderBG/RBG functions
//----------------------------------------------------------------------

void renderBG0(u32 width, u32 height, bool bGPU)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if ((vdp2Controls.m4_pendingVdp2Regs[0].m0_TVMD & 0xC0) == 0xC0)
    {
        textureWidth *= 2;
        textureHeight *= 2;
    }

    if(vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x1)
    {
        s_layerData planeData;
        planeData.CHSZ = vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA & 1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA >> 4) & 7;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->m30_PNCN0) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->m70_SCXN0 >> 16;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->m74_SCYN0 >> 16;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = (vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m40_MPABN0 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m40_MPABN0 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m42_MPCDN0 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m42_MPCDN0 >> 8) & 0x3F)) * pageSize;

        if (!bGPU)
        {
            u32* textureOutput = new u32[textureWidth * textureHeight];
            renderLayer(planeData, textureWidth, textureHeight, textureOutput);

            assert(0); // need to update the texture in bgfx

            delete[] textureOutput;
        }
        else
        {
            renderLayerGPU(planeData, textureWidth, textureHeight, NBG_data[0]);
        }
    }
}

void renderBG1(u32 width, u32 height, bool bGPU)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x2)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA >> 8) & 0x1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->m28_CHCTLA >> 12) & 3;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA >> 4) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ >> 2) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->m32_PNCN1) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->m80_SCXN1 >> 16;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->m84_SCYN1 >> 16;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN >> 4) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m44_MPABN1 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m44_MPABN1 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m46_MPCDN1 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m46_MPCDN1 >> 8) & 0x3F)) * pageSize;

        if (vdp2Controls.m4_pendingVdp2Regs->m9A_SCRCTL & 0x400)
        {
            planeData.lineScrollEA = getVdp2VramOffset(vdp2Controls.m4_pendingVdp2Regs->mA4_LSTA1) - 0x25E00000;
        }

        if (!bGPU)
        {
            u32* textureOutput = new u32[textureWidth * textureHeight];
            renderLayer(planeData, textureWidth, textureHeight, textureOutput);

            assert(0); // need to update the texture in bgfx

            delete[] textureOutput;
        }
        else
        {
            renderLayerGPU(planeData, textureWidth, textureHeight, NBG_data[1]);
        }
    }
}

void renderBG2()
{
    /*
    u32 textureWidth = 1024;
    u32 textureHeight = 1024;

    u32* textureOutput = new u32[textureWidth * textureHeight];

    s_planeData planeData;

    planeData.CHSZ = vdp2Controls.m_pendingVdp2Regs->CHCTLB & 1;
    planeData.PNB = vdp2Controls.m_pendingVdp2Regs->PNCN2 & 0x8000;

    planeData.pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
    planeData.patternSize = (planeData.PNB == 0) ? 4 : 2;

    planeData.CAOS = (vdp2Controls.m_pendingVdp2Regs->CRAOFA >> 8) & 0x7;

    planeData.pageSize = planeData.pageDimension * planeData.pageDimension * planeData.patternSize;

    planeData.planeOffsets[0] = (vdp2Controls.m_pendingVdp2Regs->MPABN2 & 0x3F) * planeData.pageSize;
    planeData.planeOffsets[1] = ((vdp2Controls.m_pendingVdp2Regs->MPABN2 >> 8) & 0x3F) * planeData.pageSize;
    planeData.planeOffsets[2] = (vdp2Controls.m_pendingVdp2Regs->MPCDN2 & 0x3F) * planeData.pageSize;
    planeData.planeOffsets[3] = ((vdp2Controls.m_pendingVdp2Regs->MPCDN2 >> 8) & 0x3F) * planeData.pageSize;

    renderPlane(planeData, textureWidth, textureHeight, textureOutput);

    glBindTexture(GL_TEXTURE_2D, gNBG2Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureOutput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    delete[] textureOutput;
    */
}

void renderBG3(u32 width, u32 height, bool bGPU)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x8)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB >> 4) & 0x1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB >> 5) & 0x1;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3 >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->mE4_CRAOFA >> 12) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ >> 6) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->m36_PNCN3) & 0x1F;
        planeData.scrollX = vdp2Controls.m4_pendingVdp2Regs->m94_SCXN3;
        planeData.scrollY = vdp2Controls.m4_pendingVdp2Regs->m96_SCYN3;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN >> 12) & 7) << 6;

        planeData.planeOffsets[0] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m4C_MPABN3 & 0x3F)) * pageSize;
        planeData.planeOffsets[1] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m4C_MPABN3 >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m4E_MPCDN3 & 0x3F)) * pageSize;
        planeData.planeOffsets[3] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m4E_MPCDN3 >> 8) & 0x3F)) * pageSize;

        if (!bGPU)
        {
            u32* textureOutput = new u32[textureWidth * textureHeight];
            renderLayer(planeData, textureWidth, textureHeight, textureOutput);

            assert(0); // need to update the texture in bgfx

            delete[] textureOutput;
        }
        else
        {
            renderLayerGPU(planeData, textureWidth, textureHeight, NBG_data[3]);
        }
    }

}

void renderRBG0(u32 width, u32 height, bool bGPU)
{
    u32 textureWidth = width;
    u32 textureHeight = height;

    if (vdp2Controls.m4_pendingVdp2Regs->m20_BGON & 0x10)
    {
        s_layerData planeData;

        planeData.CHSZ = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB >> 8) & 0x1;
        planeData.CHCN = (vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB >> 12) & 0x7;
        planeData.PNB = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR >> 15) & 0x1;
        planeData.CNSM = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR >> 14) & 0x1;
        planeData.CAOS = (vdp2Controls.m4_pendingVdp2Regs->mE6_CRAOFB >> 0) & 0x7;
        planeData.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ >> 8) & 3;
        planeData.SCN = (vdp2Controls.m4_pendingVdp2Regs->m38_PNCR) & 0x3FF;
        planeData.scrollX = 0;
        planeData.scrollY = 0;

        u32 pageDimension = (planeData.CHSZ == 0) ? 64 : 32;
        u32 patternSize = (planeData.PNB == 0) ? 4 : 2;

        u32 pageSize = pageDimension * pageDimension * patternSize;

        u32 offset = ((vdp2Controls.m4_pendingVdp2Regs->m3E_MPOFR >> 0) & 7) << 6;

        planeData.mapwh = 4;
        planeData.planeOffsets[0]  = (offset + (vdp2Controls.m4_pendingVdp2Regs->m50_MPABRA & 0x3F)) * pageSize;
        planeData.planeOffsets[1]  = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m50_MPABRA >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[2]  = (offset + (vdp2Controls.m4_pendingVdp2Regs->m52_MPCDRA & 0x3F)) * pageSize;
        planeData.planeOffsets[3]  = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m52_MPCDRA >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[4]  = (offset + (vdp2Controls.m4_pendingVdp2Regs->m54_MPEFRA & 0x3F)) * pageSize;
        planeData.planeOffsets[5]  = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m54_MPEFRA >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[6]  = (offset + (vdp2Controls.m4_pendingVdp2Regs->m56_MPGHRA & 0x3F)) * pageSize;
        planeData.planeOffsets[7]  = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m56_MPGHRA >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[8]  = (offset + (vdp2Controls.m4_pendingVdp2Regs->m58_MPIJRA & 0x3F)) * pageSize;
        planeData.planeOffsets[9]  = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m58_MPIJRA >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[10] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m5A_MPKLRA & 0x3F)) * pageSize;
        planeData.planeOffsets[11] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m5A_MPKLRA >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[12] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m5C_MPMNRA & 0x3F)) * pageSize;
        planeData.planeOffsets[13] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m5C_MPMNRA >> 8) & 0x3F)) * pageSize;
        planeData.planeOffsets[14] = (offset + (vdp2Controls.m4_pendingVdp2Regs->m5E_MPOPRA & 0x3F)) * pageSize;
        planeData.planeOffsets[15] = (offset + ((vdp2Controls.m4_pendingVdp2Regs->m5E_MPOPRA >> 8) & 0x3F)) * pageSize;

        {
            int bufIdx = (s32)vdp2Controls.m0_doubleBufferIndex;
            std::vector<fixedPoint>* pCoeffTableA = gVdp2CoefficientTables[0][bufIdx];
            u16 rpmd = vdp2Controls.m4_pendingVdp2Regs->mB0_RPMD & 3;
            sCoefficientTableData& tA = gCoefficientTables[0][bufIdx];

            if (rpmd == 2)
            {
                // Dual-plane mode: param B uses separate plane registers (MPABRB etc.)
                sCoefficientTableData& tB = gCoefficientTables[1][bufIdx];

                s_layerData planeDataB;
                planeDataB.CHSZ = planeData.CHSZ;
                planeDataB.CHCN = planeData.CHCN;
                planeDataB.PNB = planeData.PNB;
                planeDataB.CNSM = planeData.CNSM;
                planeDataB.CAOS = planeData.CAOS;
                planeDataB.PLSZ = (vdp2Controls.m4_pendingVdp2Regs->m3A_PLSZ >> 12) & 3;
                planeDataB.SCN = planeData.SCN;
                planeDataB.scrollX = 0;
                planeDataB.scrollY = 0;

                u32 pageDimensionB = (planeDataB.CHSZ == 0) ? 64 : 32;
                u32 patternSizeB = (planeDataB.PNB == 0) ? 4 : 2;
                u32 pageSizeB = pageDimensionB * pageDimensionB * patternSizeB;
                u32 offsetB = ((vdp2Controls.m4_pendingVdp2Regs->m3E_MPOFR >> 4) & 7) << 6;

                planeDataB.mapwh = 4;
                planeDataB.planeOffsets[0]  = (offsetB + (vdp2Controls.m4_pendingVdp2Regs->m60_MPABRB & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[1]  = (offsetB + ((vdp2Controls.m4_pendingVdp2Regs->m60_MPABRB >> 8) & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[2]  = (offsetB + (vdp2Controls.m4_pendingVdp2Regs->m62_MPCDRB & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[3]  = (offsetB + ((vdp2Controls.m4_pendingVdp2Regs->m62_MPCDRB >> 8) & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[4]  = (offsetB + (vdp2Controls.m4_pendingVdp2Regs->m64_MPEFRB & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[5]  = (offsetB + ((vdp2Controls.m4_pendingVdp2Regs->m64_MPEFRB >> 8) & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[6]  = (offsetB + (vdp2Controls.m4_pendingVdp2Regs->m66_MPGHRB & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[7]  = (offsetB + ((vdp2Controls.m4_pendingVdp2Regs->m66_MPGHRB >> 8) & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[8]  = (offsetB + (vdp2Controls.m4_pendingVdp2Regs->m68_MPIJRB & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[9]  = (offsetB + ((vdp2Controls.m4_pendingVdp2Regs->m68_MPIJRB >> 8) & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[10] = (offsetB + (vdp2Controls.m4_pendingVdp2Regs->m6A_MPKLRB & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[11] = (offsetB + ((vdp2Controls.m4_pendingVdp2Regs->m6A_MPKLRB >> 8) & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[12] = (offsetB + (vdp2Controls.m4_pendingVdp2Regs->m6C_MPMNRB & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[13] = (offsetB + ((vdp2Controls.m4_pendingVdp2Regs->m6C_MPMNRB >> 8) & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[14] = (offsetB + (vdp2Controls.m4_pendingVdp2Regs->m6E_MPOPRB & 0x3F)) * pageSizeB;
                planeDataB.planeOffsets[15] = (offsetB + ((vdp2Controls.m4_pendingVdp2Regs->m6E_MPOPRB >> 8) & 0x3F)) * pageSizeB;

                renderLayerCPU_RBG0_wrapper(tA, pCoeffTableA, &tB, textureWidth, textureHeight, planeData, &planeDataB, NBG_data[4]);
            }
            else
            {
                renderLayerCPU_RBG0_wrapper(tA, pCoeffTableA, nullptr, textureWidth, textureHeight, planeData, nullptr, NBG_data[4]);
            }
        }
    }

}
