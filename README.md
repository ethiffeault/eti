# eti
Extended Type Information for c++ 

rtti implementation that doesn't require c++ enable rtti. This lib is one header only without any external dependencies.

Support:
* IsA
* Cast
* Type
* POD
* Struct
* Class
* Properties
* Methods
* Attributes
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
Core type of eti, Type define all aspect of a given type T
```
     Type
    {
        Name;
        Id;
        Kind;
        Size;
        Align;
        Parent;
        Construct;
        CopyConstruct;
        MoveConstruct;
        Destruct;
        Properties;
        Methods;
        Templates;
    }
```
## IsA
## Cast
## POD
## Struct
## Class
## Properties
Property
* Access to member variables (public, protected and private)
* Pointer or value.
* Support Attributes.
* Set/Get using reflexion via eti::Property.

```
    Property
    {
        Name;
        Type;
        Offset;
        Parent;
        PropertyId;
        Attributes;
    }
```
ex:

```
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
```
for advance usage, Property provide this to get member variable pointer (offset from obj):

```
    inline void* Property::UnSafeGetPtr(void* obj) const;
    
    template <typename OBJECT>
    void* Property::UnSafeGetPtr(OBJECT& obj) const;
```


## Methods
wip
## Attributes

Attribute are supported on Struct, Class, Properties ans Methods

* Struct

  wip

* Class

  wip

* Properties

  base class: eti::PropertyAttribute

* Methods

  wip


All kind of Attribute may be user defined like this:
```
    class Accessibility : public eti::PropertyAttribute
    {
        ETI_CLASS_SLIM(Accessibility, PropertyAttribute)

    public:

        Accessibility(Access access)
        {
            Access = access;
        }

        Access Access = Access::Unknown;
    };

    // use it on any property, ex:
    ETI_PROPERTY(Age, Accessibility(Access::Private), Documentation("Age..."), ...)

    // query it:
    const Property* ageProperty = type.GetProperty("Age");
    Access access = ageProperty->GetAttribute<Accessibility>()->Access;
```    

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

  #define ETI_CONFIG_HEADER 1 in <eti/eti.h> and create "eti_config.h" with all config defines.

list of available config #define can be found  at beginning of <eti/eti.h> (see comment for documentation)

## Todo

* Repository
* External struct/class decl
* Enum
* Interface

## External

eti use great doctest framework: https://github.com/doctest/doctest 