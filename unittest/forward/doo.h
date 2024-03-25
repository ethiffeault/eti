#pragma once

#include <eti/eti.h>


#if !ETI_SLIM_MODE
namespace forward_test
{
    struct Doo
    {
        ETI_BASE(Doo)
    public:
        virtual ~Doo(){}
    };
}
#endif