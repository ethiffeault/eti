//  MIT License
//  
//  Copyright (c) 2024 Eric Thiffeault
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

// eti dev version from v0.0.1
// 2024/03/19

#pragma once

#include <functional>
#include <string_view>
#include <span>

// Configuration overriding
//
//  1. set ETI_CONFIG_HEADER to 1 to use external header
//  2. or define override before include eti.h
//
#define ETI_CONFIG_HEADER 0

#if ETI_CONFIG_HEADER

    #include "eti_config.h"

#else

    #ifndef ETI_ASSERT
        // Assert and Error
        #include <cassert>
        #include <memory>
        #define ETI_ASSERT(cond, msg) assert(cond)
        #define ETI_ERROR(msg) assert(true)
        #define ETI_INTERNAL_ASSERT(cond, msg) assert(cond)
        #define ETI_INTERNAL_ERROR(msg) assert(true)
    #endif

    // TypeId/Hashing
    #ifndef ETI_TYPE_ID_TYPE
        #define ETI_TYPE_ID_TYPE std::uint64_t
    #endif

    #ifndef ETI_HASH_FUNCTION
        #define ETI_HASH_FUNCTION ::eti::HashFNV1WithPrime
        //#define ETI_HASH_FUNCTION ::eti::HashFNV1
        #define ETI_HASH_SEED 0xCBF29CE484222325ull
    #endif
        
    #ifndef ETI_TYPE_NAME_FUNCTION
        // Function Type Name
        #define ETI_TYPE_NAME_FUNCTION ::eti::GetTypeNameHashImpl
    #endif

    #ifndef ETI_TRIVIAL_POD
        // Define basic pod Type in global scope with convenient name :
        //  bool, u8, u16, u32, u64, s8, s16, s32, s64, f32 and f64
        #define ETI_TRIVIAL_POD 1
    #endif

    #ifndef ETI_REPOSITORY
        // Enable Repository
        //
        //  All type will self register so it's possible to make at runtime:
        //      const Type* type = Repository::GetType(fooId);
        //      const Type* type = Repository::GetType("Foo");
        //
        // For stuff like serialization...
        //
        // need a #define in one cpp file to work: ETI_REPOSITORY_IMPL()
        #define ETI_REPOSITORY 1

        // If ETI_REPOSITORY == 1 and in dynamic library, export symbol using this.
        #define ETI_REPOSITORY_API
    #endif

#endif // ETI_CONFIG_HEADER

namespace eti
{
    // Forward types

    struct Type;

    template<typename T>
    const Type& TypeOf();

    template<typename T>
    const Type& TypeOfForward();

    using TypeId = ETI_TYPE_ID_TYPE;

    // Raw Type
    //  from any type whit any modifier (const, const*, *, ...) return raw type T
    //  use the prevent code blow for type, const and ptr info are stored inside Declaration

    template<typename T>
    struct RawTypeImpl
    {
        using Type = std::decay_t<std::remove_pointer_t<T>>;
    };

    template<typename T>
    using RawType = typename RawTypeImpl<T>::Type;

    // IsCompleteType / IsForwardType

    template<typename T, typename = std::void_t<>>
    struct IsCompleteTypeImpl : std::false_type {};

    template<typename T>
    struct IsCompleteTypeImpl<T, std::void_t<decltype(sizeof(T))>> : std::true_type {};

    template<typename T>
    constexpr bool IsCompleteType = IsCompleteTypeImpl<T>::value;

    template<typename T, typename = std::void_t<>>
    struct IsForwardTypeImpl : std::true_type {};

    template<typename T>
    struct IsForwardTypeImpl<T, std::void_t<decltype(sizeof(T))>> : std::false_type {};

    template<typename T>
    constexpr bool IsForwardType = IsForwardTypeImpl<T>::value;

    // GetTypeName
    //  return const name for any given types

# if defined __clang__ || defined __GNUC__
    template<typename T>
    constexpr auto GetTypeNameHashImpl()
    {
        std::string_view pretty_function{ __PRETTY_FUNCTION__ };
        auto first = pretty_function.find_first_not_of(' ', pretty_function.find_first_of('=') + 1);
        auto value = pretty_function.substr(first, pretty_function.find_last_of(']') - first);
        return value;
    }
#elif defined _MSC_VER
    template<typename T>
    constexpr auto GetTypeNameHashImpl()
    {
        std::string_view pretty_function{ __FUNCSIG__ };
        auto first = pretty_function.find_first_not_of(' ', pretty_function.find_first_of('<') + 1);
        auto value = pretty_function.substr(first, pretty_function.find_last_of('>') - first);
        return value;
    }
#else
#pragma error implement type name
#endif

