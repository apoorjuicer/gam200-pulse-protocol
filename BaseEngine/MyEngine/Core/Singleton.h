#pragma once
#include "pch/pch.h"

// allows it so that once it is made, it will never create another instance of that system again
template<typename T>
class Singleton
{
public:
    // make an instance of g_coordinator
    static T* GetInstance()
    {
        // creates instance once
        static T instance; 

        return &instance; 
    }

protected:
    Singleton() {};
    virtual ~Singleton() = default;
};