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

#include <iostream>

#include "doctest.h"

#include <eti/eti.h>

using namespace eti;

////////////////////////////////////////////////////////////////////////////////
namespace test_01
{
    struct Foo {};

    TEST_CASE("test_01")
    {
        std::string fooTypeName(GetTypeName<Foo>());
        REQUIRE(fooTypeName == "test_01::Foo");

        TypeId fooNameHash = ETI_HASH_FUNCTION(GetTypeName<Foo>());

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
        REQUIRE(type.Name == "s32");
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
        ETI_STRUCT(Foo)
    };
}

namespace test_03
{
    TEST_CASE("test_03")
    {
        const Type& type = TypeOf<Foo>();
        REQUIRE(type.Name == "test_03::Foo");
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
        ETI_BASE_EXT(Object, ETI_PROPERTIES(), ETI_METHODS())

    public:
        Object() {}
        virtual ~Object() {}
    };

    TEST_CASE("test_04")
    {
        const Type& type = TypeOf<Object>();
        REQUIRE(type.Name == "test_04::Object");
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
        ETI_BASE_EXT(Object, ETI_PROPERTIES(), ETI_METHODS())

    public:
        Object() {}
        virtual ~Object() {}
    };

    class Foo : public Object
    {
        ETI_CLASS_EXT(Foo, Object, ETI_PROPERTIES(), ETI_METHODS())

    public:
        Foo() {}
        ~Foo() override {}
    };

