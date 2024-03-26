#include "doctest.h"
#include <eti/eti.h>

#include <iostream>

namespace eti
{

    template<typename T1>
    struct TypeOfImpl<std::vector<T1>> 
    { 
        static const ::eti::Type& GetTypeStatic() 
        {
            static bool initializing = false;
            static ::eti::Type type;
            if (initializing == false)
            {
                initializing = true;

                static std::vector<::eti::Method> methods =
                {
                    ::eti::internal::MakeMethod("size", false, true, TypeOf<std::vector<T1>>(), [](void* obj, void* ret, std::span<void*>)
                    {
                        std::vector<T1>* vector = (std::vector<T1>*)obj;
                        size_t* r = (size_t*) ret;
                        *r = vector->size();
                    }, ::eti::internal::GetVariableInstance<size_t>(""), {}, {})
                };
                
                type = ::eti::internal::MakeType<std::vector<T1>>(::eti::Kind::Template, nullptr, {}, methods);
            }

            return type;
        }
    };
}

using namespace eti;

namespace playground
{
    TEST_CASE("playground")
    {
        const Type& type = TypeOf<std::vector<int>>();
        std::cout << type.Name;
    }
}
