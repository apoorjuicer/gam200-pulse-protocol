#pragma once
#include <imgui.h>
#include <vector>
#include <string>

enum class ConsoleLevel { Trace, Debug, Info, Warn, Error, Critical };

class DebugConsole
{
public:
    DebugConsole();
    ~DebugConsole();

    void Clear();

    // Existing printf-style
    void AddLog(const char* fmt, ...);

    // New: explicit color / level helpers
    void AddLogColored(const ImVec4& col, const char* fmt, ...);
    void AddLogLevel(ConsoleLevel lvl, const char* fmt, ...);

    void Draw(const char* title, bool* p_open);

private:
    struct Item {
        ImVec4  color;
        char* text;
    };

    char                  InputBuf[256]{};
    ImVector<Item>        Items;      // colored items now
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos = -1;
    ImGuiTextFilter       Filter;
    bool                  AutoScroll = true;

    int   TextEditCallback(ImGuiInputTextCallbackData* data);
    void  ExecCommand(const char* command_line);

    static ImVec4 ColorForLevel(ConsoleLevel lvl);
    static const char* LevelToStr(ConsoleLevel lvl);
};