    TEST_CASE("test_05")
    {
        {
            const Type& type = TypeOf<Object>();
            REQUIRE(type.Name == "test_05::Object");
            REQUIRE(type.Id != 0);
            REQUIRE(type.Kind == Kind::Class);
            REQUIRE(type.Size == sizeof(Object));
            REQUIRE(type.Align == alignof(Object));
            REQUIRE(type.Parent == nullptr);
        }

        {
            const Type& type = TypeOf<Foo>();
            REQUIRE(type.Name == "test_05::Foo");
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
        ETI_STRUCT(Foo)

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
        int const* iPtrConst = nullptr;

        std::string_view iName = GetTypeName<decltype(i)>();
        std::string_view iRefName = GetTypeName<decltype(iRef)>();
        std::string_view iConstRefName = GetTypeName<decltype(iConstRef)>();
        std::string_view iPtrName = GetTypeName<decltype(iPtr)>();
        std::string_view iConstPtrName = GetTypeName<decltype(iConstPtr)>();
        std::string_view iPtrConstName = GetTypeName<decltype(iPtrConst)>();

        REQUIRE(iName == "s32");
        REQUIRE(iRefName == "s32");
        REQUIRE(iConstRefName == "s32");
        REQUIRE(iPtrName == "s32");
        REQUIRE(iConstPtrName == "s32");
        REQUIRE(iPtrConstName == "s32");
    }
}

////////////////////////////////////////////////////////////////////////////////
namespace test_08
{
    struct Foo
    {
        ETI_STRUCT_EXT(Foo,
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
        ETI_STRUCT_EXT(Foo,
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
    struct Foo
    {
        ETI_STRUCT_EXT(Foo,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(i, Accessibility(Access::Private)),
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
        REQUIRE(IsA<Accessibility>(*property->Attributes[0]));

        REQUIRE(property->GetAttribute<Accessibility>() != nullptr);
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
        ETI_STRUCT(Foo)

            static constexpr int IntValue = 1;

        Foo()
        {
            ++construct;
        }

        Foo(const Foo&)
        {
            copyConstruct++;
        }

        Foo(Foo&&)
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
        void MemberFunction() {}
        static void StaticFunction() {}
    };


    TEST_CASE("test_09")
    {
        constexpr bool isMemberFunctionStatic = utils::IsMethodStatic<decltype(&Foo::MemberFunction)>;
        constexpr bool isStaticFunctionStatic = utils::IsMethodStatic<decltype(&Foo::StaticFunction)>;

        static_assert(isMemberFunctionStatic == false);
        static_assert(isStaticFunctionStatic == true);

    }
}

namespace test_13
{

    struct Foo
    {
        ETI_STRUCT_EXT(
            Foo,
            ETI_PROPERTIES(),
            ETI_METHODS(ETI_METHOD(MemberFunction)))

            void MemberFunction() {}
    };

    TEST_CASE("test_13")
    {
        const Type& type = TypeOf<Foo>();
        const Method* method = type.GetMethod("MemberFunction");
        REQUIRE(method != nullptr);
        REQUIRE(method->Return->Declaration.Type.Kind == Kind::Void);
    }
}


namespace test_14
{

    struct Foo
    {
        ETI_STRUCT_EXT(
            Foo,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(intValue),
                ETI_PROPERTY(intConstValue),
                ETI_PROPERTY(intPtr),
                ETI_PROPERTY(intConstPtr),
                ),
            ETI_METHODS())

            int intValue;
        const int intConstValue;

        int* intPtr;
        const int* intConstPtr;
    };

    TEST_CASE("test_14")
    {
        {
            const Property* p = TypeOf<Foo>().GetProperty("intValue");
            const Declaration& decl = p->Variable.Declaration;

            REQUIRE(decl.IsValue == true);
            REQUIRE(decl.IsConst == false);
            REQUIRE(decl.IsPtr == false);
            REQUIRE(decl.IsRef == false);
        }

        {
            const Property* p = TypeOf<Foo>().GetProperty("intConstValue");
            const Declaration& decl = p->Variable.Declaration;

            REQUIRE(decl.IsValue == true);
            REQUIRE(decl.IsConst == true);
            REQUIRE(decl.IsPtr == false);
            REQUIRE(decl.IsRef == false);
        }

        {
            const Property* p = TypeOf<Foo>().GetProperty("intPtr");
            const Declaration& decl = p->Variable.Declaration;

            REQUIRE(decl.IsValue == false);
            REQUIRE(decl.IsConst == false);
            REQUIRE(decl.IsPtr == true);
            REQUIRE(decl.IsRef == false);
        }

        {
            const Property* p = TypeOf<Foo>().GetProperty("intConstPtr");
            const Declaration& decl = p->Variable.Declaration;

            REQUIRE(decl.IsValue == false);
            REQUIRE(decl.IsConst == true);
            REQUIRE(decl.IsPtr == true);
            REQUIRE(decl.IsRef == false);
        }
    }
}

namespace test_15
{

    struct Foo
    {
        ETI_STRUCT_EXT(
            Foo,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(intValue),
                ETI_PROPERTY(intConstValue),
                ETI_PROPERTY(intPtr),
                ETI_PROPERTY(intConstPtr),
                ),
            ETI_METHODS())

            int intValue = 0;
        const int intConstValue = 1;

        int* intPtr = nullptr;
        const int* intConstPtr = nullptr;
    };

    TEST_CASE("test_15")
    {

        int intValue = 1;
        int intConstValue = 1;
        int* intPtr = nullptr;
        const int* intConstPtr = nullptr;

        int someValue = 101;

        Foo foo = { intValue, intConstValue, intPtr, intConstPtr };

        {
            const Property* p = TypeOf<Foo>().GetProperty("intValue");
            void* ptr = p->UnSafeGetPtr(&foo);
            REQUIRE(ptr == &foo.intValue);

            p->Set(foo, 12);
            REQUIRE(foo.intValue == 12);

        }

        {
            const Property* p = TypeOf<Foo>().GetProperty("intConstValue");
            void* ptr = p->UnSafeGetPtr(&foo);
            REQUIRE(ptr == &foo.intConstValue);

            p->Set(foo, 12);
            REQUIRE(foo.intConstValue == 12);

        }

        {
            const Property* p = TypeOf<Foo>().GetProperty("intPtr");
            void* ptr = p->UnSafeGetPtr(&foo);
            REQUIRE(ptr == &foo.intPtr);

            p->Set(foo, &someValue);
            REQUIRE(*foo.intPtr == someValue);

        }

        {
            const Property* p = TypeOf<Foo>().GetProperty("intConstPtr");
            void* ptr = p->UnSafeGetPtr(&foo);
            REQUIRE(ptr == &foo.intConstPtr);

            p->Set(foo, &someValue);
            REQUIRE(*foo.intConstPtr == someValue);
        }
    }
}

namespace test_16
{
    class Foo;
    class Doo;

    class Object
    {
        ETI_BASE_EXT(Object, ETI_PROPERTIES
        (
            ETI_PROPERTY(ObjectPtr),
            ETI_PROPERTY(FooPtr),
            ETI_PROPERTY(DooPtr),
            ),
            ETI_METHODS())

    public:

        Object* ObjectPtr = nullptr;
        Foo* FooPtr = nullptr;
        Doo* DooPtr = nullptr;

        Object() {}
        virtual ~Object() {}
    };

    class Foo : public Object
    {
        ETI_CLASS(Foo, Object)
    };

    class Doo : public Object
    {
        ETI_CLASS(Foo, Object)
    };

