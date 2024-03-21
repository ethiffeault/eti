//  MIT License
//  
//  Copyright (c) 2024 Eric Thiffeault
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <eti/eti.h>

#if !ETI_SLIM_MODE

using namespace eti;

////////////////////////////////////////////////////////////////////////////////
namespace test_01
{
    struct Foo{};

    TEST_CASE("test_01")
    {
        std::string fooTypeName(GetTypeName<Foo>());
        REQUIRE(fooTypeName == "struct test_01::Foo");

        TypeId fooNameHash =  ETI_HASH_FUNCTION(GetTypeName<Foo>());

        TypeId fooTypeId = GetTypeId<Foo>();
        REQUIRE(fooTypeId == fooNameHash);
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_02
{
    TEST_CASE("test_02")
    {
        const Type& type = TypeOf<int>();
        REQUIRE(type.Name == "i32");
        REQUIRE(type.Id != 0);
        REQUIRE(type.Kind == Kind::Pod);
        REQUIRE(type.Size == sizeof(int));
        REQUIRE(type.Align == alignof(int));
        REQUIRE(type.Parent == nullptr);
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_03
{
    struct Foo
    {
        ETI_STRUCT_SLIM(Foo)
    };
}

namespace test_03
{
    TEST_CASE("test_03")
    {
        const Type& type = TypeOf<Foo>();
        REQUIRE(type.Name == "struct test_03::Foo");
        REQUIRE(type.Id != 0);
        REQUIRE(type.Kind == Kind::Struct);
        REQUIRE(type.Size == sizeof(Foo));
        REQUIRE(type.Align == alignof(Foo));
        REQUIRE(type.Parent == nullptr);
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_04
{
    class Object
    {
        ETI_BASE(Object, ETI_PROPERTIES(), ETI_METHODS())

    public:
        Object(){}
        virtual ~Object(){}
    };

    TEST_CASE("test_04")
    {
        const Type& type = TypeOf<Object>();
        REQUIRE(type.Name == "class test_04::Object");
        REQUIRE(type.Id != 0);
        REQUIRE(type.Kind == Kind::Class);
        REQUIRE(type.Size == sizeof(Object));
        REQUIRE(type.Align == alignof(Object));
        REQUIRE(type.Parent == nullptr);
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_05
{
    class Object
    {
        ETI_BASE(Object, ETI_PROPERTIES(), ETI_METHODS())

    public:
        Object(){}
        virtual ~Object(){}
    };

    class Foo : public Object
    {
        ETI_CLASS(Foo, Object, ETI_PROPERTIES(), ETI_METHODS())

    public:
        Foo(){}
        ~Foo() override{}
    };

    TEST_CASE("test_05")
    {
        {
            const Type& type = TypeOf<Object>();
            REQUIRE(type.Name == "class test_05::Object");
            REQUIRE(type.Id != 0);
            REQUIRE(type.Kind == Kind::Class);
            REQUIRE(type.Size == sizeof(Object));
            REQUIRE(type.Align == alignof(Object));
            REQUIRE(type.Parent == nullptr);
        }

        {
            const Type& type = TypeOf<Foo>();
            REQUIRE(type.Name == "class test_05::Foo");
            REQUIRE(type.Id != 0);
            REQUIRE(type.Kind == Kind::Class);
            REQUIRE(type.Size == sizeof(Object));
            REQUIRE(type.Align == alignof(Object));
            REQUIRE(type.Parent != nullptr);
            REQUIRE(*type.Parent == TypeOf<Object>());
        }

        {
            Object o;
            Foo f;

            REQUIRE(IsA<Object>(o));
            REQUIRE(IsA<Object>(f));

            REQUIRE(!IsA<Foo>(o));
            REQUIRE(IsA<Foo>(f));
        }

        {
            REQUIRE(IsATyped<Object, Object>());
            REQUIRE(IsATyped<Foo, Object>());

            REQUIRE(!IsATyped<Object, Foo>());
            REQUIRE(IsATyped<Foo, Foo>());            
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_06
{
    int construct = 0;
    int copyConstruct = 0;
    int moveConstruct = 0;
    int destruct = 0;

    struct Foo
    {
        ETI_STRUCT_SLIM(Foo)

            static constexpr int IntValue = 1;

        Foo()
        {
            ++construct;
            ptrInt = (int*)malloc(sizeof(int));
            *ptrInt = IntValue;
        }

        Foo(const Foo& foo)
        {
            copyConstruct++;
            ptrInt = (int*)malloc(sizeof(int));
            *ptrInt = *foo.ptrInt;
        }

        Foo(Foo&& foo)
        {
            moveConstruct++;
            ptrInt = foo.ptrInt;
            foo.ptrInt = nullptr;
        }

        ~Foo()
        {
            destruct++;
            if (ptrInt != nullptr)
            {
                free(ptrInt);
                ptrInt = nullptr;
            }
        }

        int* ptrInt = nullptr;
    };
    

    TEST_CASE("test_06")
    {
        const Type& type = TypeOf<Foo>();

        // standard behavior
        {
            construct = 0;
            copyConstruct = 0;
            moveConstruct = 0;
            destruct = 0;
            {
                Foo foo1;
                Foo foo2(std::move(foo1));
            }
            REQUIRE(construct == 1);
            REQUIRE(copyConstruct == 0);
            REQUIRE(moveConstruct == 1);
            REQUIRE(destruct == 2);
        }

        // construct / destruct
        {
            construct = 0;
            copyConstruct = 0;
            moveConstruct = 0;
            destruct = 0;

            Foo* foo = (Foo*)malloc(type.Size);
            memset(foo, 0, sizeof(Foo));
            REQUIRE(construct == 0);
            REQUIRE(destruct == 0);
            REQUIRE(foo->ptrInt == nullptr);

            type.Construct(foo);
            REQUIRE(construct == 1);
            REQUIRE(destruct == 0);
            REQUIRE(foo->ptrInt != nullptr);
            REQUIRE(*foo->ptrInt == Foo::IntValue);

            type.Destruct(foo);
            REQUIRE(construct == 1);
            REQUIRE(destruct == 1);
            REQUIRE(foo->ptrInt == nullptr);

            free(foo);

            REQUIRE(copyConstruct == 0);
            REQUIRE(moveConstruct == 0);
        }

        // construct / copy construct / destruct
        {
            construct = 0;
            copyConstruct = 0;
            moveConstruct = 0;
            destruct = 0;

            Foo* foo1 = (Foo*)malloc(type.Size);
            memset(foo1, 0, sizeof(Foo));
            Foo* foo2 = (Foo*)malloc(type.Size);
            memset(foo2, 0, sizeof(Foo));

            type.Construct(foo1);
            REQUIRE(foo1->ptrInt != nullptr);
            REQUIRE(*foo1->ptrInt == Foo::IntValue);

            type.CopyConstruct(foo1, foo2);
            REQUIRE(*foo2->ptrInt == Foo::IntValue);

            type.Destruct(foo1);
            REQUIRE(foo1->ptrInt == nullptr);

            type.Destruct(foo2);
            REQUIRE(foo2->ptrInt == nullptr);

            free(foo1);
            free(foo2);

            REQUIRE(construct == 1);
            REQUIRE(copyConstruct == 1);
            REQUIRE(moveConstruct == 0);
            REQUIRE(destruct == 2);
        }

        // construct / move / destruct
        {
            construct = 0;
            copyConstruct = 0;
            moveConstruct = 0;
            destruct = 0;

            Foo* foo1 = (Foo*)malloc(type.Size);
            memset(foo1, 0, sizeof(Foo));
            Foo* foo2 = (Foo*)malloc(type.Size);
            memset(foo2, 0, sizeof(Foo));

            type.Construct(foo1);
            REQUIRE(foo1->ptrInt != nullptr);
            REQUIRE(*foo1->ptrInt == Foo::IntValue);

            type.MoveConstruct(foo1, foo2);

            REQUIRE(foo1->ptrInt == nullptr);
            type.Destruct(foo1);
            REQUIRE(foo1->ptrInt == nullptr);
            type.Destruct(foo2);
            REQUIRE(foo2->ptrInt == nullptr);

            REQUIRE(construct == 1);
            REQUIRE(copyConstruct == 0);
            REQUIRE(moveConstruct == 1);
            REQUIRE(destruct == 2);

            free(foo1);
            free(foo2);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_07
{
    TEST_CASE("test_07")
    {
        int i;
        int& iRef = i;
        const int& iConstRef = i;
        int* iPtr = nullptr;
        const int* iConstPtr = nullptr;
        int const * iPtrConst = nullptr;

        std::string_view iName          = GetTypeName<decltype(i)>();
        std::string_view iRefName       = GetTypeName<decltype(iRef)>();
        std::string_view iConstRefName  = GetTypeName<decltype(iConstRef)>();
        std::string_view iPtrName       = GetTypeName<decltype(iPtr)>();
        std::string_view iConstPtrName  = GetTypeName<decltype(iConstPtr)>();
        std::string_view iPtrConstName  = GetTypeName<decltype(iPtrConst)>();

        REQUIRE(iName         == "i32");
        REQUIRE(iRefName      == "i32");
        REQUIRE(iConstRefName == "i32");
        REQUIRE(iPtrName      == "i32");
        REQUIRE(iConstPtrName == "i32");
        REQUIRE(iPtrConstName == "i32");
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_08
{
    struct Foo
    {
        ETI_STRUCT(Foo,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(i),
                ETI_PROPERTY(f),
                ETI_PROPERTY(ptr),
                ETI_PROPERTY(fv)
            ),
            ETI_METHODS()
        )

        int i = 0;
        float f = 0.0f;
        int* ptr = nullptr;
        std::vector<float> fv;
    };

    TEST_CASE("test_08")
    {
        const Type& type = TypeOf<Foo>();
        REQUIRE(type.Properties.size() == 4);
        REQUIRE(type.Properties[0].Variable.Name == "i");
        REQUIRE(type.Properties[0].Offset == 0);
        REQUIRE(type.Properties[0].Variable.Declaration.IsPtr == false);
        REQUIRE(type.Properties[0].Variable.Declaration.Type.Id == TypeOf<int>().Id);

        REQUIRE(type.Properties[1].Variable.Name == "f");
        REQUIRE(type.Properties[1].Offset == 4);
        REQUIRE(type.Properties[1].Variable.Declaration.IsPtr == false);
        REQUIRE(type.Properties[1].Variable.Declaration.Type.Id == TypeOf<float>().Id);

        REQUIRE(type.Properties[2].Variable.Name == "ptr");
        REQUIRE(type.Properties[2].Offset == 8);
        REQUIRE(type.Properties[2].Variable.Declaration.IsPtr == true);
        REQUIRE(type.Properties[2].Variable.Declaration.Type.Id == TypeOf<int>().Id);

        REQUIRE(type.Properties[3].Variable.Name == "fv");
        REQUIRE(type.Properties[3].Offset == 16);
        REQUIRE(type.Properties[3].Variable.Declaration.IsPtr == false);
        REQUIRE(type.Properties[3].Variable.Declaration.Type.Id == TypeOf<std::vector<float>>().Id);
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_09
{
    struct Foo
    {
        ETI_STRUCT(Foo, 
            ETI_PROPERTIES(), 
            ETI_METHODS(
                ETI_METHOD(GetI),
                ETI_METHOD(SetI)
            ))

        int GetI()
        {
            return i;
        }

        void SetI(int value)
        {
            i = value;
        }

        int i = 3;
    };

    TEST_CASE("test_09")
    {
        {
            const Method* method = TypeOf<Foo>().GetMethod("GetI");
            REQUIRE(method != nullptr);
            Foo foo;
            int ret = 0;
            method->Function(&foo, &ret, {});
            REQUIRE(foo.i == ret);
        }

        {
            const Method* method = TypeOf<Foo>().GetMethod("SetI");
            REQUIRE(method != nullptr);
            REQUIRE(method->Arguments.size() == 1);
            REQUIRE(method->Arguments[0].Declaration.Type == TypeOf<int>());
            Foo foo;
            std::vector<void*> args;
            int value = 99;
            args.push_back(&value);
            
            method->Function(&foo, nullptr, args);
            REQUIRE(foo.i == value);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_10
{
    class AccessAttribute : public eti::Attribute
    {
        ETI_CLASS_SLIM(AccessAttribute, Attribute)

    public:

        AccessAttribute(Access access)
        {
            Access = access;
        }

        eti::Access Access = Access::Unknown;
    };

    struct Foo
    {
        ETI_STRUCT(Foo,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(i, AccessAttribute(Access::Private) ),
            ),
            ETI_METHODS()
        )

        int i = 0;
    };

    TEST_CASE("test_09")
    {
        const Type& fooType = TypeOf<Foo>();

        const Property* property = fooType.GetProperty("i");
        REQUIRE(property != nullptr);

        REQUIRE(property->Attributes.size() == 1);
        REQUIRE(IsA<AccessAttribute>(*property->Attributes[0]));

        REQUIRE(property->GetAttribute<AccessAttribute>() != nullptr);
    }
}


////////////////////////////////////////////////////////////////////////////////
namespace test_11
{

    int construct = 0;
    int copyConstruct = 0;
    int moveConstruct = 0;
    int destruct = 0;
    void ResetCounters()
    {
        construct = 0;
        copyConstruct = 0;
        moveConstruct = 0;
        destruct = 0;
    }

    struct Foo
    {
        ETI_STRUCT_SLIM(Foo)

        static constexpr int IntValue = 1;

        Foo()
        {
            ++construct;
        }

        Foo(const Foo& foo)
        {
            copyConstruct++;
        }

        Foo(Foo&& foo)
        {
            moveConstruct++;
        }

        ~Foo()
        {
            destruct++;
        }

        int Int = 123;
    };

    TEST_CASE("test_11")
    {
        const Type& fooType = TypeOf<Foo>();

        {
            ResetCounters();
            REQUIRE(construct == 0);
            REQUIRE(copyConstruct == 0);
            REQUIRE(moveConstruct == 0);
            REQUIRE(destruct == 0);

            Foo* foo = fooType.New<Foo>();
            REQUIRE(foo != nullptr);
            REQUIRE(foo->Int == 123);
            fooType.Delete(foo);

            REQUIRE(construct == 1);
            REQUIRE(copyConstruct == 0);
            REQUIRE(moveConstruct == 0);
            REQUIRE(destruct == 1);
        }

        {
            ResetCounters();
            
            REQUIRE(construct == 0);
            REQUIRE(copyConstruct == 0);
            REQUIRE(moveConstruct == 0);
            REQUIRE(destruct == 0);

            {
                Foo foo1;
                foo1.Int = 321;
                Foo* foo2 = fooType.NewCopy<Foo>(foo1);
                fooType.Delete(foo2);
            }

            REQUIRE(construct == 1);
            REQUIRE(copyConstruct == 1);
            REQUIRE(moveConstruct == 0);
            REQUIRE(destruct == 2);
        }
    }
}

namespace test_12
{

    struct Foo
    {
        void MemberFunction(){}
        static void StaticFunction(){}
    };


    TEST_CASE("test_09")
    {
        constexpr bool isMemberFunctionStatic = IsMethodStatic<decltype(&Foo::MemberFunction)>;
        constexpr bool isStaticFunctionStatic = IsMethodStatic<decltype(&Foo::StaticFunction)>;

        static_assert(isMemberFunctionStatic == false);
        static_assert(isStaticFunctionStatic == true);

    }
}

namespace test_13
{

    struct Foo
    {
        ETI_STRUCT(
            Foo, 
            ETI_PROPERTIES(),
            ETI_METHODS( ETI_METHOD( MemberFunction ) ) )

        void MemberFunction(){}
    };

    TEST_CASE("test_09")
    {
        const Type& type = TypeOf<Foo>();
        const Method* method = type.GetMethod("MemberFunction");
        REQUIRE(method != nullptr);
        REQUIRE(method->Return->Declaration.Type.Kind == Kind::Void);
    }
}
////////////////////////////////////////////////////////////////////////////////
namespace doc_introduction
{
    using namespace eti;

    struct Point
    {
        ETI_STRUCT(
            Point, 
            ETI_PROPERTIES( 
                ETI_PROPERTY( X ), 
                ETI_PROPERTY( Y ) ),
            ETI_METHODS( 
                ETI_METHOD( SetX ), 
                ETI_METHOD( Add ) ) )

        void SetX(int x)
        {
            X = x;
        }

        static Point Add(const Point& p0, const Point& p1)
        {
            return { p0.X + p1.X, p0.Y + p1.Y };
        }

        int X = 0;
        int Y = 0;
    };

    TEST_CASE("doc_introduction")
    {
        const Type& type = TypeOf<Point>();

        // set value using property
        {

            const Property* propertyX = type.GetProperty("X");
            Point p{ 1, 1 };
            propertyX->Set(p, 2);

            // stout: p.x = 2
            std::cout << "p.x = " << p.X << std::endl;       
        }

        // call SetX
        {
            const Method* set = type.GetMethod("SetX");
            Point p{ 1, 1 };
            int value = 101;
            set->CallMethod(p, (void*)nullptr, &value);


            // stout: p.x = 101
            std::cout << "p.x = " << p.X << std::endl;       
        }

        // call static method Add
        {
            const Method* add = type.GetMethod("Add");
            Point p1{ 1, 1 };
            Point p2{ 2, 2 };
            Point result;
            add->CallStaticMethod(&result, &p1, &p2);

            // stout: p1 + p2 = {3, 3}
            std::cout << "p1 + p2 = {" << result.X << ", " << result.Y << "}" << std::endl; 
        }
    }
}

#endif // #if !ETI_SLIM_MODE