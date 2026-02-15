#pragma once

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

