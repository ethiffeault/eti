# eti
Efficient Type Information in c++ 

Efficient rtti implementation that doesn't require c++ enable rtti. This lib is one header only without any external dependencies.

Support:
* IsA
* Cast
* New, NewCopy, Move and Delete
* POD
* Struct
* Class
* Properties
* Methods (member and static)
* Attributes (on types, properties and methods)
* Repository

## Introduction

Using eti is straightforward, simple usage:
```
using namespace eti;

class Object
{
    ETI_BASE_SLIM(Object)
};

class Foo : public Object
{
    ETI_CLASS_SLIM(Foo, Object)
};

class Doo : public Object
{
    ETI_CLASS_SLIM(Doo, Object)
};

void main()
{
    Foo foo;
    Doo doo;

    // output: Foo is a Object ? true
    std::count << "Foo is a Object ? " << ( IsA<Object>(foo) ? "true" : "false" );

    // output: Doo is a Foo ? false
    std::count << "Doo is a Foo ? " << ( IsA<Foo>(doo) ? "true" : "false" );
}
```
more complex:
```
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

## Type
## IsA
## Cast
## New
## NewCopy
## Move
## POD
## Struct
## Class
## Properties
Property is a way to access any member variables and may contain custom attributes.
Member can be private and protected.
Support attributes.
```
    class Person
    {
        ETI_BASE(
            Person,
            ETI_PROPERTIES
            (
                ETI_PROPERTY(Age, Accessibility(Access::Private)) // optional Accessibility attribute, can be any user defined attributes
            ),
            ETI_METHODS())
    public:
        virtual ~Person(){}
        int GetAge() const { return Age; }
    private:
        int Age = 0;
    };

    void main()
    {
        Person person;
        const Property* ageProperty = TypeOf<Person>().GetProperty("Age");

        int age;
        ageProperty->Get(person, age);
        cout << "Init age is " << age << endl;

        ageProperty->Set(person, 21);
        ageProperty->Get(person, age);
        cout << "Adult age is " << age << endl;

        cout << "Person::Age member is " << GetAccessName(ageProperty->GetAttribute<Accessibility>()->Access) << endl;
    }
    // output
    //  Init age is 0
    //  Adult age is 21
    //  Person::Age member is private
```

## Methods (member and static)
## Attributes (on types, properties and methods)
## Repository

To enable Repository use config : 
* #define ETI_REPOSITORY 1

Repository contain type mapping from TypeId to Type and from Name to Type. Practical for stuff like serialization...

todo: more doc...

## Slim

Support slim mode when only basic type information are needed (no properties, no functions, no attributes, ...)

Available in slime mode: 
* TypeOf<T>()
* IsA<T>()
* Cast<T>()
* ETI_BASE_SLIM
* ETI_CLASS_SLIM
* ETI_STRUCT_SLIM
* ETI_POD
* ETI_POD_NAMED

## Configuration

To change default behavior this lib provide 2 ways, one is to declare #define before include, one it's to provide your own config file.

* #define before include:
define what you need before #include <eti/eti.h>

* config file:
#define ETI_CONFIG_HEADER 1 in <eti/eti.h>
create a eti_config.h using config defines

list of available config #define can be found  at beginning of <eti/eti.h> (see comment for documentation)

## Todo

* Repository
* External struct/class decl
* Enum
* Interface

## External

eti use doctest for unittests: https://github.com/doctest/doctest 