    template<typename T>
    constexpr auto GetTypeNameImpl()
    {
        return ETI_TYPE_NAME_FUNCTION<T>();
    }

    template<typename T>
    constexpr auto GetTypeName()
    {
        return GetTypeNameImpl<RawType<T>>();
    }

    // Hash constexpr string_view to constexpr TypeId

    constexpr TypeId HashFNV1WithPrime(const std::string_view str, TypeId hash = ETI_HASH_SEED)
    {
        std::uint64_t prime = 0x100000001B3ull;

        for (char c : str)
        {
            hash ^= static_cast<TypeId>(c);
            hash *= prime;
            hash = (hash << 5) | (hash >> (64 - 5));
            hash ^= 0x27d4eb2d;
            hash *= 0x00000100000001B3;
        }
        return hash;
    }

    // Simple hash constexpr string_view to constexpr TypeId (faster)

    constexpr TypeId HashFNV1(std::string_view str, TypeId hash = ETI_HASH_SEED)
    {
        for (char c : str)
        {
            hash ^= static_cast<TypeId>(c);
            hash *= 0x100000001B3ull;
        }
        return hash;
    }

    constexpr TypeId GetStringHash(const std::string_view str)
    {
        return ETI_HASH_FUNCTION(str);
    }

    template<typename T>
    constexpr TypeId GetTypeId()
    {
        ETI_ERROR("GetTypeId not defined for this type");
        return GetStringHash(GetTypeName<T>());
    }

    // void* vs type helpers

    // cast void* to T and remove ref if needed
    template<typename T>
    auto VoidPrtToTypeArg(void* ptr) -> decltype(auto)
    {
        if constexpr (std::is_reference_v<T>)
        {
            using PtrType = std::remove_reference_t<T>*;
            return *static_cast<PtrType>(ptr);
        }
        else
        {
            return *static_cast<T*>(ptr);
        }
    }

    // convert void* args to tuple
    template<typename... ARGS, size_t... Is>
    auto VoidArgsToTuple(std::span<void*> args, std::index_sequence<Is...>)
    {
        return std::make_tuple(&VoidPrtToTypeArg<ARGS>(args[Is])...);
    }

    // static function call

    // static function call with return value
    template<typename RETURN, typename... ARGS>
    struct CallStaticFunctionImpl
    {
        static void Call(RETURN(*func)(ARGS...), void* obj, void* ret, std::span<void*> args)
        {
            ETI_ASSERT(sizeof...(ARGS) == args.size(), "invalid size of args");
            ETI_ASSERT(obj == nullptr, "call static function should have obj to nullptr");
            auto args_tuple = VoidArgsToTuple<ARGS...>(args, std::index_sequence_for<ARGS...>{});
            std::apply([&](auto... applyArgs) { *((RETURN*)ret) = func(*applyArgs...); }, args_tuple);
        }
    };

    // static function call no return
    template<typename... ARGS>
    struct CallStaticFunctionImpl<void, ARGS...>
    {
        static void Call(void(*func)(ARGS...), void* obj, void* ret, std::span<void*> args)
        {
            ETI_ASSERT(sizeof...(ARGS) == args.size(), "invalid size of args");
            ETI_ASSERT(obj == nullptr, "call static function should have obj to nullptr");
            ETI_ASSERT(ret == nullptr, "call function that return void should have ret arg to nullptr");
            auto args_tuple = VoidArgsToTuple<ARGS...>(args, std::index_sequence_for<ARGS...>{});
            std::apply([&](auto... applyArgs) { func(*applyArgs...); }, args_tuple);

        }
    };

    // static function call switch
    template<typename RETURN, typename... ARGS>
    void CallFunction(RETURN(*func)(ARGS...), void* obj, void* ret, std::span<void*> args)
    {
        ETI_ASSERT(sizeof...(ARGS) == args.size(), "invalid size of args");
        CallStaticFunctionImpl<RETURN, ARGS...>::Call(func, obj, ret, args);
    }

