#pragma once

#include <eti/eti.h>

#if !ETI_SLIM_MODE

namespace forward_test
{
    using namespace eti;

    struct Doo;

    struct Foo
    {
   
        ETI_BASE_EXT(Foo, ETI_PROPERTIES
                 (
                     ETI_PROPERTY(DooPtr)
                 ),
                 ETI_METHODS
                 (
                 ))

    public:
        virtual ~Foo(){}

        const Type& DooType = ::eti::TypeOfForward<Doo>();

        Doo* DooPtr = nullptr;

    };

}

#endif