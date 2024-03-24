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

#include <iostream>

#include "doctest.h"

#include <eti/eti.h>

using namespace eti;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
namespace doc_introduction
{
    using namespace eti;

    struct Point
    {
        ETI_STRUCT(
            Point,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(X)
            ),
            ETI_METHODS
            (
                ETI_METHOD(SetX),
                ETI_METHOD(Add)
            )
        )

        void SetX(int x) { X = x; }

        static Point Add(const Point& p0, const Point& p1)
        {
            return { p0.X + p1.X, p0.Y + p1.Y };
        }

        int X = 0;
        int Y = 0;
    };

    TEST_CASE("doc_introduction")
    {
        std::cout << "doc_introduction" << std::endl;

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

class Base
{
    ETI_BASE_SLIM(Base)
};

class Foo : public Base
{
    ETI_CLASS_SLIM(Foo, Base)
};

class Doo : public Base
{
    ETI_CLASS_SLIM(Doo, Base)
};

namespace doc_isa
{
    TEST_CASE("doc_isa")
    {
        Base base;
        Foo foo;
        Doo doo;
        std::cout << "base isa Base ? " << IsA<Base>(base) << std::endl;
        std::cout << "base type name is: " << TypeOf<Base>().Name << std::endl;
        std::cout << "foo isa Base ? " << IsA<Base>(foo) << std::endl;
        std::cout << "foo type name is: " << TypeOf<Foo>().Name << std::endl;
        std::cout << "doo isa Base ? " << IsA<Base>(doo) << std::endl;
        std::cout << "doo type name is: " << TypeOf<Doo>().Name << std::endl;
        std::cout << "base isa Foo ? " << IsA<Foo>(base) << std::endl;
        std::cout << "foo isa Foo ? " << IsA<Foo>(foo) << std::endl;
        std::cout << "doo isa Foo ? " << IsA<Foo>(doo) << std::endl;
        std::cout << "base isa Doo ? " << IsA<Doo>(base) << std::endl;
        std::cout << "foo isa Doo ? " << IsA<Doo>(foo) << std::endl;
        std::cout << "doo isa Doo ? " << IsA<Doo>(doo) << std::endl;
    }
}

namespace doc_cast
{
    class Base
    {
        ETI_BASE_SLIM(Base)
    };

    class Foo : public Base
    {
        ETI_CLASS_SLIM(Foo, Base)
    };

    class Doo : public Base
    {
        ETI_CLASS_SLIM(Doo, Base)
    };
    
    TEST_CASE("doc_isa")
    {
        Base base;
        Foo foo;
        Doo doo;

        {
            Foo* basePtr = Cast<Foo>(&base);
            std::cout << (basePtr != nullptr ? "valid" : "invalid") << std::endl;
            Foo* fooPtr = Cast<Foo>(&foo);
            std::cout << (fooPtr != nullptr ? "valid" : "invalid") << std::endl;
            // Foo* dooPtr = Cast<Foo>(&doo);  not compile
            Foo* dooPtr = Cast<Foo>((Base*)& doo);
            std::cout << (dooPtr != nullptr ? "valid" : "invalid") << std::endl;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
namespace doc_properties
{
    using namespace eti;

    class Person
    {
        ETI_BASE(
            Person,
            ETI_PROPERTIES
            (
                // optional Accessibility attribute, can also be user defined... see Attributes
                ETI_PROPERTY(Age, Accessibility(Access::Private))
            ),
            ETI_METHODS())
    public:
        virtual ~Person(){}
        int GetAge() const { return Age; }
    private:
        int Age = 0;
    };

    TEST_CASE("doc_properties")
    {
        Person person;
        const Property* ageProperty = TypeOf<Person>().GetProperty("Age");

        int age;
        ageProperty->Get(person, age);
        cout << "Initial Age is " << age << endl;

        ageProperty->Set(person, 21);
        ageProperty->Get(person, age);
        cout << "Adult Age is " << age << endl;

        cout << "Person::Age member is "
        << GetAccessName(ageProperty->GetAttribute<Accessibility>()->Access)
        << " of type : " << ageProperty->Variable.Declaration.Type.Name
        << endl;
    }
    // output
    //  Init Age is 0
    //  Adult Age is 21
    //  Person::Age member is private of type : i32
}

namespace doc_methods
{
    using namespace eti;

    struct Point
    {
        ETI_STRUCT(
            Point,
            ETI_PROPERTIES(
                ETI_PROPERTY(X),
                ETI_PROPERTY(Y)),
            ETI_METHODS(
                ETI_METHOD(SetX),
                ETI_METHOD(Add)))

        void SetX(int x)
        {
            X = x;
        }

        static Point Add(const Point& p0, const Point& p1)
        {
            return { p0.X + p1.X, p0.Y + p1.Y };
        }

        friend std::ostream& operator<<(std::ostream& os, const Point& obj)
        {
            os << "{x = " << obj.X << ", y = " << obj.Y << "}";
            return os;
        }

        int X = 0;
        int Y = 0;
    };

    TEST_CASE("doc_methods")
    {
        std::cout << "doc_methods" << std::endl;
        const Type& type = TypeOf<Point>();

        {
            Point p;
            const Method* setX = type.GetMethod("SetX");
            setX->CallMethod(p, NoReturn, 1);
            std::cout << p << endl;
        }

        {
            Point p1 = {1, 1};
            Point p2 = {2, 2};
            const Method* add = type.GetMethod("Add");

            Point result;

            add->CallStaticMethod(&result, p1, p2);
            std::cout << p1 << " + " << p2 << " = " << result << endl;
        }
    }
}