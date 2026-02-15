#pragma once

class backend
{
public:
    virtual void bindBackBuffer() = 0;
    virtual void ImGUI_NewFrame() = 0;
    virtual void ImGUI_RenderDrawData(ImDrawData* pDrawData) = 0;
};
