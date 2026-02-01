#define generic generic_workaround

#include "ToggleScreenAPI.h"

// Access the global CoreEngine* Engine + fullscreen functions.
// __has_include makes it resilient to different header paths in your project.
#if __has_include("CoreEngine.h")
  #include "CoreEngine.h"
#elif __has_include("CoreEngine/CoreEngine.h")
  #include "CoreEngine/CoreEngine.h"
#elif __has_include("CoreEngine/Core/CoreEngine.h")
  #include "CoreEngine/Core/CoreEngine.h"
#else
  #include "CoreEngine.h" // last-resort; adjust include path if needed
#endif

#undef generic

namespace ScriptAPI
{
    void ToggleScreenAPI::ToggleFullscreen()
    {
        if (!Engine) return;
        Engine->ToggleFullscreen();
    }

    void ToggleScreenAPI::SetFullscreen(bool fullscreen)
    {
        if (!Engine) return;

        // CoreEngine::SetFullscreen is private in your engine,
        // so we toggle only if current state differs.
        if (Engine->IsFullscreen() == fullscreen)
            return;

        Engine->ToggleFullscreen();
    }

    bool ToggleScreenAPI::IsFullscreen()
    {
        if (!Engine) return false;
        return Engine->IsFullscreen();
    }
}