    // member function call

    // member function call with return value
    template<typename OBJECT, typename RETURN, typename... ARGS>
    struct CallMemberFunctionImpl
    {
        static void Call(RETURN(OBJECT::* func)(ARGS...), void* obj, void* ret, std::span<void*> args)
        {
            ETI_ASSERT(sizeof...(ARGS) == args.size(), "invalid size of args");
            ETI_ASSERT(obj != nullptr, "internal error");
            ETI_ASSERT(ret != nullptr, "internal error");
            auto args_tuple = VoidArgsToTuple<ARGS...>(args, std::index_sequence_for<ARGS...>{});
            std::apply([&](auto... applyArgs) { *((RETURN*)ret) = ((OBJECT*)obj->*func)(*applyArgs...); }, args_tuple);

        }
    };

    // member function call void return
    template<typename OBJECT, typename... ARGS>
    struct CallMemberFunctionImpl<OBJECT, void, ARGS...>
    {
        static void Call(void(OBJECT::* func)(ARGS...), void* obj, void* ret, std::span<void*> args)
        {
            ETI_ASSERT(sizeof...(ARGS) == args.size(), "invalid size of args");
            ETI_ASSERT(obj != nullptr, "internal error");
            ETI_ASSERT(ret == nullptr, "internal error");
            auto args_tuple = VoidArgsToTuple<ARGS...>(args, std::index_sequence_for<ARGS...>{});
            std::apply([&](auto... applyArgs) { ((OBJECT*)obj->*func)(*applyArgs...); }, args_tuple);
        }
    };

    // member function call switch
    template<typename OBJECT, typename RETURN, typename... ARGS>
    void CallFunction(RETURN(OBJECT::* func)(ARGS...), void* obj, void* ret, std::span<void*> args)
    {
        ETI_ASSERT(sizeof...(ARGS) == args.size(), "invalid size of args");
        CallMemberFunctionImpl<OBJECT, RETURN, ARGS...>::Call(func, obj, ret, args);
    }

    // Types
    //
    // all types have default construct/copy/move/destruct and use static Make(...) method pattern for construction (may be template and easier code reading)
    // note that user only have access to const type, not need to to have public accessor
    enum class Kind : std::uint8_t
    {
        Void,       // void
        Class,      // ETI_BASE or ETI_CLASS
        Struct,     // ETI_STRUCT
        Pod,        // ETI_POD
        Template,   // ETI_TEMPLATE_X
        Unknown,    // automatic declared type, when user not supply it's own declaration
        Forward     // forward type
    };

    // return compile time name for Kind
    static constexpr std::string_view GetKindName(Kind typeDesc)
    {
        switch (typeDesc)
        {
            case Kind::Void:
                return "void";
            case Kind::Class:
                return "class";
            case Kind::Struct:
                return "struct";
            case Kind::Pod:
                return "pod";
            case Kind::Template:
                return "template";
            case Kind::Unknown:
                return "unknown";
        }
    }

    // store information about type and it's modifier
    struct Declaration
    {
        const Type& Type;
        bool IsPtr = false;         // note: ref are considered as ptr in eti
        bool IsConst = false;

        template <typename T>
        static Declaration Make()
        {
            return
            {
                TypeOfForward<T>(),
                std::is_pointer_v<T>,
                std::is_const_v<T>,
            };
        }
    };

    // class/struct access
    enum class Access : std::uint8_t
    {
        Private,
        Protected,
        Public,
        Unknown
    };

    // variable, used for function args, property...
    struct Variable
    {
        std::string_view Name;
        Declaration Declaration;

        static Variable Make(std::string_view name, ::eti::Declaration declaration)
        {
            return
            {
                name,
                declaration,
            };
        }
    };

    // Helper to create and get variables

    template<typename T>
    const Variable* GetVariable(std::string_view name)
    {
        static Variable variable = Variable::Make(name, Declaration::Make<T>());
        return &variable;
    }

    template<typename... ARGS>
    std::span<Variable> GetVariables()
    {
        // note: name not supported yet (maybe in c++ 34?)
        static std::vector<Variable> variables = { Variable::Make("", Declaration::Make<ARGS>())... };
        return variables;
    }

