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

Using eti is straightforward, simple usage look like:
```
#include <eti/eti.h>

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
    ETI_CLASS_SLIM(Foo, Object)
};

void main()
{
    Foo foo;
    Doo doo;
    std::count << "For is Object ?" << (IsA<Object>(foo) ? "true" : "false");   // true
    std::count << "Doo is Foo ?" << (IsA<Foo>(doo) ? "true" : "false");         // false
}
```

more complex:

```
struct Point
{
    ETI_STRUCT(
        Point, 
        ETI_PROPERTIES( ETI_PROPERTY(X), ETI_PROPERTY(Y) ), 
        ETI_METHODS( ETI_METHOD(Mul) ))

    static Point Mul(const Point& p0, const Point& p1)
    {
        return { p0.X * p1.X, p0.Y * p1.Y };
    }

    int X = 0;
    int Y = 0;
};

void main()
{
    // todo ex
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