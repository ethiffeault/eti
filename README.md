# eti
Extended Type Information for c++ 

rtti implementation that doesn't require c++ enable rtti. This lib is one header only without any external dependencies.

[Introduction](#Introduction)

[Type](#Type)

[IsA](#IsA)

[Cast](#Cast)

[POD](#POD)

[Struct](#Struct)

[Class](#Class)

[Properties](#Properties)

[Methods](#Methods)

[Attributes](#Attributes)

[External](#External)

[Templates](#Templates)

[Repository](#Repository)

[Configuration](#Configuration)

[SlimMode](#SlimMode)

[UnitTests](#UnitTests)

[Todo](#Todo)

[Others](#Others)

# Introduction

Using eti is straightforward, simple usage:
```
using namespace eti;

class Object
{
    ETI_BASE(Object)
};

class Foo : public Object
{
    ETI_CLASS(Foo, Object)
};

class Doo : public Object
{
    ETI_CLASS(Doo, Object)
};

void main()
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
```
output:
```
base isa Base ? 1
base type name is: Base
foo isa Base ? 1
foo type name is: Foo
doo isa Base ? 1
doo type name is: Doo
base isa Foo ? 0
foo isa Foo ? 1
doo isa Foo ? 0
base isa Doo ? 0
foo isa Doo ? 0
doo isa Doo ? 1
```

more complex:
```
using namespace eti;

struct Point
{
    ETI_STRUCT_EXT(
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

void main()
{
    const Type& type = TypeOf<Point>();
    // set value using property
    {
        const Property* propertyX = type.GetProperty("X");
        Point p{ 1, 1 };
        propertyX->Set(p, 2);

        // output: p.x = 2
        std::cout << "p.x = " << p.X << std::endl;       
    }
    // call SetX
    {
        const Method* set = type.GetMethod("SetX");
        Point p{ 1, 1 };
        int value = 101;
        set->CallMethod(p, (void*)nullptr, &value);

        // output: p.x = 101
        std::cout << "p.x = " << p.X << std::endl;       
    }
    // call static method Add
    {
        const Method* add = type.GetMethod("Add");
        Point p1{ 1, 1 };
        Point p2{ 2, 2 };
        Point result;
        add->CallStaticMethod(&result, &p1, &p2);

        // output: p1 + p2 = {3, 3}
        std::cout << "p1 + p2 = {" << result.X << ", " << result.Y << "}" << std::endl; 
    }
}
```
# Type
Core type of eti, Type define all aspect of a given type T

To get Type from any given type use: 
```
const Type& fooType = eti::TypeOf<Foo>();
const Type& intType = eti::TypeOf<int>();
```

```
    Type
    {
        Name;               // name
        Id;                 // id ( hash of it's name)
        Kind;               // kind (Void, Class, Struct, Pod, Template, Unknown or Forward)
        Size;               // size(T)
        Align;              // alignof(T)
        Parent;             // parent if any
        Construct;          // Constructor
        CopyConstruct;      // Copy Constructor
        MoveConstruct;      // Move Constructor
        Destruct;           // Destructor
        Properties;         // Properties
        Methods;            // Methods
        Templates;          // Templates types
        Attributes;         // Attributes
    }
```
# IsA
IsA to know if a type is a base type of another type

usage:
```
    IsA<Foo>(instance);
    IsA<Foo, Doo>();
```

```
    class Base
    {
        ETI_BASE(Base)
    };

    class Foo : public Base
    {
        ETI_CLASS(Foo, Base)
    };

    class Doo : public Base
    {
        ETI_CLASS(Doo, Base)
    };

    TEST_CASE("doc_isa")
    {
        Base base;
        Foo foo;
        Doo doo;
        std::cout << "base isa Base ? " << IsA<Base>(base) << std::endl;
        std::cout << "foo isa Base ? " << IsA<Base>(foo) << std::endl;
        std::cout << "doo isa Base ? " << IsA<Base>(doo) << std::endl;
        std::cout << "base isa Foo ? " << IsA<Foo>(base) << std::endl;
        std::cout << "foo isa Foo ? " << IsA<Foo>(foo) << std::endl;
        std::cout << "doo isa Foo ? " << IsA<Foo>(doo) << std::endl;
        std::cout << "base isa Doo ? " << IsA<Doo>(base) << std::endl;
        std::cout << "foo isa Doo ? " << IsA<Doo>(foo) << std::endl;
        std::cout << "doo isa Doo ? " << IsA<Doo>(doo) << std::endl;
    }
```
output:
```
base isa Base ? 1
foo isa Base ? 1
doo isa Base ? 1
base isa Foo ? 0
foo isa Foo ? 1
doo isa Foo ? 0
base isa Doo ? 0
foo isa Doo ? 0
doo isa Doo ? 1
```

# Cast
dynamic cast of T, when not match, return nullptr or not compile on incompatible type.
usage:
```
    Foo* basePtr = Cast<Foo>(&base);
```

```
    class Base
    {
        ETI_BASE(Base)
    };

    class Foo : public Base
    {
        ETI_CLASS(Foo, Base)
    };

    class Doo : public Base
    {
        ETI_CLASS(Doo, Base)
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
```
output
```
invalid
valid
invalid
```
# POD
define your own pod type using ETI_POD:
```
    ETI_POD(bool)
```
or user named pod using ETI_POD_EXT:
```
    ETI_POD_EXT(std::int8_t, i8);
```
by default ETI_TRIVIAL_POD is defined to 1, unless you see it to 0, will defined basic pods:
```
    ETI_POD(bool);

    ETI_POD_EXT(std::int8_t, i8);
    ETI_POD_EXT(std::int16_t, i16);
    ETI_POD_EXT(std::int32_t, i32);
    ETI_POD_EXT(std::int64_t, i64);

    ETI_POD_EXT(std::uint8_t, s8);
    ETI_POD_EXT(std::uint16_t, s16);
    ETI_POD_EXT(std::uint32_t, s32);
    ETI_POD_EXT(std::uint64_t, s64);

    ETI_POD_EXT(std::float_t, f32);
    ETI_POD_EXT(std::double_t, f64);
```

# Struct
use ETI_STRUCT or ETI_STRUCT_EXT to define struct, struct are base type, no virtual table and no inheritance:
```
    struct Point
    {
        ETI_STRUCT_EXT(
            Point,
            ETI_PROPERTIES(
                ETI_PROPERTY(X),
                ETI_PROPERTY(Y)),
            ETI_METHODS(
                ETI_METHOD(SetX),
                ETI_METHOD(Add)),
            [, attibutes...])

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
```
# Class
Use ETI_BASE or ETI_BASE_EXT to define class, class are dynamic object with virtual table. eti provide an optional common base class eti::Object
```
    class Object
    {
        ETI_BASE(Object)
    public:
        virtual ~Object(){}
    };

    class Foo
    {
        ETI_CLASS(Foo, Object)
    };    

    class Doo
    {
        ETI_CLASS_EXT(Doo, Object, PROPERTIES(), METHODS() [, attributes...])
    };    

```
you can define you own base class as needed.

# Properties
Property wrap member variables of class/struct
```
    Property
    {
        Name;       // name
        Type;       // Type
        Offset;     // offset from parent
        Parent;     // parent Type
        PropertyId; // id of this property (hash of it's name)
        Attributes; // all attributes
    }
```

```
    class Person
    {
        ETI_BASE_EXT(
            Person,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(Age [, attributes...])
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

        cout << "Person::Age member is of type : " << ageProperty->Variable.Declaration.Type.Name
        << endl;
    }
    // output
    //  Init Age is 0
    //  Adult Age is 21
    //  Person::Age member is of type : i32
```
for advance usage, Property provide unsafe method for direct access (offset from obj):

```
    inline void* Property::UnSafeGetPtr(void* obj) const;
    
    template <typename OBJECT>
    void* Property::UnSafeGetPtr(OBJECT& obj) const;
```

# Methods

Method wrap static and non-static member methods on struct/class.
```
Method
{
        Name;       // name
        MethodId;   // id
        IsStatic;   // static
        IsConst;    // const
        Function;   // std::function<void(void*, void*, std::span<void*>)>
        Return;     // return type, const Variable*
        Arguments;  // arguments, std::span<const Variable>
        Parent;     // parent, const Type* Parent
        Attributes; // all attributes
}
```
```
    class Person
    {
        ETI_BASE_EXT(
            Person,
            ETI_PROPERTIES(),
            ETI_METHODS
            ( 
                ETI_METHOD(GetAge[, attributes...])
            ))
    public:
        virtual ~Person(){}
        int GetAge() const { return Age; }
    private:
        int Age = 0;
    };
```
Call look like:
* void CallMethod(PARENT& owner, RETURN* ret, ARGS... args) const;
or
* void CallStaticMethod(RETURN* ret, ARGS... args) const;

when method return void, use eti::NoReturn like :
* setX->CallMethod(p, NoReturn, 1);

UnSafeCall is the bare bone way of calling method:
* void UnSafeCall(void* obj, void* ret, std::span<void*> args) const;

```
    using namespace eti;

    struct Point
    {
        ETI_STRUCT_EXT(
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
```
output:
```
{x = 1, y = 0}
{x = 1, y = 1} + {x = 2, y = 2} = {x = 3, y = 3}
```

# Attributes
Attribute are supported on Struct, Class, Properties and Methods

* Struct
* Class
* Properties
* Methods

May be user defined like this:
```
    class Documentation : public eti::Attribute
    {
        ETI_CLASS(Documentation, Attribute)

    public:

        Documentation(string_view documentation)
        {
            Documentation = documentation;
        }

        string_view Documentation;
    };

    // use it on any property, method and type:
    ETI_PROPERTY(Age, Documentation("Age doc..."))
    ETI_METHOD(GetAge, Documentation("Age getter..."))
    ETI_CLASS_EXT(Foo, Base, ETI_PROPERTIES(), ETI_METHODS(), Documentation("Foo class..."))

    // query it:
    const Documentation* doc = TypeOf<Foo>().GetProperty<Documentation>();
    // note: return nullptr if not exist
    std::cout << doc->Documentation;
```    

# External
Support external type declaration from third party library
```
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

ETI_STRUCT_EXTERNAL(::third_party::Point, ETI_PROPERTIES(ETI_PROPERTY(X), ETI_PROPERTY(Y)), ETI_METHODS(ETI_METHOD(GetX), ETI_METHOD(SetX)))
ETI_BASE_EXTERNAL(::third_party::Base, ETI_PROPERTIES(), ETI_METHODS());
ETI_CLASS_EXTERNAL(::third_party::Foo, ::third_party::Base, ETI_PROPERTIES(), ETI_METHODS(ETI_METHOD(GetName)));

namespace test_external
{
    using namespace eti;
    using namespace third_party;

    TEST_CASE("test_external")
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
```

# Templates

template are supported, define it using :
```
    ETI_TEMPLATE_1(std::vector)
    ETI_TEMPLATE_2(std::map)
```
note: std::vector and std::map are automatically define if your config use ETI_COMMON_TYPE (enabled per default)

then you have access like this:
```
    using namespace eti;
    struct Foo
    {
        ETI_STRUCT_EXT(Foo, ETI_PROPERTIES( ETI_PROPERTY(Values)), ETI_METHODS())
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
```
note that if you do not define your template, you still have access to :
```
    if ( propertyType == TypeOf<std::vector<int>>() )
    {
        // ....
    }
```

# Repository
**WIP**

To enable Repository use config : 
* #define ETI_REPOSITORY 1

Repository contain type mapping from TypeId to Type and from Name to Type. Practical for pattern like serialization.

# Configuration

To change default behavior this lib provide 2 ways, one is to declare #define before include, one it's to provide your own config file.

* #define before include:

  define what you need before #include <eti/eti.h>

* config file:

  #define ETI_CONFIG_HEADER 1 in <eti/eti.h> and create "eti_config.h" with your configurations.

list of available config #define can be found  at beginning of <eti/eti.h> (see comment for documentation)

# SlimMode
Support slim mode when only basic type information are needed (no properties, no functions, no attributes, ...)

Configuration ( default is 0)
```
#define ETI_SLIM_MODE 1
```

Available in slim mode: 
* TypeOf<T>()
* IsA<T>()
* Cast<T>()
* ETI_BASE
* ETI_CLASS
* ETI_STRUCT
* ETI_POD
* ETI_POD_EXT

# UnitTests

see ./unittest/eti_unittests.cpp

Compile:
* eti_unittest.sln
* clang++ -I . -std=c++20 -o unittest.exe ./unittest/eti_unittests.cpp

# Todo

* Repository
* Enum
* Interface
* Static member variable

# Others

eti use awesome great unit tests framework: [doctest](https://github.com/doctest/doctest)