    // GetFunctionVariables, return variables args of the function

    // static 
    template<typename RETURN, typename... ARGS>
    const Variable* GetFunctionReturn(RETURN(*func)(ARGS...))
    {
        return GetVariable<RETURN>("");
    }

    // member
    template<typename OBJECT, typename RETURN, typename... ARGS>
    const Variable* GetFunctionReturn(RETURN(OBJECT::* func)(ARGS...))
    {
        return GetVariable<RETURN>("");
    }

    // static 
    template<typename RETURN, typename... ARGS>
    std::span<Variable> GetFunctionVariables(RETURN(*func)(ARGS...))
    {
        return GetVariables<ARGS...>();
    }

    // member
    template<typename OBJECT, typename RETURN, typename... ARGS>
    std::span<Variable> GetFunctionVariables(RETURN(OBJECT::* func)(ARGS...))
    {
        return GetVariables<ARGS...>();
    }

    class Attribute;

    // member variable of class/struct
    struct Property
    {
        Variable Variable;
        size_t Offset;
        std::vector<std::shared_ptr<Attribute>> Attributes;

        template <typename T>
        static Property Make(std::string_view Name, size_t offset, std::vector<std::shared_ptr<Attribute>>&& attributes = {})
        {
            return
            {
                ::eti::Variable::Make(Name, ::eti::Declaration::Make<T>()),
                offset,
                std::move(attributes)
            };
        }

        template <typename T>
        const T* GetAttribute() const;
    };

    // method of class/struct
    struct Method
    {
        std::string_view Name;
        std::function<void(void*, void*, std::span<void*>)> Function;
        Access Access;
        const Variable* Return;
        std::span<const Variable> Arguments;

        static Method Make(std::string_view name, std::function<void(void*, void*, std::span<void*>)>&& function, const Variable* _return = nullptr, std::span<const Variable> arguments = {})
        {
            return
            {
                name,
                function,
                ::eti::Access::Unknown, // todo
                _return,
                arguments,
            };
        }
    };

    // Type, eti core type, represent runtime type info about <T>
    struct Type
    {
        std::string_view Name;
        TypeId Id = 0;
        Kind Kind = Kind::Unknown;
        size_t Size = 0;
        size_t Align = 0;
        const Type* Parent = nullptr;
        std::function<void(void* /* dst */)> Construct;
        std::function<void(void* /* src */, void* /* dst */)> CopyConstruct;
        std::function<void(void* /* src */, void* /* dst */)> Move;
        std::function<void(void* /* dst */)> Destruct;
        std::span<const Property> Properties;
        std::span<const Method> Methods;
        std::span<const Type*> Templates;

        bool operator==(const Type& other) const { return Id == other.Id; }
        bool operator!=(const Type& other) const { return !(*this == other); }

        template<typename T>
        static std::function<void(void* /* dst */)> GetConstruct()
        {
            if constexpr (std::is_default_constructible_v<T>)
                return [](void* dst) { new (dst) T(); };
            else
                return {};
        }

        template<typename T>
        static std::function<void(void* /* src */, void* /* dst */)> GetCopyConstruct()
        {
            if constexpr (std::is_copy_constructible_v<T>)
                return [](void* src, void* dst) { new (dst) T(*(T*)src); };
            else
                return {};
        }

        template<typename T>
        static std::function<void(void* /* src */, void* /* dst */)> GetMoveConstruct()
        {
            if constexpr (std::is_default_constructible_v<T>)
                return [](void* src, void* dst) { new (dst) T(std::move(*(T*)src)); };
            else
                return {};
        }

        template<typename T>
        static std::function<void(void* /* dst */)> GetDestruct()
        {
            return [](void* dst) { ((T*)dst)->~T(); };
        }

