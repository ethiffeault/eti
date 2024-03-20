#pragma once

#include <eti/eti.h>

#if !ETI_MINIMAL

namespace forward_test
{
    using namespace eti;

    struct Doo;

    struct Foo
    {
   
        ETI_BASE(Foo, ETI_PROPERTIES
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