    TEST_CASE("test_16")
    {
        Object object;
        Foo foo;
        Doo doo;

        const Type& objectType = Object::GetTypeStatic();
        const Property* objectPtrProperty = objectType.GetProperty("ObjectPtr");

        objectPtrProperty->Set(object, &object);
        REQUIRE(object.ObjectPtr == &object);
        Object* obj;
        objectPtrProperty->Get(object, obj);
        REQUIRE(obj == object.ObjectPtr);

        objectPtrProperty->Set(object, &foo);
        REQUIRE(object.ObjectPtr == &foo);
        objectPtrProperty->Get(object, obj);
        REQUIRE(obj == object.ObjectPtr);

        objectPtrProperty->Set(object, &doo);
        REQUIRE(object.ObjectPtr == &doo);
        objectPtrProperty->Get(object, obj);
        REQUIRE(obj == object.ObjectPtr);
    }
}

namespace test_17
{
    class Foo;
    class Doo;

    class Object
    {
        ETI_BASE_EXT
        (
            Object,
            ETI_PROPERTIES(ETI_PROPERTY(I)),
            ETI_METHODS
            (
                ETI_METHOD(GetName),
                ETI_METHOD(Add),
                ETI_METHOD(GetIPtr, Accessibility(Access::Public))
            ),
            Accessibility(Access::Public)
        )

    public:
        virtual ~Object() {}

        static double Add(int n0, float n1) { return n0 + n1; }

        int* GetIPtr() { return &I; }
        int I = 12;

    protected:
        virtual std::string_view GetName() { return "my name is Object"; }
    };

    class Foo : public Object
    {
        ETI_CLASS_EXT
        (
            Foo, Object,
            ETI_PROPERTIES(),
            ETI_METHODS()
        )

    public:
        ~Foo() override {}

    protected:
        std::string_view GetName() override { return "my name is Foo"; }
    };

    class Doo : public Object
    {
        ETI_CLASS_EXT
        (
            Doo, Object,
            ETI_PROPERTIES(),
            ETI_METHODS()
        )

    public:
        ~Doo() override {}

    protected:
        std::string_view GetName() override { return "my name is Doo"; }
    };

    TEST_CASE("test_17")
    {
        Object obj;
        Foo foo;
        Doo doo;

        {
            const Method* getNameMethod = TypeOf<Object>().GetMethod("GetName");

            std::string_view name;
            getNameMethod->CallMethod(obj, &name);
            REQUIRE(name == "my name is Object");

            getNameMethod->CallMethod(foo, &name);
            REQUIRE(name == "my name is Foo");

            getNameMethod->CallMethod(doo, &name);
            REQUIRE(name == "my name is Doo");
        }

        {
            const Method* addMethod = TypeOf<Object>().GetMethod("Add");
            double result = 0.0f;
            float p1 = 2.0f;
            addMethod->CallStaticMethod(&result, 1, p1);
            REQUIRE((int)result == 3);
        }

        {
            const Method* getIPtrMethod = TypeOf<Object>().GetMethod("GetIPtr");

            int* getIPtr = nullptr;
            getIPtrMethod->CallMethod(foo, &getIPtr);

            int* iPtr = &foo.I;
            REQUIRE(iPtr == getIPtr);

            *getIPtr = 3;
            REQUIRE(foo.I == 3);

            const Accessibility* accessibility= getIPtrMethod->GetAttribute<Accessibility>();
            REQUIRE(accessibility != nullptr);
            REQUIRE(accessibility->Access == Access::Public);
        }

        {
            const Type& objType = TypeOf<Object>();
            const Accessibility* accessibility = objType.GetAttribute<Accessibility>();
            REQUIRE(accessibility != nullptr);
            REQUIRE(accessibility->Access == Access::Public);

        }
    }
}

namespace third_party
{
    struct Point
    {
        float GetX() { return X; }
        float GetY() { return Y; }
        void SetX(float value) { X = value; }
        void SetY(float value) { Y = value; }
        float X = 0.0f;
        float Y = 0.0f;
    };

    class Base
    {
    public:
        virtual ~Base(){}
        std::string_view GetName() { return "My name is Base"; }
    };

    class Foo
    {
    public:
        std::string_view GetName() { return "My name is Foo"; }
    };
}

ETI_STRUCT_EXTERNAL(::third_party::Point, ETI_PROPERTIES(ETI_PROPERTY(X), ETI_PROPERTY(Y)), ETI_METHODS(ETI_METHOD(GetX), ETI_METHOD(SetX)), eti::Accessibility(eti::Access::Public))
ETI_BASE_EXTERNAL(::third_party::Base, ETI_PROPERTIES(), ETI_METHODS());
ETI_CLASS_EXTERNAL(::third_party::Foo, ::third_party::Base, ETI_PROPERTIES(), ETI_METHODS(ETI_METHOD(GetName)));

namespace test_18
{
    using namespace eti;
    using namespace third_party;

