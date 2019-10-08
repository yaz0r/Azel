#pragma once

#ifdef WITH_GL
#ifdef USE_GL_ES3
    #if defined(__APPLE__)
        #include <OpenGLES/ES3/gl.h>
        #include <OpenGLES/ES3/glext.h>
    #else
        #include <GLES3/gl3.h>
    #endif
#elif defined(USE_GL)
    #include <GL/gl3w.h>
    #if (defined(__APPLE__) && (TARGET_OS_OSX)) || defined(__MACOS__)
        #include <Opengl/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#endif

#include "renderer.h"

class SDL_ES3_backend : public backend
{
private:
    static bool init();
public:
    static backend* create();

    virtual void bindBackBuffer() override;

    virtual void ImGUI_NewFrame() override;
    virtual void ImGUI_RenderDrawData(ImDrawData* pDrawData) override;
};

#endif
