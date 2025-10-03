/******************************************************************************/
/**
 * @file        MessageManager.h
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief		Defines the messaging system and central manager for
                subscribing, publishing, and broadcasting engine events.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
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