    TEST_CASE("test_18")
    {
        const Type& type = TypeOf<Point>();
        REQUIRE(type.Name == "third_party::Point");
        REQUIRE(type.GetProperty("X") != nullptr);
        REQUIRE(type.GetProperty("Y") != nullptr);
        REQUIRE(type.GetMethod("GetX") != nullptr);
        REQUIRE(type.GetMethod("SetX") != nullptr);

        Point p;
        type.GetProperty("X")->Set(p, 1.0f);
        REQUIRE(p.X == 1.0f);

        type.GetMethod("SetX")->CallMethod(p, NoReturn, 2.0f);
        REQUIRE(p.X == 2.0f);

        float x;
        type.GetMethod("GetX")->CallMethod(p, &x);
        REQUIRE(p.X == 2.0f);

        REQUIRE(type.Attributes.size() == 1);
        REQUIRE(type.GetAttribute<::eti::Accessibility>() != nullptr);
        REQUIRE(type.GetAttribute<::eti::Accessibility>()->Access == ::eti::Access::Public);

        // note: IsA not available on external defined type since cannot add virtual method (will not compile)
        //       but IsATyped is available
        REQUIRE(IsATyped<Foo, Base>());
        REQUIRE(TypeOf<Foo>().GetMethod("GetName") != nullptr);

        Foo foo;
        std::string_view name;
        TypeOf<Foo>().GetMethod("GetName")->CallMethod(foo, &name);
        REQUIRE(name == "My name is Foo");
    }
}


namespace test_20
{
    // const methods test
    struct Point
    {
        ETI_STRUCT_EXT(Point, ETI_PROPERTIES(), ETI_METHODS( ETI_METHOD(GetX) ) )
        int GetX() const { return X; }
        int X = 0;
    };

    TEST_CASE("test_20")
    {
        Point p;
        int x = -1;
        TypeOf<Point>().GetMethod("GetX")->CallMethod(p, &x);
        REQUIRE(x == 0);

    }
}

namespace test_21
{
    using namespace eti;
    struct Foo
    {
        ETI_STRUCT_EXT(Foo, 
            ETI_PROPERTIES
            ( 
                ETI_PROPERTY(Values)
            ), 
            ETI_METHODS())

        std::vector<int> Values;
    };

    void AddValue(const Property* property, void* foo, int value)
    {
        const Type& propertyType = property->Variable.Declaration.Type;

        if (propertyType.Kind == Kind::Template &&
            propertyType.Templates.size() == 1 &&
            *propertyType.Templates[0] == TypeOf<int>()  &&
            propertyType == TypeOf<std::vector<int>>())
        {
            // ok to cast here, we validated type
            std::vector<int>* vector = (std::vector<int>*)property->UnSafeGetPtr(foo);
            vector->push_back(value);
        }
    }

    TEST_CASE("test_21")
    {
        Foo foo;
        AddValue(TypeOf<Foo>().GetProperty("Values"), &foo, 123);
        REQUIRE(foo.Values.size() == 1);
        REQUIRE(foo.Values[0] == 123);
    }
}

namespace test_22
{
    ETI_ENUM
    (
        std::uint8_t, Day,
            Monday,
            Tuesday,
            Wednesday,
            Thursday,
            Friday,
            Saturday,
            Sunday
    )

    struct Time
    {
        ETI_STRUCT_EXT(Time, 
            ETI_PROPERTIES
            (
                ETI_PROPERTY(Day)
            ), ETI_METHODS())
        Day Day = Day::Friday;
    };

    TEST_CASE("test_22")
    {
        Time time;
        Day day;
        TypeOf<Time>().GetProperty("Day")->Get(time, day);
        REQUIRE(day == Day::Friday);
        TypeOf<Time>().GetProperty("Day")->Set(time, Day::Monday);
        REQUIRE(time.Day == Day::Monday);
    }
}
ETI_ENUM_IMPL(test_22::Day)

namespace test_23
{
    struct Time
    {
        ETI_ENUM
        (
            std::uint8_t, Day,
                Monday,
                Tuesday,
                Wednesday,
                Thursday,
                Friday,
                Saturday,
                Sunday
        )

