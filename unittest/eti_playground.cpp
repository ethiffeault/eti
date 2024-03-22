#include "doctest.h"
#include <eti/eti.h>

#include <iostream>

using namespace eti;

namespace playground
{
  /*  template<typename T>
    void ValidateArgument(const Variable& variable)
    {
        std::cout << "ValidateVariable, T = " << TypeOf<T>().Name << ", Variable T = " << variable.Declaration.Type.Name << std::endl;
    }

    
    template<typename... Ts, std::size_t... Is>
    void ValidateArgumentsForEach(std::span<Variable> variables, std::index_sequence<Is...>)
    {
        (ValidateArgument<Ts>(variables[Is]), ...);
    }

    template<typename... Args>
    void ValidateArguments(std::span<Variable> variables)
    {
        ETI_ASSERT(variables.size() == sizeof...(Args), "argument count missmatch, method need " << variables.size() << ", " << sizeof...(Args) << " provided");
        ValidateArgumentsForEach<Args...>(variables, std::index_sequence_for<Args...>{});
    }

    TEST_CASE("playground")
    {
        
        Variable v1 = internal::MakeVariable("v1", internal::MakeDeclaration<int>());
        Variable v2 = internal::MakeVariable("v2", internal::MakeDeclaration<float>());
        Variable v3 = internal::MakeVariable("v3", internal::MakeDeclaration<double>());
        std::vector<Variable> variables = {v1, v2, v3};
        ValidateArguments<int, float, double*>(variables);

    }*/
}
