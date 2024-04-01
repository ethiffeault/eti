#include "doctest.h"
#include <eti/eti.h>
#include <iostream>


#define ETI_METHOD_OVERLOAD(NAME, METHOD_TYPE, ...) \
    ::eti::internal::MakeMethod(#NAME, \
        ::eti::utils::IsMethodStatic<decltype((METHOD_TYPE)&Self::NAME)>, \
        ::eti::utils::IsMethodConst<decltype((METHOD_TYPE)&Self::NAME)>, \
        ::eti::TypeOf<Self>(), \
        [](void* obj, void* _return, std::span<void*> args) \
        { \
            ::eti::utils::CallFunction((METHOD_TYPE)&Self::NAME, obj, _return, args); \
        }, \
        ::eti::internal::GetFunctionReturn((METHOD_TYPE)&Self::NAME), \
        ::eti::internal::GetFunctionVariables((METHOD_TYPE)&Self::NAME), \
        ::eti::internal::GetAttributes<::eti::Attribute>(__VA_ARGS__))

#define ETI_TEMPLATE_1_EXTERNAL(TEMPLATE_NAME, PROPERTIES, METHODS, ...) \
    namespace eti \
    { \
        template <typename T1> \
        struct TypeOfImpl<TEMPLATE_NAME<T1>> \
        { \
            static const ::eti::Type& GetTypeStatic() \
            { \
                using Self = TEMPLATE_NAME<T1>; \
                static bool initializing = false; \
                static ::eti::Type type; \
                if (initializing == false) \
                { \
                    initializing = true; \
                    static std::vector<::eti::Property> properties = { PROPERTIES }; \
                    static std::vector<::eti::Method> methods =  { METHODS  }; \
                    type = ::eti::internal::MakeType<Self>(::eti::Kind::Template, nullptr, properties, methods, ::eti::internal::GetTypes<T1>(), ::eti::internal::GetAttributes<::eti::Attribute>(__VA_ARGS__)); \
                } \
                return type; \
            } \
        }; \
    }

typedef int& (std::vector<int>::* AtType)(const size_t);

ETI_TEMPLATE_1_EXTERNAL
(std::vector, 
    ETI_PROPERTIES(), 
    ETI_METHODS
    (
        ETI_METHOD(size),
        //ETI_METHOD_OVERLOAD(at, AtType),
        ETI_METHOD_OVERLOAD(push_back, void (std::vector<T1>::*)(const T1&)),
        ETI_METHOD_OVERLOAD(erase, std::vector<int>::iterator (std::vector<int>::*)(std::vector<int>::const_iterator ))
    )
)


using namespace eti;

namespace playground
{
    TEST_CASE("playground")
    {
        std::vector<int> v;
        int i = 2;

        //std::vector<int>::erase()

        const Type& type = TypeOf<std::vector<int>>();

        typedef void (std::vector<int>::*PushBackType)(const int&);
        //auto ptrFunc =  (void (std::vector<int>::*)(const int&)) & std::vector<int>::push_back;

        //auto ptrFunc2 =  (std::vector<int>::iterator (std::vector<int>::*)(std::vector<int>::const_iterator )) & std::vector<int>::erase;

        //auto ptrFunc3 =  (int& (std::vector<int>::*)(const size_t )) & std::vector<int>::at;

        const Method* pushBack = type.GetMethod("push_back");
        pushBack->CallMethod(v,NoReturn, &i);
        REQUIRE(v.size() == 1);
        REQUIRE(v[0] == 2);

        const Method* erase = type.GetMethod("erase");
        std::vector<int>::iterator itResult;
        std::vector<int>::const_iterator at = v.begin();
        erase->CallMethod(v, &itResult, at);
        REQUIRE(v.size() == 0);

     //   v.at
    }
}
