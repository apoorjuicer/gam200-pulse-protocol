#pragma once
#include <imgui.h>
#include <vector>
#include <string>

class DebugConsole
{
public:
    DebugConsole();
    ~DebugConsole();

    void Clear();
    void AddLog(const char* fmt, ...);
    void Draw(const char* title, bool* p_open);

private:
    char                  InputBuf[256];
    ImVector<char*>       Items;
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos = -1;    // -1: new line, 0..History.Size-1 browsing history
    ImGuiTextFilter       Filter;
    bool                  AutoScroll = true;

    int   TextEditCallback(ImGuiInputTextCallbackData* data);
    void  ExecCommand(const char* command_line);
};
