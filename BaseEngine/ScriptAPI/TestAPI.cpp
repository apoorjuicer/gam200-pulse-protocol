#include "TestAPI.h"

#include <iostream>

namespace ScriptAPI
{
    void TestAPI::SayHello()
    {
        std::cout << "[C++/CLI Bridge] Hello from ScriptAPI!" << std::endl;
    }

    int TestAPI::Add(int a, int b)
    {
        std::cout << "[C++/CLI Bridge] Adding " << a << " + " << b << std::endl;
        return a + b;
    }
}