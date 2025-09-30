// ==================================================================================
// MessageManager.h
// Authors: Chloe Lau Rey En
// Brief: Generic messaging system for inter-system communication using observer pattern
// 
// USAGE GUIDE:
// ------------
// 1. Define your message type in Message::Type enum
// 2. Create a message class that inherits from Message
// 3. Publisher: Get MessageManager instance and call Publish()
// 4. Subscriber: Get MessageManager instance and call Subscribe() with a callback
//
// EXAMPLE - Sending a message:
//    auto* msgMgr = MessageManager::GetInstance();
//    InputComboMessage msg("Basic Strike", glfwGetTime());
//    msgMgr->Publish(msg);
//
// EXAMPLE - Receiving a message:
//    auto* msgMgr = MessageManager::GetInstance();
//    msgMgr->Subscribe(Message::Type::INPUT_COMBO, "MySystemName", 
//        [](const Message& msg) {
//            const InputComboMessage& combo = static_cast<const InputComboMessage&>(msg);
//            // Handle the message here
//            std::cout << "Received: " << combo.comboName << std::endl;
//        });
//
// ADDING NEW MESSAGE TYPES:
// 1. Add enum value to Message::Type (e.g., ENTITY_SPAWNED)
// 2. Create new message class inheriting from Message
// 3. Systems can now publish/subscribe to this new type
// ==================================================================================
#pragma once
#include "pch/pch.h"

// Base Message class
class Message {
public:
    enum class Type {
        INPUT_RHYTHM,
        INPUT_COMBO,
        COLLISION_HIT,
        AUDIO_BEAT
    };

    Type type;
    double timestamp;

    Message(Type t, double time) : type(t), timestamp(time) {}
    virtual ~Message() = default;
};

// Input combo message
class InputComboMessage : public Message {
public:
    std::string comboName;

    InputComboMessage(const std::string& combo, double time)
        : Message(Type::INPUT_COMBO, time), comboName(combo) {
    }
};

// Collision message
class CollisionMessage : public Message {
public:
    Entity entityA;
    Entity entityB;
    Vector2 collisionPoint;

    CollisionMessage(Entity a, Entity b, Vector2 point, double time)
        : Message(Type::COLLISION_HIT, time), entityA(a), entityB(b), collisionPoint(point) {
    }
};

// MessageManager - Central hub for system communication
class MessageManager {
public:
    using MessageCallback = std::function<void(const Message&)>;

    static MessageManager* GetInstance();
    static void DestroyInstance();

    void Subscribe(Message::Type type, const std::string& listenerName, MessageCallback callback);
    void Unsubscribe(Message::Type type, const std::string& listenerName);
    void Publish(const Message& message);
    void Broadcast(const Message& message);

private:
    MessageManager() = default;
    static MessageManager* instance;

    std::unordered_map<Message::Type, std::unordered_map<std::string, MessageCallback>> subscribers;
};