        template<typename T>
        static const Type Make(::eti::Kind kind, const Type* parent, std::span<const Property> properties = {}, std::span<const Method> methods = {}, std::span<const Type*> templates = {})
        {
            if constexpr (std::is_void<T>::value == false)
            {
                if  constexpr (IsCompleteType<T>)
                {
                    return
                    {
                        GetTypeName<T>(),
                        GetTypeId<T>(),
                        kind,
                        sizeof(T),
                        alignof(T),
                        parent,
                        GetConstruct<T>(),
                        GetCopyConstruct<T>(),
                        GetMoveConstruct<T>(),
                        GetDestruct<T>(),
                        properties,
                        methods,
                        templates
                    };
                }
                else
                {
                    return
                    {
                        GetTypeName<T>(),
                        GetTypeId<T>(),
                        Kind::Forward,
                        0,
                        0,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr,
                        {},
                        {},
                        {}
                    };
                }
            }
            else
            {
                return
                {
                    "void",
                    0,
                    Kind::Void,
                    0,
                    0,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    {},
                    {},
                    {}
                };
            }
        }

        const Property* GetProperty(std::string_view name) const
        {
            auto it = std::ranges::find_if(Properties, [name](const Property& it) { return it.Variable.Name == name; });
            return (it != Properties.end()) ? &(*it) : nullptr;
        }

        const Method* GetMethod(std::string_view name) const
        {
            auto it = std::ranges::find_if(Methods, [name](const Method& it) { return it.Name == name; });
            return (it != Methods.end()) ? &(*it) : nullptr;
        }
    };

    template<typename T>
    Type __InternalGetType(::eti::Kind kind, const Type* parent, std::span<const Property> properties = {}, std::span<const Method> methods = {}, std::span<const Type*> templates = {})
    {
        static Type type = Type::Make<T>(kind, parent, properties, methods, templates);

        // type may be Forward type, in this case update is on demand
        if (type.Kind == Kind::Forward && IsCompleteType<T>)
            type = Type::Make<T>(kind, parent, properties, methods, templates);

        return type;
    }


    //
    // TypesOf

    template<typename T>
    struct HasStaticGetTypeStatic
    {

    private:

        template<typename _T>
        static constexpr auto Check(_T*)
            -> typename std::is_same<decltype(_T::GetTypeStatic()), const Type&>::type
        {
            return {};
        }

        template<typename>
        static constexpr std::false_type Check(...)
        {
            return {};
        }

        typedef decltype(Check<T>(nullptr)) type;

    public:

        static constexpr bool value = type::value;
    };


    template<typename... Args>
    std::span<const Type*> TypesOf();

    template<typename T>
    struct TypeOfImpl
    {
        static const Type& GetTypeStatic()
        {
            // default create an Unknown type
            static Type type = Type::Make<T>(Kind::Unknown, nullptr, {}, {}, {});
            return type;
        }
    };

    // use static const Type& T::GetTypeStatic(){...} if available
    template<typename T>
    const Type& OwnerGetType()
    {
        if constexpr (HasStaticGetTypeStatic<T>::value)
            return T::GetTypeStatic();
        else
            return TypeOfImpl<RawType<T>>::template GetTypeStatic();
    }

    template<typename T>
    const Type& TypeOf()
    {
        static_assert(IsCompleteType<T>, "Type must be completely declared, missing include ?");
        return OwnerGetType<T>();
    }

    // TypeOfForward
    //  allow get type on forward decl type, may return Kind == Forward
    template<typename T>
    const Type& TypeOfForward()
    {
        //static_assert(IsCompleteType<T>, "Type must be complete declared, missing include ?");
        return OwnerGetType<T>();
    }

    template<typename... ARGS>
    std::span<const Type*> TypesOf()
    {
        static const Type* types[] = { &TypeOfForward<ARGS>()... };
        return std::span<const Type*>(types, sizeof...(ARGS));
    }

    // IsA

    constexpr bool IsA(const Type& type, const Type& base)
    {
        const Type* cur = &type;
        while (cur != nullptr)
        {
            if (*cur == base)
                return true;
            cur = cur->Parent;
        }
        return false;
    }

    template<typename BASE, typename T>
    bool IsA(const T& instance)
    {
        static_assert(IsCompleteType<BASE>, "Base type must be completely declared, missing include ?");
        static_assert(IsCompleteType<T>, "Type must be completely declared, missing include ?");
        return IsA(instance.GetType(), TypeOf<BASE>());
    }

    template<typename T, typename BASE>
    constexpr bool IsATyped()
    {
        static_assert(IsCompleteType<BASE>, "Base type must be completely declared, missing include ?");
        static_assert(IsCompleteType<T>, "Type must be completely declared, missing include ?");
        return IsA(TypeOf<T>(), TypeOf<BASE>());
    }

