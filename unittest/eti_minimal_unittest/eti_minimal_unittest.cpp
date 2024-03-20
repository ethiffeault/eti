#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <unittest/doctest.h>
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

ETI_NAMED_POD(::minimal_test_01::Point, Point)

namespace minimal_test_01
{
    using namespace eti;

    TEST_CASE("minimal_test_01")
    {
        const Type& type = TypeOf<Point>();
        REQUIRE(type.Name == "Point");
        REQUIRE(type.Size == sizeof(Point));
    }
}
