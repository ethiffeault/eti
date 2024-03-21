#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <unittest/doctest.h>

#define ETI_SLIM_MODE 1
#include <eti/eti.h>

////////////////////////////////////////////////////////////////////////////////
namespace minimal_test_01
{
    struct Point
    {
        int X = 0;
        int Y = 0;
    };
}

ETI_POD_NAMED(::minimal_test_01::Point, Point)

namespace minimal_test_01
{
    using namespace eti;

    TEST_CASE("minimal_test_01")
    {
        const Type& type = TypeOf<Point>();
        REQUIRE(type.Name == "Point");
        REQUIRE(type.Size == sizeof(Point));
    }

    class Object
    {
        ETI_BASE_SLIM(Object)
    public:
        virtual ~Object(){}
        int x = 0;
    };

    class Foo : public Object
    {
        ETI_CLASS_SLIM(Foo, Object)
    public:
        ~Foo() override{}
        int i = 0;
        int j = 0;
    };

    class Doo : public Object
    {
        ETI_CLASS_SLIM(Doo, Object)
    public:
        ~Doo() override{}
        int i = 0;
    };

    TEST_CASE("minimal_test_02")
    {
        Foo foo;
        Doo doo;

        REQUIRE(IsA<Object>(foo));
        REQUIRE(IsA<Object>(doo));
        REQUIRE(IsA<Foo>(foo));
        REQUIRE(!IsA<Foo>(doo));
        REQUIRE(IsA<Doo>(doo));
        REQUIRE(!IsA<Doo>(foo));

        REQUIRE(TypeOf<Object>().Size == sizeof(Object));
        REQUIRE(TypeOf<Foo>().Size == sizeof(Foo));
        REQUIRE(TypeOf<Doo>().Size == sizeof(Doo));

        REQUIRE(TypeOf<Object>().Align == alignof(Object));
        REQUIRE(TypeOf<Foo>().Align == alignof(Foo));
        REQUIRE(TypeOf<Doo>().Align == alignof(Doo));

        REQUIRE(TypeOf<Object>().Name == "class minimal_test_01::Object");
        REQUIRE(TypeOf<Foo>().Name == "class minimal_test_01::Foo");
        REQUIRE(TypeOf<Doo>().Name == "class minimal_test_01::Doo");

    }
}
