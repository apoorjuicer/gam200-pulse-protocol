#ifndef TESTAPI_H
#define TESTAPI_H

namespace ScriptAPI
{
    // This is a C++/CLI class (note the "ref class")
    public ref class TestAPI
    {
    public:
        static void SayHello();
        static int Add(int a, int b);
    };
}

#endif // !TESTAPI_H