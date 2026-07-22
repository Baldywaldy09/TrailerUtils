#pragma once

#include <cstdint>
#include <cstring>


#pragma pack(push, 1)
namespace prism
{
    class string_dyn_t      // Size: 0x0018
    {
    public:
        void* vtable_ptr;   //0x0000 (0x08)
        char* string;       //0x0008 (0x08)
        uint32_t size;      //0x0010 (0x04)
        uint32_t capacity;  //0x0014 (0x04)
    };
    static_assert(sizeof(string_dyn_t) == 0x18);


    // basically a const char** (if a prism::string*)
    class string            // Size: 0x08
    {
    public:
        const char* value;  //0x0000 (0x08)

        string(const char* str) {
            size_t len = strlen(str) + 1;
            value = new char[len];
            strcpy_s(const_cast<char*>(value), len, str);
        }
    };
    static_assert(sizeof(string) == 0x08);
};
#pragma pack(pop)