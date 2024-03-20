#include "foo.h"

#include "../doctest.h"

namespace forward_test
{
    TEST_CASE("forward_test")
    {
        Foo foo;
        const Type& dooType = TypeOfForward<Doo>();
        REQUIRE(dooType.Kind == Kind::Forward);

        const Type& fooDooType = foo.DooType;
        REQUIRE(fooDooType.Kind == Kind::Forward);
    }
}
