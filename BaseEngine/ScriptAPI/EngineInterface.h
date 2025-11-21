#pragma once
#include "ScriptAPIClass.h"

class CoreEngine;

namespace ScriptAPI
{
    // ref classes are classes in C#, value classes are structs in C#
    public ref class EngineInterface
    {
    public:
        static void Init(CoreEngine* coreEngine);
        static void HelloWorld();
        static bool AddScriptViaName(int entityID, System::String^ scriptName); 
        // ^ = A "handle" to something the garbage collector owns and will clean up for you

        static void ExecuteUpdate();
    
    private:
        using ScriptList = System::Collections::Generic::List<Script^>;
        static System::Collections::Generic::List<ScriptList^>^ scripts;                // stores all scripts for all entities in your scene.
        static System::Collections::Generic::IEnumerable<System::Type^>^ scriptTypeList;// stores all available script types that you can attach to entities.

        // chatgpt easy examplination if u cant picture it (i am dumb thats why):
        // scriptTypeList →[PlayerScript, EnemyScript, CameraScript, ...]  // All available types
        //    ↓
        //    scripts[0] →[PlayerScript instance, CameraScript instance]      // Entity 0's scripts
        //    scripts[1] →[EnemyScript instance]                              // Entity 1's scripts  
        //    scripts[2] →[PlayerScript instance]                             // Entity 2's scripts

        static void updateScriptTypeList();

    internal:
        static CoreEngine* coreEngine = nullptr;
    };
}