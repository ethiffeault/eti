#pragma once

#include <eti/eti.h>

namespace forward_test
{
    struct Doo
    {
        ETI_BASE_SLIM(Doo)
    public:
        virtual ~Doo(){}
    };
}
