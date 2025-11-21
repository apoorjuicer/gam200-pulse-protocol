#pragma once

namespace ScriptAPI
{
    public ref class Script abstract
    {
      public:
        void virtual Update() {};

        // Store which entity this script is attached too
        property unsigned int EntityID;
    };
}