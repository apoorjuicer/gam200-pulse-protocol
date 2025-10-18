/******************************************************************************/
/**
 * @file        MessageManager.cpp
 * @project     Pulse Protocol
 * @author      Chloe Lau Rey En
 * @brief		Implements a singleton message manager that allows subscribing,
                unsubscribing, publishing, and broadcasting engine messages.
 * @copyright   Copyright (C) 2025 DigiPen Institute of Technology.
 *              Reproduction or disclosure of this file or its contents without the
 *              prior written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/

#include "pch/pch.h"

MessageManager* MessageManager::instance = nullptr;

MessageManager* MessageManager::GetInstance() {
    if (!instance) {
        instance = new MessageManager();
    }
    return instance;
}

void MessageManager::DestroyInstance() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

void MessageManager::Subscribe(Message::Type type, const std::string& listenerName, MessageCallback callback) {
    subscribers[type][listenerName] = callback;
}

void MessageManager::Unsubscribe(Message::Type type, const std::string& listenerName) {
    if (subscribers.find(type) != subscribers.end()) {
        subscribers[type].erase(listenerName);
    }
}

void MessageManager::Publish(const Message& message) {
    auto it = subscribers.find(message.type);
    if (it != subscribers.end()) {
        for (const auto& [name, callback] : it->second) {
            callback(message);
        }
    }
}

void MessageManager::Broadcast(const Message& message) {
    for (auto& [type, listeners] : subscribers) {
        for (const auto& [name, callback] : listeners) {
            callback(message);
        }
    }
}