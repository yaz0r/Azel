#pragma once

#ifndef SHIPPING_BUILD
struct sPDS_Logger
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets;        // Index to lines offset
    bool                ScrollToBottom;

    void    Clear() { Buf.clear(); LineOffsets.clear(); }

    void    AddLog(const char* fmt, ...)
    {
	{
		va_list args;
		va_start (args, fmt);
		vprintf (fmt, args);
		va_end (args);
	}
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);

        if (*(Buf.end() - 1) != '\n')
            Buf.appendf("\n");

        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size);
        ScrollToBottom = true;
    }

    void    Draw(const char* title, bool* p_opened = NULL)
    {
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin(title, p_opened);
        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        ImGui::Separator();
        ImGui::BeginChild("scrolling");
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
        if (copy) ImGui::LogToClipboard();

        if (Filter.IsActive())
        {
            const char* buf_begin = Buf.begin();
            const char* line = buf_begin;
            for (int line_no = 0; line != NULL; line_no++)
            {
                const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
                if (Filter.PassFilter(line, line_end))
                    ImGui::TextUnformatted(line, line_end);
                line = line_end && line_end[1] ? line_end + 1 : NULL;
            }
        }
        else
        {
            ImGui::TextUnformatted(Buf.begin());
        }

        if (ScrollToBottom)
            ImGui::SetScrollHere(1.0f);
        ScrollToBottom = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
    }
};

enum eLogCategories
{
    log_default = 0,
    log_task,
    log_unimlemented,
    log_warning,

    log_max
};
extern sPDS_Logger PDS_Logger[eLogCategories::log_max];
#endif

#ifdef SHIPPING_BUILD
#define PDS_Log(string, ...)
#define PDS_CategorizedLog(logCategory, string, ...)
#define PDS_unimplemented(name)
#define PDS_warningOnce(name)
#else
#define PDS_Log(string, ...) {PDS_Logger[log_default].AddLog(string, __VA_ARGS__);}
#define PDS_CategorizedLog(logCategory, string, ...) {PDS_Logger[logCategory].AddLog(string, __VA_ARGS__);}
#define PDS_unimplemented(name) { static bool printed = false; if(!printed) {printed = true; PDS_Logger[log_unimlemented].AddLog("Unimplemented: %s\n", name);}}
#define PDS_warningOnce(name) { static bool printed = false; if(!printed) {printed = true; PDS_Logger[log_warning].AddLog("Warning: %s\n", name);}}
#endif
