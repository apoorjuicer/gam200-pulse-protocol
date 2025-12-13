/******************************************************************************/
/**
* @file        EngineInterface.cpp
* @project     Pulse Protocol
* @author      Chia Wei Xuan Rachael - 100%
* @brief       Manages the lifecycle and execution of C# scripts within the native C++ engine by providing script discovery, 
               instantiation, attachment, and update orchestration through the C++/CLI bridge.
*
* @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
*              Reproduction or disclosure of this file or its contents without the
*              prior written consent of DigiPen Institute of Technology is prohibited.
*
/******************************************************************************/
#include "EngineInterface.h"
#include "CoreEngine/Core/ScriptingBridge.h"
#pragma comment(lib, "PulseEngine.lib")

namespace ScriptAPI
{
    void EngineInterface::Init(CoreEngine* p_coreEngine)
    {
        coreEngine = p_coreEngine;


        // Load assembly
        System::Reflection::Assembly::LoadFrom("ManagedScripts.dll");
        
        updateScriptTypeList();

        // Create the main container that will hold scripts for ALL entities
        scripts = gcnew System::Collections::Generic::List<ScriptList^>();
        const int ENTITY_COUNT = 5000; // same as the one in Types.h 

        // Create an empty script list for each possible entity in the engine
        // Each entity gets its own List<Script^> to hold multiple scripts
        for (int i = 0; i < ENTITY_COUNT; ++i)
        {
            scripts->Add(gcnew ScriptList()); // List<Script^> += ScriptList 
        }
    }
    
    // Update script type list to help LINQ find all Script classes via .NET reflection
    namespace
    {
        // Used to temporarily store an assembly and one of its types together */
        ref struct Pair
        {
            System::Reflection::Assembly^ assembly;
            System::Type^ type;
        };

        // SelectMany helper: Gets all types exported from an assembly 
        System::Collections::Generic::IEnumerable<System::Type^>^ selectorFunc(System::Reflection::Assembly^ assembly)
        {
            return assembly->GetExportedTypes();
        }

        // SelectMany helper: Creates a Pair linking an assembly with one of its types
        Pair^ resultSelectorFunc(System::Reflection::Assembly^ assembly, System::Type^ type)
        {
            Pair^ p = gcnew Pair();
            p->assembly = assembly;
            p->type = type;
            return p;
        }

        // Where filter: Checks if a type is a concrete (non-abstract) Script subclass
        bool predicateFunc(Pair^ pair)
        {
            return pair->type->IsSubclassOf(Script::typeid) && !pair->type->IsAbstract;
        }
        
        // Select helper : Extracts just the Type from a Pair
        System::Type^ selectorFunc(Pair^ pair)
        {
            return pair->type;
        }

    }

    // Discover all available Script classes using .NET reflection
    // This builds a catalog of what scripts exist (PlayerScript, EnemyScript, etc.)
    void EngineInterface::updateScriptTypeList()
    {
        using namespace System;
        using namespace System::Reflection;
        using namespace System::Linq;
        using namespace System::Collections::Generic;

        // STEP 1: Get all loaded assemblies in the current AppDomain
        IEnumerable<Assembly^>^ assemblies = AppDomain::CurrentDomain->GetAssemblies();

        // STEP 2: SelectMany - Get all types from all assemblies and pair them together
        // This flattens: [[Assembly1Types], [Assembly2Types]] → [Type1, Type2, Type3, ...]
        Func<Assembly^, IEnumerable<Type^>^>^ collectionSelector = gcnew Func<Assembly^, IEnumerable<Type^>^>(selectorFunc);
        Func<Assembly^, Type^, Pair^>^ resultSelector = gcnew Func<Assembly^, Type^, Pair^>(resultSelectorFunc);
        IEnumerable<Pair^>^ selectManyResult = Enumerable::SelectMany(assemblies, collectionSelector, resultSelector);
        
        // STEP 3: Where - Filter to keep only concrete Script subclasses
        // Removes abstract classes, non-Script classes, etc.
        Func<Pair^, bool>^ predicate = gcnew Func<Pair^, bool>(predicateFunc);
        IEnumerable<Pair^>^ whereResult = Enumerable::Where(selectManyResult, predicate);
        
        // STEP 4: Select - Extract just the Type objects from the Pairs
        // Result: A list of Type objects representing all usable Script classes
        Func<Pair^, Type^>^ selector = gcnew Func<Pair^, Type^>(selectorFunc);
        scriptTypeList = Enumerable::Select(whereResult, selector);
    }

    // allow the native code to add scripts to different entities via script name by searching the list for a match and 
    // construct an instance of it to be added into the entity's own script list
    bool EngineInterface::AddScriptViaName(int entityId, System::String^ scriptName)
    {
        // Check if valid entityid. our range is 0 - 5000
        if (entityId < 0 || entityId > 5000)
        {
            return false;
        }

        // Remove any whitespaces just in case
        scriptName = scriptName->Trim();

        // Look for the correct script
        System::Type^ scriptType = nullptr;
        for each (System::Type ^ type in scriptTypeList)
        {
            if (type->FullName == scriptName || type->Name == scriptName)
            {
                scriptType = type;
                break;
            }
        }

        // Failed to get any script
        if (scriptType == nullptr)
        {
            return false;
        }

        // Create the script (Default construct an object of the specified type)
        Script^ script = safe_cast<Script^>(System::Activator::CreateInstance(scriptType));

        // make is so ECS ID and script ID is the same
        script->SetEntityID(entityId);

        // Add the script
        scripts[entityId]->Add(script);
        return true;
    }

    // iterate thru all of our scripts that has been created and execute our scripts.
    void EngineInterface::ExecuteUpdate()
    {
        for each (ScriptList ^ entityScriptList in scripts)
        {
            // Update each script available
            for each (Script ^ script in entityScriptList)
            {
                script->Update();
            }
        }
    }

    void EngineInterface::HelloWorld()
    {
        System::Console::WriteLine("Hello Managed World!");
        //coreEngine->HelloWorld();
    }
} 