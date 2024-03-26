#include "doctest.h"
#include <eti/eti.h>
#include <iostream>

//enum class MyEnum : std::uint8_t
//{
//    Value1,
//    Value2,
//};
//
//static constexpr size_t MyEnumSize = 2;
//static constexpr std::string_view MyEnumName[MyEnumSize] = { "Value1", "Value2" };
//
//namespace eti
//{
//    template <>
//    struct TypeOfImpl<MyEnum>
//    {
//        static const ::eti::Type& GetTypeStatic()
//        {
//            using Self = MyEnum;
//            static bool initializing = false;
//            static ::eti::Type type;
//            if (initializing == false)
//            {
//                initializing = true;
//                type = ::eti::internal::MakeType<Self>(::eti::Kind::Enum, nullptr, {});
//            }
//            return type;
//        }
//    };
//}


namespace playground
{
    TEST_CASE("playground")
    {
    }
}
