#pragma once

#include <eti/eti.h>


#if !ETI_MINIMAL
namespace forward_test
{
    struct Doo
    {
        ETI_BASE_SLIM(Doo)
    public:
        virtual ~Doo(){}
    };
}
#endif