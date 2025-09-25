#pragma once


// allows it so that once it is made, it will never create another instance of that system again
template<typename T>
class Singleton
{
public:
    // make an instance of g_coordinator
    static T& getInstance()
    {
        static T instance; // creates instance once

        return instance; 
    }
protected:
    Singleton() {}
    ~Singleton() {}
public:
    Singleton(Singleton const&) = delete;
    Singleton& operator=(Singleton const&) = delete;

};