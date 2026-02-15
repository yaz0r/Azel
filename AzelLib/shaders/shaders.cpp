#include "PDS.h"
#include "embeddedShadersMacro.h"

#if BGFX_PLATFORM_SUPPORTS_SPIRV
#include "shaders/generated/spirv/VDP1_2dUI_ps.sc.bin.h"
#include "shaders/generated/spirv/VDP1_2dUI_vs.sc.bin.h"
#include "shaders/generated/spirv/VDP1_2dUIVertexColor_ps.sc.bin.h"
#include "shaders/generated/spirv/VDP1_2dUIVertexColor_vs.sc.bin.h"
#include "shaders/generated/spirv/VDP1_ps.sc.bin.h"
#include "shaders/generated/spirv/VDP1_vs.sc.bin.h"
#include "shaders/generated/spirv/VDP1_vertexColor_ps.sc.bin.h"
#include "shaders/generated/spirv/VDP1_vertexColor_vs.sc.bin.h"
#include "shaders/generated/spirv/VDP2_blit_ps.sc.bin.h"
#include "shaders/generated/spirv/VDP2_blit_vs.sc.bin.h"
#include "shaders/generated/spirv/VDP2_ps.sc.bin.h"
#include "shaders/generated/spirv/VDP2_vs.sc.bin.h"
#endif

#if BGFX_PLATFORM_SUPPORTS_METAL
#include "shaders/generated/metal/VDP1_2dUI_ps.sc.bin.h"
#include "shaders/generated/metal/VDP1_2dUI_vs.sc.bin.h"
#include "shaders/generated/metal/VDP1_2dUIVertexColor_ps.sc.bin.h"
#include "shaders/generated/metal/VDP1_2dUIVertexColor_vs.sc.bin.h"
#include "shaders/generated/metal/VDP1_ps.sc.bin.h"
#include "shaders/generated/metal/VDP1_vs.sc.bin.h"
#include "shaders/generated/metal/VDP1_vertexColor_ps.sc.bin.h"
#include "shaders/generated/metal/VDP1_vertexColor_vs.sc.bin.h"
#include "shaders/generated/metal/VDP2_blit_ps.sc.bin.h"
#include "shaders/generated/metal/VDP2_blit_vs.sc.bin.h"
#include "shaders/generated/metal/VDP2_ps.sc.bin.h"
#include "shaders/generated/metal/VDP2_vs.sc.bin.h"
#endif

#if BGFX_PLATFORM_SUPPORTS_GLSL
#include "shaders/generated/glsl/VDP1_2dUI_ps.sc.bin.h"
#include "shaders/generated/glsl/VDP1_2dUI_vs.sc.bin.h"
#include "shaders/generated/glsl/VDP1_2dUIVertexColor_ps.sc.bin.h"
#include "shaders/generated/glsl/VDP1_2dUIVertexColor_vs.sc.bin.h"
#include "shaders/generated/glsl/VDP1_ps.sc.bin.h"
#include "shaders/generated/glsl/VDP1_vs.sc.bin.h"
#include "shaders/generated/glsl/VDP1_vertexColor_ps.sc.bin.h"
#include "shaders/generated/glsl/VDP1_vertexColor_vs.sc.bin.h"
#include "shaders/generated/glsl/VDP2_blit_ps.sc.bin.h"
#include "shaders/generated/glsl/VDP2_blit_vs.sc.bin.h"
#include "shaders/generated/glsl/VDP2_ps.sc.bin.h"
#include "shaders/generated/glsl/VDP2_vs.sc.bin.h"
#endif

#if BGFX_PLATFORM_SUPPORTS_DXBC
#include "shaders/generated/dx11/VDP1_2dUI_ps.sc.bin.h"
#include "shaders/generated/dx11/VDP1_2dUI_vs.sc.bin.h"
#include "shaders/generated/dx11/VDP1_2dUIVertexColor_ps.sc.bin.h"
#include "shaders/generated/dx11/VDP1_2dUIVertexColor_vs.sc.bin.h"
#include "shaders/generated/dx11/VDP1_ps.sc.bin.h"
#include "shaders/generated/dx11/VDP1_vs.sc.bin.h"
#include "shaders/generated/dx11/VDP1_vertexColor_ps.sc.bin.h"
#include "shaders/generated/dx11/VDP1_vertexColor_vs.sc.bin.h"
#include "shaders/generated/dx11/VDP2_blit_ps.sc.bin.h"
#include "shaders/generated/dx11/VDP2_blit_vs.sc.bin.h"
#include "shaders/generated/dx11/VDP2_ps.sc.bin.h"
#include "shaders/generated/dx11/VDP2_vs.sc.bin.h"
#endif

static const bgfx::EmbeddedShader s_embeddedShaders[] =
{
    BGFX_EMBEDDED_SHADER(VDP1_2dUI_ps),
    BGFX_EMBEDDED_SHADER(VDP1_2dUI_vs),
    BGFX_EMBEDDED_SHADER(VDP1_2dUIVertexColor_ps),
    BGFX_EMBEDDED_SHADER(VDP1_2dUIVertexColor_vs),
    BGFX_EMBEDDED_SHADER(VDP1_ps),
    BGFX_EMBEDDED_SHADER(VDP1_vs),
    BGFX_EMBEDDED_SHADER(VDP1_vertexColor_ps),
    BGFX_EMBEDDED_SHADER(VDP1_vertexColor_vs),
    BGFX_EMBEDDED_SHADER(VDP2_blit_ps),
    BGFX_EMBEDDED_SHADER(VDP2_blit_vs),
    BGFX_EMBEDDED_SHADER(VDP2_ps),
    BGFX_EMBEDDED_SHADER(VDP2_vs),

    BGFX_EMBEDDED_SHADER_END()
};

bgfx::ProgramHandle loadBgfxProgram(const std::string& VSFile, const std::string& PSFile)
{
    bgfx::RendererType::Enum type = bgfx::getRendererType();

    bgfx::ProgramHandle ProgramHandle = bgfx::createProgram(
        bgfx::createEmbeddedShader(s_embeddedShaders, type, VSFile.c_str())
        , bgfx::createEmbeddedShader(s_embeddedShaders, type, PSFile.c_str())
        , true
    );
    assert(bgfx::isValid(ProgramHandle));
    return ProgramHandle;
}