        ETI_STRUCT_EXT(Time, 
            ETI_PROPERTIES
            (
                ETI_PROPERTY(CurrentDay)
            ), ETI_METHODS())

        Day CurrentDay = Day::Friday;
    };

    TEST_CASE("test_23")
    {
        Time time;
        Time::Day day;
        TypeOf<Time>().GetProperty("CurrentDay")->Get(time, day);
        REQUIRE(day == Time::Day::Friday);
        TypeOf<Time>().GetProperty("CurrentDay")->Set(time, Time::Day::Monday);
        REQUIRE(time.CurrentDay == Time::Day::Monday);
    }
}
ETI_ENUM_IMPL(test_23::Time::Day)

namespace test_24
{
    // all kind of return  args
    struct Foo
    {
        ETI_STRUCT_EXT
        (
            Foo,
            ETI_PROPERTIES(),
            ETI_METHODS
            (
                ETI_METHOD(Get), 
                ETI_METHOD(Set),
                ETI_METHOD(GetRef),
                ETI_METHOD(GetPtr),
                ETI_METHOD(GetValueRef),
                ETI_METHOD(SetValueRef),
                ETI_METHOD(GetValuePtr),
                ETI_METHOD(SetValuePtr),
                ETI_METHOD(GetValuePtrPtr)
            )
        )

        int Get() { return Value; }
        void Set(int v) { Value = v; }

        int& GetRef() { return Value; }
        int* GetPtr() { return &Value; }

        void GetValueRef(int& v)
        {
            v = Value;
        }

        void SetValueRef(const int& v) { Value = v; }

        void GetValuePtr(int* v)
        {
            *v = Value;
        }
        void SetValuePtr(const int* v) { Value = *v; }

        void GetValuePtrPtr(int** v)
        {
            *v = &Value;
        }

        int Value = 1;
    };

    TEST_CASE("test_24")
    {
        {
            // int Get() { return Value; }
            Foo foo;
            int value = 0;
            const Method* method = TypeOf<Foo>().GetMethod("Get");
            method->CallMethod(foo, &value);
            REQUIRE(value == 1);
        }
        {
            // void Set(int v) { Value = v; }
            Foo foo;
            const Method* method = TypeOf<Foo>().GetMethod("Set");
            method->CallMethod(foo, NoReturn, 2);
            REQUIRE(foo.Value == 2);
        }
        {
            // int& GetRef() { return Value; }
            Foo foo;
            int* value = nullptr;
            const Method* method = TypeOf<Foo>().GetMethod("GetRef");
            method->CallMethod(foo, &value);
            REQUIRE(*value == 1);
            *value = 2;
            REQUIRE(foo.Value == 2);
        }
        {
            // int& GetPtr() { return Value; }
            Foo foo;
            int* value = nullptr;
            const Method* method = TypeOf<Foo>().GetMethod("GetPtr");
            method->CallMethod(foo, &value);
            REQUIRE(*value == 1);
            *value = 2;
            REQUIRE(foo.Value == 2);
        }
        {
            //void GetValueRef(int& v)  { v = Value; }
            Foo foo;
            int value = 9;
            const Method* method = TypeOf<Foo>().GetMethod("GetValueRef");
            method->CallMethod(foo, NoReturn, &value);
            REQUIRE(value == 1);
        }
        {
            //void SetValueRef(const int& v) { Value = v; }
            Foo foo;
            int value = 9;
            const Method* method = TypeOf<Foo>().GetMethod("SetValueRef");
            method->CallMethod(foo, NoReturn, &value);
            REQUIRE(foo.Value == 9);
        }
        {
            //void GetValuePtr(int* v)  { *v = Value; }
            Foo foo;
            int value = 9;
            const Method* method = TypeOf<Foo>().GetMethod("GetValuePtr");
            method->CallMethod(foo, NoReturn, &value);
            REQUIRE(value == 1);
        }
        {
            //void SetValuePtr(const int* v) { Value = *v; }
            Foo foo;
            int value = 9;
            const Method* method = TypeOf<Foo>().GetMethod("SetValuePtr");
            method->CallMethod(foo, NoReturn, &value);
            REQUIRE(foo.Value == 9);
        }

        {
            // void GetValuePtrPtr(int** v)

            Foo foo;
            int* ptr = nullptr;
            const Method* method = TypeOf<Foo>().GetMethod("GetValuePtrPtr");
            method->CallMethod(foo, NoReturn, &ptr);
            REQUIRE(*ptr == 1);
            *ptr = 12;
            REQUIRE(foo.Value == 12);

        }
    }
}
