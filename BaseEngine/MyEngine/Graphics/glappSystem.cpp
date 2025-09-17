///*********************************************************************************************
// \file    glappSystem.cpp
// \par		Bloop
// \author	Goh Pin Kai ( pinkai.g@digipen.edu )
//
//
//\copyright All content   2024 DigiPen Institute of Technology Singapore. All rights reserved.
//**********************************************************************************************/
//#include "glappSystem.h"
//#include "Transform.h"
////#include "../ComponentJson/files/Composition.h"
////#include "../ComponentJson/Component_System.h"
////#include "../Engine/Core.h"
////
////#include "../Managers/AssetsManager.h"
//
//using namespace Framework;
//
//
//Transform trans;
//
//
//namespace Framework
//{
//    glappSystem::glappSystem() { glapp = new GLApp; };
//    void glappSystem::Initialize() {
//
//        AssetsManager::Instance().LoadAllImage();   
//        
//        GAM200graphics::Log::Init(); 
//
//        // Part 2
//        GLHelper::print_specs();
//            glapp->init(editorWidth,editorHeight);                            
//
//    }
//
//    void glappSystem::NewImage() {
//        transObj.clear(); 
//
//    }
//
//
//
//    void glappSystem::Update(float timeSlice) {
//
//        UNREFERENCED_PARAMETER(timeSlice);
//        //update
//        glfwPollEvents();
//        GLHelper::update_time(1.0);
//
//     glapp->update();                                       
//     glapp->draw();  
//
//      
//
//        if (glfwWindowShouldClose(GLHelper::ptr_window)) {    
//            Message pause(QUIT);
//            CORE->BroadcastMessage(&pause);
//        }
//
//    }
//
//    glappSystem::~glappSystem() {
//        AssetsManager::Instance().UnloadAllImage();   
//        transObj.clear();
//        glapp->cleanup();
//        delete glapp;
//    }
//    std::string glappSystem::GetName() {
//        return "Graphics";
//    }
//    void glappSystem::SendMessageMess(Message* message) {
//
//        //TEMP
//        if (message->messageId==SCENECHANGE) {
//            NewImage();
//        }
//
//        
//    }
//
//
//
//}