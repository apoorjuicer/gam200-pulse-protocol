///******************************************************************************/
///**
// * @file        FSMJSON.h
// * @project     Pulse Protocol
// * @author		Reginald Lew Yee Ren
// * @brief		
// *
// * @copyright   Copyright (C) 2026 DigiPen Institute of Technology.
// *              Reproduction or disclosure of this file or its contents without the
// *              prior written consent of DigiPen Institute of Technology is prohibited.
// */
// /******************************************************************************/
//#pragma once
//#include <pch/pch_temp.h>
//
////Serialize
//#pragma warning(push, 0)
//#include <rapidjson/document.h>     // for Document, Value, AllocatorType
//#include <rapidjson/prettywriter.h>
//#include <rapidjson/istreamwrapper.h>
//#include <rapidjson/stringbuffer.h> // for StringBuffer
//#pragma warning(pop)
//
//
////for .NET
//#include "CoreEngine/Core/ImportExport.h" // for DLL_API
//#pragma warning(push)
//#pragma warning(disable: 4251)
//
//struct DLL_API FSM
//{
//	//    std::string ScriptName;
//	std::vector<std::string> FSM_Container;
//
//	void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
//	{
//        out.SetObject();
//
//        rapidjson::Value FSMArray(rapidjson::kArrayType);
//
//        for (const auto& FSM_Data : FSM_Container)
//        {
//            rapidjson::Value nameVal(FSM_Data.c_str(), alloc);
//            FSMArray.PushBack(nameVal, alloc);
//        }
//
//        out.AddMember("FSMContainer", FSMArray, alloc);
//	}
//
//	void Deserialize(const rapidjson::Value& in)
//	{
//        if (!in.IsObject()) throw std::runtime_error("Invalid JSON for LogicComponent");
//
//        FSM_Container.clear();
//
//        if (in.HasMember("FSMContainer") && in["FSMContainer"].IsArray())
//        {
//            const auto& arr = in["FSMContainer"];
//            for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
//            {
//                if (arr[i].IsString())
//                {
//                    FSM_Container.push_back(arr[i].GetString());
//                }
//            }
//        }
//	}
//};