    // Cast

    template<typename BASE, typename T>
    BASE* Cast(T* instance)
    {
        static_assert(IsCompleteType<BASE>, "Base type must be completely declared, missing include ?");
        static_assert(IsCompleteType<T>, "Type must be completely declared, missing include ?");

        if (instance == nullptr)
            return nullptr;
        if (IsA<BASE>(*instance))
            return static_cast<BASE*>(instance);
        return nullptr;
    }

    template<typename BASE, typename T>
    const BASE* Cast(const T* instance)
    {
        return Cast<BASE, T>(const_cast<T>(instance));
    }
}

// macros fun!

#define ETI_PROPERTIES(...) __VA_ARGS__

#define ETI_PROPERTY(NAME, ...) ::eti::Property::Make<decltype(NAME)>(#NAME, offsetof(Self, NAME), ::eti::Attribute::GetAttributes(__VA_ARGS__))

#define ETI_PROPERTY_INTERNAL(...) \
    static const std::span<::eti::Property> GetProperties() \
    { \
        static std::vector<::eti::Property> properties = { __VA_ARGS__ }; \
        return properties; \
    }

#define ETI_METHODS(...) __VA_ARGS__

#define ETI_METHOD(NAME, ...) \
    ::eti::Method::Make(#NAME,  \
    [](void* obj, void* _return, std::span<void*> args) \
    { \
        ::eti::CallFunction(&NAME, obj, _return, args); \
    }, \
    GetFunctionReturn(&NAME), \
    GetFunctionVariables(&NAME))

#define ETI_METHOD_INTERNAL(...) \
    static const std::span<::eti::Method> GetMethods() \
    { \
        static std::vector<::eti::Method> methods = { __VA_ARGS__ }; \
        return methods; \
    }

#define ETI_BASE(BASE, PROPERTIES, METHODS) \
    public: \
        using Self = BASE; \
        virtual const ::eti::Type& GetType() const { return GetTypeStatic(); } \
        ETI_TYPE_DECL_INTERNAL(BASE, nullptr, ::eti::Kind::Class, PROPERTIES, METHODS) \
        ETI_PROPERTY_INTERNAL(PROPERTIES) \
        ETI_METHOD_INTERNAL(METHODS) \
    private:

#define ETI_BASE_SLIM(CLASS) \
    ETI_BASE(CLASS, ETI_PROPERTIES(), ETI_METHODS())

#define ETI_CLASS(CLASS, BASE, PROPERTIES, METHODS) \
    public: \
        using Self = CLASS; \
        using Super = BASE; \
        const ::eti::Type& GetType() const override { return GetTypeStatic(); }\
        ETI_TYPE_DECL_INTERNAL(CLASS, &::eti::TypeOf<BASE>(), ::eti::Kind::Class, PROPERTIES, METHODS) \
        ETI_PROPERTY_INTERNAL(PROPERTIES) \
        ETI_METHOD_INTERNAL(METHODS) \
    private: 

#define ETI_CLASS_SLIM(CLASS, BASE) \
    ETI_CLASS(CLASS, BASE, ETI_PROPERTIES(), ETI_METHODS())

#define ETI_TYPE_DECL_INTERNAL(TYPE, PARENT, KIND, PROPERTIES, METHODS) \
    using Self = TYPE; \
    static constexpr ::eti::TypeId TypeId = ::eti::GetTypeId<TYPE>();\
    static const ::eti::Type& GetTypeStatic()  \
    {  \
        static ::eti::Type type = ::eti::__InternalGetType<TYPE>(KIND, PARENT, TYPE::GetProperties(), TYPE::GetMethods(), {}); \
        return type; \
    } \

#define ETI_STRUCT(STRUCT, PROPERTIES, METHODS) \
    ETI_TYPE_DECL_INTERNAL(STRUCT, nullptr, ::eti::Kind::Struct, PROPERTIES, METHODS) \
    ETI_PROPERTY_INTERNAL(PROPERTIES) \
    ETI_METHOD_INTERNAL(METHODS)

#define ETI_STRUCT_SLIM(STRUCT) \
    ETI_STRUCT(STRUCT, ETI_PROPERTIES(), ETI_METHODS())

