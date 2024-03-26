#include "doctest.h"
#include <eti/eti.h>

#include <iostream>

//namespace eti
//{
//    template<typename T>
//    struct TypeNameImpl<std::vector<T>>
//    {
//        static constexpr auto Get()
//        {
//            return "vector<T>";
//        }
//    };
//
//    template<typename T>
//    struct TypeOfImpl<std::vector<T>> 
//    { 
//        static const ::eti::Type& GetTypeStatic() 
//        {
//            static bool initializing = false;
//            static ::eti::Type type;
//            if (initializing == false)
//            {
//                initializing = true;
//
//                static std::vector<::eti::Method> methods =
//                {
//                    ::eti::internal::MakeMethod("size", false, true, TypeOf<std::vector<T>>(), [](void* obj, void* ret, std::span<void*> args)
//                    {
//                        std::vector<T>* vector = (std::vector<T>*)obj;
//                        size_t* r = (size_t*) ret;
//                        *r = vector->size();
//                    }, ::eti::internal::GetVariable<size_t>(""), {}, {})
//                };
//
//                static std::vector<const Type*> templates = { &TypeOf<T>() };
//                type = ::eti::internal::MakeType<std::vector<T>>(::eti::Kind::Template, nullptr, {}, methods, templates);
//            }
//
//            return type;
//        }
//    };
//}
//
//namespace playground
//{
//    TEST_CASE("playground")
//    {
//        const Type& type = TypeOf<std::vector<int>>();
//        std::cout << type.Name;
//    }
//}


namespace playground
{

    TEST_CASE("playground")
    {
    }
}
