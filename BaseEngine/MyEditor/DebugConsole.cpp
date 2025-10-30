#include "DebugConsole.h"
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

static int Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }

DebugConsole::DebugConsole()
{
    Clear();
    Commands.push_back("HELP");
    Commands.push_back("CLEAR");
    Commands.push_back("HISTORY");
}

DebugConsole::~DebugConsole() { Clear(); }

void DebugConsole::Clear()
{
    for (int i = 0; i < Items.Size; i++)
        free(Items[i]);
    Items.clear();
}

void DebugConsole::AddLog(const char* fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Items.push_back(strdup(buf));
}

void DebugConsole::ExecCommand(const char* command_line)
{
    AddLog("> %s", command_line);

    if (Stricmp(command_line, "CLEAR") == 0)
        Clear();
    else if (Stricmp(command_line, "HELP") == 0)
        AddLog("Commands: CLEAR, HELP, HISTORY");
    else
        AddLog("Unknown command: '%s'", command_line);

    // Add to history
    History.push_back(strdup(command_line));
}

void DebugConsole::Draw(const char* title, bool* p_open)
{
    if (!ImGui::Begin(title, p_open))
    {
        ImGui::End();
        return;
    }

    // Log window
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false))
    {
        for (int i = 0; i < Items.Size; i++)
        {
            const char* item = Items[i];
            if (!Filter.PassFilter(item))
                continue;
            ImGui::TextUnformatted(item);
        }
        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();

    // Input field
    if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf),
        ImGuiInputTextFlags_EnterReturnsTrue))
    {
        ExecCommand(InputBuf);
        strcpy(InputBuf, "");
    }

    ImGui::End();
}