#define ETI_TYPE_IMPL(TYPE, KIND, PARENT, PROPERTY_VARIABLES) \
    namespace eti \
    { \
        template<> \
        struct TypeOfImpl<TYPE> \
        { \
            static const ::eti::Type& GetTypeStatic() \
            { \
                static ::eti::Type type = ::eti::__InternalGetType<TYPE>(KIND, PARENT, PROPERTY_VARIABLES, {}, {}); \
                return type; \
            } \
        }; \
    }

// use in global namespace
#define ETI_POD(TYPE) \
    ETI_TYPE_IMPL(TYPE, ::eti::Kind::Pod, nullptr, {})

#define ETI_TEMPLATE_1_IMPL(TYPE) \
    namespace eti \
    {  \
        template<typename T1> \
        struct TypeOfImpl<TYPE<T1>>  \
        {  \
            static const Type& GetTypeStatic()  \
            {  \
                static ::eti::Type type = ::eti::__InternalGetType<TYPE<T1>>(::eti::Kind::Template, nullptr, {}, {}, ::eti::TypesOf<T1>());  \
                return type;  \
            } \
        }; \
    }

// use in global namespace
#define ETI_NAMED_POD(T, NAME) \
    namespace eti \
    { \
        template<> \
        constexpr auto GetTypeNameImpl<RawType<T>>() \
        { \
            return ::std::string_view(#NAME); \
        } \
    } \
    ETI_TYPE_IMPL(T, ::eti::Kind::Pod, nullptr, {})


#define ETI_TEMPLATE_2_IMPL(TYPE) \
    namespace eti \
    {  \
        template<typename T1, typename T2> \
        struct TypeOfImpl<TYPE<T1, T2>>  \
        {  \
            static const ::eti::Type& GetTypeStatic()  \
            {  \
                static ::eti::Type::eti:: type = ::eti::__InternalGetType<TYPE<T1, T2>>(::eti::Kind::Template, nullptr, {}, {}, ::eti::TypesOf<T1, T2>());  \
                return type;  \
            } \
        }; \
    }

//ETI_TEMPLATE_2_IMPL(std::vector)
// vector hack util we support Method
//namespace eti 
//{  
//    template<typename T1, typename T2> 
//    struct TypeOfImpl<std::vector<T1, T2>>  
//    {  
//        static const Type& GetTypeStatic()  
//        {
//            static std::vector<Method> methods = 
//            {
//                Method::Make("Size", [](void* obj, void* r, std::span<void*>)
//                {
//                    std::vector<T1, T2>* array = (std::vector<T1, T2>*) obj;
//                    int* returnValue = (int*)r;
//                    *returnValue = (int)array->size();
//                }),
//                Method::Make("Get", nullptr),
//                Method::Make("Add", nullptr),
//                Method::Make("Remove", nullptr)
//            };
//            static Type type = Type::Make<std::vector<T1, T2>>(Kind::Template, nullptr, {},  methods, TypesOf<T1, T2>());
//            return type;  
//        } 
//    }; 
//}

namespace eti
{
    class Attribute
    {
        ETI_BASE(Attribute, ETI_PROPERTIES(), ETI_METHODS())
    public:
        Attribute() {}
        virtual ~Attribute() {}

        template<typename... ARGS>
        static std::vector<std::shared_ptr<Attribute>> GetAttributes(ARGS... args)
        {
            return { std::make_shared<ARGS>(args)... };
        }
    };

    template <typename T>
    const T* Property::GetAttribute() const
    {
        for (const std::shared_ptr<Attribute>& a : Attributes)
        {
            if (IsA<T>(*a))
                return Cast<T>(a.get());
        }
        return nullptr;
    }

}

#if ETI_TRIVIAL_POD

ETI_POD(bool);

ETI_NAMED_POD(std::int8_t, i8);
ETI_NAMED_POD(std::int16_t, i16);
ETI_NAMED_POD(std::int32_t, i32);
ETI_NAMED_POD(std::int64_t, i64);

ETI_NAMED_POD(std::uint8_t, s8);
ETI_NAMED_POD(std::uint16_t, s16);
ETI_NAMED_POD(std::uint32_t, s32);
ETI_NAMED_POD(std::uint64_t, s64);

ETI_NAMED_POD(std::float_t, f32);
ETI_NAMED_POD(std::double_t, f64);

#endif
