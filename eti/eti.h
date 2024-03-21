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

#pragma region Configuration

// Configuration overriding
//
//  1. set ETI_CONFIG_HEADER to 1 to use external header : "eti_config.h"
//  2. or define override before include <eti/eti.h>
//
#define ETI_CONFIG_HEADER 0

#if ETI_CONFIG_HEADER

    #include "eti_config.h"

#else

    // Minimal
    //  when ETI_SLIM_MODE is 1, only IsA<>, dynamic Cast<> and factory functions are available (no properties, no functions, no attributes, ...)
    #ifndef ETI_SLIM_MODE
        #define ETI_SLIM_MODE 0
    #endif

    #ifndef ETI_ASSERT
        // Assert and Error
        #include <cassert>
        #include <memory>
        #define ETI_ASSERT(cond, msg, ...) assert(cond)
        #define ETI_ERROR(msg) assert(true)
        #define ETI_INTERNAL_ASSERT(cond, msg, ...) assert(cond)
        #define ETI_INTERNAL_ERROR(msg) assert(true)
    #endif

    // TypeId/Hashing
    #ifndef ETI_TYPE_ID_TYPE
        #define ETI_TYPE_ID_TYPE std::uint64_t
    #endif

    #ifndef ETI_HASH_FUNCTION
        //#define ETI_HASH_FUNCTION ::eti::utils::HashFNV1
        #define ETI_HASH_FUNCTION ::eti::utils::HashFNV1WithPrime
        #define ETI_HASH_SEED 0xCBF29CE484222325ull
    #endif
        
    #ifndef ETI_TYPE_NAME_FUNCTION
        // Function Type Name
        #define ETI_TYPE_NAME_FUNCTION ::eti::GetTypeNameDefault
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


#define ETI_FULL_MODE !ETI_SLIM_MODE

// todo: implement slim mode
#if ETI_SLIM_MODE
    #error "slim mode not implemented yet"
#endif

#pragma endregion

namespace eti
{
#pragma region Forwards

    using TypeId = ETI_TYPE_ID_TYPE;
    struct Type;
    struct Declaration;
    struct Variable;
    struct Property;
    struct Method;
    enum class Kind : std::uint8_t;

    class Attribute;

    template<typename T>
    const Type& TypeOf();

    template<typename T>
    const Type& TypeOfForward();

    static constexpr std::string_view GetKindName(Kind typeDesc);

#pragma endregion

#pragma region Utils

    namespace utils
    {
        // RawType<T>

        template<typename T>
        struct RawTypeImpl
        {
            using Type = std::decay_t<std::remove_pointer_t<T>>;
        };

        template<typename T>
        using RawType = typename RawTypeImpl<T>::Type;

        // IsCompleteType<T>

        template<typename T, typename = std::void_t<>>
        struct IsCompleteTypeImpl : std::false_type {};

        template<typename T>
        struct IsCompleteTypeImpl<T, std::void_t<decltype(sizeof(T))>> : std::true_type {};

        template<typename T>
        constexpr bool IsCompleteType = IsCompleteTypeImpl<T>::value;

        // IsForwardType<T>

        template<typename T, typename = std::void_t<>>
        struct IsForwardTypeImpl : std::true_type {};

        template<typename T>
        struct IsForwardTypeImpl<T, std::void_t<decltype(sizeof(T))>> : std::false_type {};

        template<typename T>
        constexpr bool IsForwardType = IsForwardTypeImpl<T>::value;

        // IsMethodStatic<T>

        template<typename T>
        struct IsMethodStaticImpl : std::false_type {};

        template<typename RETURN, typename... ARGS>
        struct IsMethodStaticImpl< RETURN(*)(ARGS...) > : std::true_type {};

        template<typename T>
        constexpr bool IsMethodStatic = IsMethodStaticImpl<T>::value;

        // IsMethodConst<T>

        template<typename T>
        struct IsMethodConstImpl : std::false_type {};

        template<typename OBJECT, typename RETURN, typename... ARGS>
        struct IsMethodConstImpl< RETURN(OBJECT::*)(ARGS...) const > : std::true_type {};

        template<typename T>
        constexpr bool IsMethodConst = IsMethodConstImpl<T>::value;

        template <typename... ARGS>
        std::vector<void*> GetVoidPtrFromArgs(ARGS... args)
        {
            return { args... };
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
            return std::make_tuple(&utils::VoidPrtToTypeArg<ARGS>(args[Is])...);
        }


        // static function call

        // static function call with return value
        template<typename RETURN, typename... ARGS>
        struct CallStaticFunctionImpl
        {
            static void Call(RETURN(*func)(ARGS...), void* obj, void* ret, std::span<void*> args)
            {
                ETI_ASSERT(ret != nullptr, "call function that return void should have ret arg to nullptr");
                auto args_tuple = utils::VoidArgsToTuple<ARGS...>(args, std::index_sequence_for<ARGS...>{});
                std::apply([&](auto... applyArgs) { *((RETURN*)ret) = func(*applyArgs...); }, args_tuple);
            }
        };

        // static function call no return
        template<typename... ARGS>
        struct CallStaticFunctionImpl<void, ARGS...>
        {
            static void Call(void(*func)(ARGS...), void* obj, void* ret, std::span<void*> args)
            {
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
            ETI_ASSERT(obj == nullptr, "call static function should have obj to nullptr");
            CallStaticFunctionImpl<RETURN, ARGS...>::Call(func, obj, ret, args);
        }

        // member function call

        // member function call with return value
        template<typename OBJECT, typename RETURN, typename... ARGS>
        struct CallMemberFunctionImpl
        {
            static void Call(RETURN(OBJECT::* func)(ARGS...), void* obj, void* ret, std::span<void*> args)
            {
                ETI_ASSERT(ret != nullptr, "internal error");
                auto args_tuple = utils::VoidArgsToTuple<ARGS...>(args, std::index_sequence_for<ARGS...>{});
                std::apply([&](auto... applyArgs) { *((RETURN*)ret) = ((OBJECT*)obj->*func)(*applyArgs...); }, args_tuple);

            }
        };

        // member function call void return
        template<typename OBJECT, typename... ARGS>
        struct CallMemberFunctionImpl<OBJECT, void, ARGS...>
        {
            static void Call(void(OBJECT::* func)(ARGS...), void* obj, void* ret, std::span<void*> args)
            {
                ETI_ASSERT(ret == nullptr, "internal error");
                auto args_tuple = utils::VoidArgsToTuple<ARGS...>(args, std::index_sequence_for<ARGS...>{});
                std::apply([&](auto... applyArgs) { ((OBJECT*)obj->*func)(*applyArgs...); }, args_tuple);
            }
        };

        // member function call switch
        template<typename OBJECT, typename RETURN, typename... ARGS>
        void CallFunction(RETURN(OBJECT::* func)(ARGS...), void* obj, void* ret, std::span<void*> args)
        {
            ETI_ASSERT(sizeof...(ARGS) == args.size(), "invalid size of args");
            ETI_ASSERT(obj != nullptr, "internal error");
            CallMemberFunctionImpl<OBJECT, RETURN, ARGS...>::Call(func, obj, ret, args);
        }

        // utility to know at compile time if a T have static method : const Type& GetTypeStatic()
        //
        // ex: if constexpr (HaveGetTypeStatic<T>) {...}
        template<typename T>
        struct HaveGetTypeStaticImpl
        {
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
            static constexpr bool value = type::value;
        };

        template<typename T>
        static constexpr bool HaveGetTypeStatic = HaveGetTypeStaticImpl<T>::value;

        template<typename T>
        static std::function<void(void* /* dst */)> GetConstruct()
        {
            if constexpr (std::is_default_constructible_v<T>)
                return [](void* dst) { new (dst) T(); };
            else
                return nullptr;
        }

        template<typename T>
        static std::function<void(void* /* src */, void* /* dst */)> GetCopyConstruct()
        {
            if constexpr (std::is_copy_constructible_v<T>)
                return [](void* src, void* dst) { new (dst) T(*(T*)src); };
            else
                return nullptr;
        }

        template<typename T>
        static std::function<void(void* /* src */, void* /* dst */)> GetMoveConstruct()
        {
            if constexpr (std::is_default_constructible_v<T>)
                return [](void* src, void* dst) { new (dst) T(std::move(*(T*)src)); };
            else
                return nullptr;
        }

        template<typename T>
        static std::function<void(void* /* dst */)> GetDestruct()
        {
            return [](void* dst) { ((T*)dst)->~T(); };
        }
    }

#pragma endregion

#pragma region Internal

    //internal forwards
    namespace internal
    {
        //
        // Declaration

        template <typename T>
        static Declaration MakeDeclaration();

        //
        // Variable

        static Variable MakeVariable(std::string_view name, ::eti::Declaration declaration);

        template<typename T>
        const Variable* GetVariableInstance(std::string_view name);

        template<typename... ARGS>
        std::span<Variable> GetVariableInstances();

        //
        // Methods

        template<typename RETURN, typename... ARGS>
        const Variable* GetFunctionReturn(RETURN(*func)(ARGS...));

        template<typename OBJECT, typename RETURN, typename... ARGS>
        const Variable* GetFunctionReturn(RETURN(OBJECT::* func)(ARGS...));

        template<typename RETURN, typename... ARGS>
        std::span<Variable> GetFunctionVariables(RETURN(*func)(ARGS...));

        template<typename OBJECT, typename RETURN, typename... ARGS>
        std::span<Variable> GetFunctionVariables(RETURN(OBJECT::* func)(ARGS...));

        static Method MethodMake(std::string_view name, bool isStatic, bool isConst, const Type& parent, std::function<void(void*, void*, std::span<void*>)>&& function, const Variable* _return = nullptr, std::span<const Variable> arguments = {});

        //
        // Property

        template <typename T>
        static Property MakeProperty(std::string_view name, size_t offset, std::vector<std::shared_ptr<Attribute>>&& attributes = {});

        //
        // Type

        template<typename T>
        static const Type MakeType(::eti::Kind kind, const Type* parent, std::span<const Property> properties = {}, std::span<const Method> methods = {}, std::span<const Type*> templates = {});

        template<typename T>
        Type GetTypeInstance(::eti::Kind kind, const Type* parent, std::span<const Property> properties = {}, std::span<const Method> methods = {}, std::span<const Type*> templates = {});

        template<typename... ARGS>
        std::span<const Type*> GetTypeInstances();

        // internal Utils
        // use static const Type& T::GetTypeStatic(){...} if available
        template<typename T>
        const Type& OwnerGetType();
    }

#pragma endregion

#pragma region ETI

    // GetTypeName
    //  return constexpr name for any given types

# if defined __clang__ || defined __GNUC__
    template<typename T>
    constexpr auto GetTypeNameDefault()
    {
        std::string_view pretty_function{ __PRETTY_FUNCTION__ };
        auto first = pretty_function.find_first_not_of(' ', pretty_function.find_first_of('=') + 1);
        auto value = pretty_function.substr(first, pretty_function.find_last_of(']') - first);
        return value;
    }
#elif defined _MSC_VER
    template<typename T>
    constexpr auto GetTypeNameDefault()
    {
        std::string_view pretty_function{ __FUNCSIG__ };
        auto first = pretty_function.find_first_not_of(' ', pretty_function.find_first_of('<') + 1);
        auto value = pretty_function.substr(first, pretty_function.find_last_of('>') - first);
        return value;
    }
#else
    #pragma error implement GetTypeNameDefault
#endif

    // user may specialize this per type to have custom name (be careful to hash clash)
    //  ex: use in ETI_POD_NAMED macro
    template<typename T>
    constexpr auto GetTypeNameImpl()
    {
        return ETI_TYPE_NAME_FUNCTION<T>();
    }

    template<typename T>
    constexpr auto GetTypeName()
    {
        return GetTypeNameImpl<utils::RawType<T>>();
    }

    template<typename T>
    constexpr TypeId GetTypeId()
    {
        ETI_ERROR("GetTypeId not defined for this type");
        return utils::GetStringHash(GetTypeName<T>());
    }

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

    // return constexpr name for Kind
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
            case Kind::Forward:
                return "forward";
        }
    }

    // Declaration, store information about type and it's modifier
    struct Declaration
    {
        const Type& Type;
        bool IsPtr = false;
        bool IsRef = false;     // todo: ref are considered as ptr for now
        bool IsConst = false;
    };

    // Variable, used for function args, property...
    struct Variable
    {
        std::string_view Name;
        Declaration Declaration;
    };

    // Property: member variable of class/struct
    struct Property
    {
        Variable Variable;
        size_t Offset;
        TypeId PropertyId = 0;
        std::vector<std::shared_ptr<Attribute>> Attributes;
        const Type* Parent = nullptr; // todo

        template <typename T>
        const T* GetAttribute() const;

        template <typename T>
        const T* HaveAttribute() const;

        template <typename OBJECT, typename T>
        void Set(OBJECT& obj, const T& value) const;

        template <typename OBJECT, typename T>
        void Get(OBJECT& obj, T& value) const;
    };

    // Method: method and static method of class/struct
    struct Method
    {
        std::string_view Name;
        TypeId MethodId = 0;
        bool IsStatic = false;
        bool IsConst = false;
        std::function<void(void*, void*, std::span<void*>)> Function;
        const Variable* Return;
        std::span<const Variable> Arguments;
        const Type* Parent = nullptr;

        template <typename T>
        const T* GetAttribute() const;

        template <typename T>
        const T* HaveAttribute() const;

        template <typename... ARGS>
        bool IsValidArgs();

        template <typename OWNER, typename RETURN, typename... ARGS>
        void CallMethod(OWNER& owner, RETURN* ret, ARGS... args) const;

        template <typename RETURN, typename... ARGS>
        void CallStaticMethod(RETURN* ret, ARGS... args) const;
    };

    // Type, core eti type, represent runtime type information about any T
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
        std::function<void(void* /* src */, void* /* dst */)> MoveConstruct;
        std::function<void(void* /* dst */)> Destruct;

        std::span<const Property> Properties;
        std::span<const Method> Methods;
        std::span<const Type*> Templates;

        bool operator==(const Type& other) const { return Id == other.Id; }
        bool operator!=(const Type& other) const { return !(*this == other); }

        bool HaveConstruct() const { return Construct != nullptr; }
        bool HaveCopyConstruct() const { return CopyConstruct != nullptr; }
        bool HaveMove() const { return MoveConstruct != nullptr; }
        bool HaveDestroy() const { return Destruct != nullptr; }

        const Property* GetProperty(std::string_view name) const;
        const Property* GetProperty(TypeId propertyId) const;
        const Method* GetMethod(std::string_view name) const;
        const Method* GetMethod(TypeId methodId) const;

        template<typename T>
        T* New() const;

        template<typename T>
        T* NewCopy(const T& other) const;

        template<typename T>
        void Delete(T* ptr) const;

        template<typename FROM, typename TO>
        void Move(FROM& from, TO& to) const;
    };

    // default impl of TypeOfImpl::GetTypeStatic(), should be specialized
    // undeclared type (not using ETI_* macro automatically fallback here as Kind::Unknown type
    template<typename T>
    struct TypeOfImpl
    {
        static const Type& GetTypeStatic();
    };

    // IsA

    constexpr bool IsA(const Type& type, const Type& base);

    template<typename BASE, typename T>
    bool IsA(const T& instance);

    template<typename T, typename BASE>
    constexpr bool IsATyped();

    // Cast

    template<typename BASE, typename T>
    BASE* Cast(T* instance);

    template<typename BASE, typename T>
    const BASE* Cast(const T* instance);

#pragma endregion

}

#pragma region Macros

#define ETI_PROPERTIES(...) __VA_ARGS__

#define ETI_PROPERTY(NAME, ...) ::eti::internal::MakeProperty<decltype(NAME)>(#NAME, offsetof(Self, NAME), ::eti::Attribute::GetAttributes(__VA_ARGS__))

#define ETI_PROPERTY_INTERNAL(...) \
    static const std::span<::eti::Property> GetProperties() \
    { \
        static std::vector<::eti::Property> properties = { __VA_ARGS__ }; \
        return properties; \
    }

#define ETI_METHODS(...) __VA_ARGS__

#define ETI_METHOD(NAME, ...) \
    ::eti::internal::MethodMake(#NAME, \
    ::eti::utils::IsMethodStatic<decltype(&Self::NAME)>, \
    ::eti::utils::IsMethodConst<decltype(&Self::NAME)>, \
    TypeOf<Self>(), \
    [](void* obj, void* _return, std::span<void*> args) \
    { \
        ::eti::utils::CallFunction(&NAME, obj, _return, args); \
    }, \
    ::eti::internal::GetFunctionReturn(&NAME), \
    ::eti::internal::GetFunctionVariables(&NAME))

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

    // use init pattern to prevent infinite recursion (like in method with Self* as arguments)
    #define ETI_TYPE_DECL_INTERNAL(TYPE, PARENT, KIND, PROPERTIES, METHODS) \
    using Self = TYPE; \
    static constexpr ::eti::TypeId TypeId = ::eti::GetTypeId<TYPE>();\
    static const ::eti::Type& GetTypeStatic()  \
    {  \
        static bool initializing = false; \
        static ::eti::Type type; \
        if (initializing == false) \
        { \
            initializing = true; \
            type = ::eti::internal::GetTypeInstance<TYPE>(KIND, PARENT, TYPE::GetProperties(), TYPE::GetMethods(), {}); \
        } \
        return type; \
    }


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
                static ::eti::Type type = ::eti::internal::GetTypeInstance<TYPE>(KIND, PARENT, PROPERTY_VARIABLES, {}, {}); \
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
                static ::eti::Type type = ::eti::internal::GetTypeInstance<TYPE<T1>>(::eti::Kind::Template, nullptr, {}, {}, ::eti::TypesOf<T1>());  \
                return type;  \
            } \
        }; \
    }

// use in global namespace

#define ETI_POD_NAMED(T, NAME) \
    namespace eti \
    { \
        template<> \
        constexpr auto GetTypeNameImpl<::eti::utils::RawType<T>>() \
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
                static ::eti::Type::eti:: type = ::eti::internal::GetTypeInstance<TYPE<T1, T2>>(::eti::Kind::Template, nullptr, {}, {}, ::eti::TypesOf<T1, T2>());  \
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
//                internal::MethodMake("Size", [](void* obj, void* r, std::span<void*>)
//                {
//                    std::vector<T1, T2>* array = (std::vector<T1, T2>*) obj;
//                    int* returnValue = (int*)r;
//                    *returnValue = (int)array->size();
//                }),
//                internal::MethodMake("Get", nullptr),
//                internal::MethodMake("Add", nullptr),
//                internal::MethodMake("Remove", nullptr)
//            };
//            static Type type = Type::MakeType<std::vector<T1, T2>>(Kind::Template, nullptr, {},  methods, TypesOf<T1, T2>());
//            return type;  
//        } 
//    }; 
//}

#pragma endregion

namespace eti
{

#pragma region Internal Implementation

    namespace internal
    {
        //
        // Declaration

        template <typename T>
        static Declaration MakeDeclaration()
        {
            return
            {
                TypeOfForward<T>(),
                std::is_pointer_v<T>,
                std::is_const_v<T>,
            };
        }

        //
        // Variable

        static Variable MakeVariable(std::string_view name, ::eti::Declaration declaration)
        {
            return
            {
                name,
                declaration,
            };
        }

        template<typename T>
        const Variable* GetVariableInstance(std::string_view name)
        {
            static Variable variable = internal::MakeVariable(name, internal::MakeDeclaration<T>());
            return &variable;
        }

        template<typename... ARGS>
        std::span<Variable> GetVariableInstances()
        {
            static std::vector<Variable> variables = { internal::MakeVariable("", internal::MakeDeclaration<ARGS>())... };
            return variables;
        }

        //
        // Methods

        template<typename RETURN, typename... ARGS>
        const Variable* GetFunctionReturn(RETURN(*func)(ARGS...))
        {
            return internal::GetVariableInstance<RETURN>("");
        }

        template<typename OBJECT, typename RETURN, typename... ARGS>
        const Variable* GetFunctionReturn(RETURN(OBJECT::* func)(ARGS...))
        {
            return internal::GetVariableInstance<RETURN>("");
        }

        template<typename RETURN, typename... ARGS>
        std::span<Variable> GetFunctionVariables(RETURN(*func)(ARGS...))
        {
            return internal::GetVariableInstances<ARGS...>();
        }

        template<typename OBJECT, typename RETURN, typename... ARGS>
        std::span<Variable> GetFunctionVariables(RETURN(OBJECT::* func)(ARGS...))
        {
            return internal::GetVariableInstances<ARGS...>();
        }

        //
        // Property

        template <typename T>
        Property MakeProperty(std::string_view name, size_t offset, std::vector<std::shared_ptr<Attribute>>&& attributes /*= {}*/)
        {
            return
            {
                ::eti::internal::MakeVariable(name, ::eti::internal::MakeDeclaration<T>()),
                offset,
                utils::GetStringHash(name),
                std::move(attributes)
            };
        }

        //
        // Method

        inline Method MethodMake(std::string_view name, bool isStatic, bool isConst, const Type& parent, std::function<void(void*, void*, std::span<void*>)>&& function, const Variable* _return /*= nullptr*/, std::span<const Variable> arguments /*= {}*/)
        {
            return
            {
                name,
                utils::GetStringHash(name),
                isStatic,
                isConst,
                function,
                _return,
                arguments,
                &parent,
                // todo: attributes
            };
        }

        template<typename T>
        static const Type MakeType(::eti::Kind kind, const Type* parent, std::span<const Property> properties /*= {}*/, std::span<const Method> methods /*= {}*/, std::span<const Type*> templates /*= {}*/)
        {
            if constexpr (std::is_void<T>::value == false)
            {
                if  constexpr (utils::IsCompleteType<T>)
                {
                    return
                    {
                        GetTypeName<T>(),
                        GetTypeId<T>(),
                        kind,
                        sizeof(T),
                        alignof(T),
                        parent,
                        utils::GetConstruct<T>(),
                        utils::GetCopyConstruct<T>(),
                        utils::GetMoveConstruct<T>(),
                        utils::GetDestruct<T>(),
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

        template<typename T>
        Type GetTypeInstance(::eti::Kind kind, const Type* parent, std::span<const Property> properties /*= {}*/, std::span<const Method> methods /*= {}*/, std::span<const Type*> templates /*= {}*/)
        {
            static Type type = internal::MakeType<T>(kind, parent, properties, methods, templates);
            // type may be Forward type, in this case update it on demand
            if (type.Kind == Kind::Forward && utils::IsCompleteType<T>)
                type = internal::MakeType<T>(kind, parent, properties, methods, templates);

            return type;
        }

        template<typename... ARGS>
        std::span<const Type*> GetTypeInstances()
        {
            static const Type* types[] = { &TypeOfForward<ARGS>()... };
            return std::span<const Type*>(types, sizeof...(ARGS));
        }

        template<typename T>
        const Type& OwnerGetType()
        {
            if constexpr (utils::HaveGetTypeStatic<T>)
                return T::GetTypeStatic();
            else
                return TypeOfImpl<utils::RawType<T>>::template GetTypeStatic();
        }

    }

#pragma endregion

#pragma region Property Implementation

    template <typename T>
    const T* Property::HaveAttribute() const
    {
        return GetAttribute<T>() != nullptr;
    }

    template <typename OBJECT, typename T>
    void Property::Set(OBJECT& obj, const T& value) const
    {
        // ETI_ASSERT(IsA(obj, *Parent)); todo
        ETI_ASSERT(Variable.Declaration.Type == TypeOf<T>(), "invalid type");
        T* ptr = (T*)((char*)&obj + Offset);
        *ptr = value;
    }

    template <typename OBJECT, typename T>
    void Property::Get(OBJECT& obj, T& value) const
    {
        // ETI_ASSERT(IsA(obj, *Parent)); todo
        ETI_ASSERT(Variable.Declaration.Type == TypeOf<T>(), "invalid type");
        T* ptr = (T*)((char*)&obj + Offset);
        value = *ptr;
    }

#pragma endregion

#pragma region Method Implementation

    template <typename T>
    const T* Method::GetAttribute() const
    {
        // not implemented
        return nullptr;
    }

    template <typename T>
    const T* Method::HaveAttribute() const
    {
        return GetAttribute<T>() != nullptr;
    }

    template <typename... ARGS>
    bool Method::IsValidArgs()
    {
        // todo, validate each args using Arguments
        return true;
    }

    template <typename OWNER, typename RETURN, typename... ARGS>
    void Method::CallMethod(OWNER& owner, RETURN* ret, ARGS... args) const
    {
        if (ret == nullptr)
            ETI_ASSERT(Return->Declaration.Type.Kind == Kind::Void, "cannot provide return value on method returning void: %s::%s()", Parent->Name, Name);
        else
            ETI_ASSERT(Return->Declaration.Type.Kind != Kind::Void, "missing return value on method with return: %s::%s()", Parent->Name, Name);

        //ETI_ASSERT(IsValidArgs<ARGS...>(), "invalid arguments calling method: %s::%s()", Parent->Name, Name);

        std::vector<void*> voidArgs = utils::GetVoidPtrFromArgs(args...);

        Function(&owner, ret, voidArgs);

    }
     
    template <typename RETURN, typename... ARGS>
    void Method::CallStaticMethod(RETURN* ret, ARGS... args) const
    {
        if (ret == nullptr)
            ETI_ASSERT(Return->Declaration.Type.Kind == Kind::Void, "cannot provide return value on method returning void: %s::%s()", Parent->Name, Name);
        else
            ETI_ASSERT(Return->Declaration.Type.Kind != Kind::Void, "missing return value on method with return: %s::%s()", Parent->Name, Name);

        //ETI_ASSERT(IsValidArgs<ARGS...>(), "invalid arguments calling method: %s::%s()", Parent->Name, Name);

        std::vector<void*> voidArgs = utils::GetVoidPtrFromArgs(args...);

        Function(nullptr, ret, voidArgs);
    }

#pragma endregion

#pragma region Attribute Implementation

    inline const Property* Type::GetProperty(std::string_view name) const
    {
        auto it = std::ranges::find_if(Properties, [name](const Property& it) { return it.Variable.Name == name; });
        return (it != Properties.end()) ? &(*it) : nullptr;
    }

    inline const Property* Type::GetProperty(TypeId propertyId) const
    {
        auto it = std::ranges::find_if(Properties, [propertyId](const Property& it) { return it.PropertyId == propertyId; });
        return (it != Properties.end()) ? &(*it) : nullptr;
    }

    inline const Method* Type::GetMethod(std::string_view name) const
    {
        auto it = std::ranges::find_if(Methods, [name](const Method& it) { return it.Name == name; });
        return (it != Methods.end()) ? &(*it) : nullptr;
    }

    inline const Method* Type::GetMethod(TypeId methodId) const
    {
        auto it = std::ranges::find_if(Methods, [methodId](const Method& it) { return it.MethodId == methodId; });
        return (it != Methods.end()) ? &(*it) : nullptr;
    }

    template<typename T>
    T* Type::New() const
    {
        ETI_ASSERT(IsA(TypeOf<T>(), *this), "try to call Type::New with non compatible type");
        ETI_ASSERT(HaveConstruct(), "try to call Type::New on Type without Construct");
        void* memory = _aligned_malloc(Size, Align);
        ETI_ASSERT(memory, "out of memory on Type::New call");
        Construct(memory);
        return static_cast<T*>(memory);
    }

    template<typename T>
    T* Type::NewCopy(const T& other) const
    {
        ETI_ASSERT(IsA(TypeOf<T>(), *this), "try to call Type::New with non compatible type");
        ETI_ASSERT(HaveCopyConstruct(), "try to call Type::New on Type without Construct");
        void* memory = _aligned_malloc(Size, Align);
        ETI_ASSERT(memory, "out of memory on Type::New call");
        CopyConstruct((void*) & other, memory);
        return static_cast<T*>(memory);
    }

    template<typename T>
    void Type::Delete(T* ptr) const
    {
        ETI_ASSERT(IsA(TypeOf<T>(), *this), "try to call Type::Delete with non compatible type");
        ETI_ASSERT(HaveDestroy(), "try to call Type::Delete on Type without Destroy");
        // support null delete
        if (ptr == nullptr)
            return;
        Destruct(ptr);
    }

    template<typename FROM, typename TO>
    void Type::Move(FROM& from, TO& to) const
    {
        ETI_ASSERT(TypeOf<FROM>().Id == TypeOf<TO>().Id, "Move should be call using same type");
        ETI_ASSERT(HaveMove(), "try to call Type::Move on Type without MoveConstruct");
        MoveConstruct(&from, &to);
    }

#pragma endregion

#pragma region ETI Implementation

    // default impl of TypeOfImpl::GetTypeStatic(), should be specialized
    // undeclared type (not using ETI_* macro automatically fallback here as Kind::Unknown type
    template<typename T>
    const Type& TypeOfImpl<T>::GetTypeStatic()
    {
            static Type type = internal::MakeType<T>(Kind::Unknown, nullptr, {}, {}, {});
            return type;
    }

    template<typename T>
    const Type& TypeOf()
    {
        static_assert(utils::IsCompleteType<T>, "Type must be completely declared, missing include ?");
        return internal::OwnerGetType<T>();
    }

    template<typename T>
    const Type& TypeOfForward()
    {
        return internal::OwnerGetType<T>();
    }

    inline constexpr bool IsA(const Type& type, const Type& base)
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
        static_assert(utils::IsCompleteType<BASE>, "Base type must be completely declared, missing include ?");
        static_assert(utils::IsCompleteType<T>, "Type must be completely declared, missing include ?");
        return IsA(instance.GetType(), TypeOf<BASE>());
    }

    template<typename T, typename BASE>
    constexpr bool IsATyped()
    {
        static_assert(utils::IsCompleteType<BASE>, "Base type must be completely declared, missing include ?");
        static_assert(utils::IsCompleteType<T>, "Type must be completely declared, missing include ?");
        return IsA(TypeOf<T>(), TypeOf<BASE>());
    }

    template<typename BASE, typename T>
    BASE* Cast(T* instance)
    {
        static_assert(utils::IsCompleteType<BASE>, "Base type must be completely declared, missing include ?");
        static_assert(utils::IsCompleteType<T>, "Type must be completely declared, missing include ?");

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

#pragma endregion

#pragma region Attribute

    // Attribute decl and impl at the end, sinec it's using type information
    class Attribute
    {
        ETI_BASE(Attribute, ETI_PROPERTIES(), ETI_METHODS())
    public:
        Attribute() {}
        virtual ~Attribute() {}

        template<typename... ARGS>
        static std::vector<std::shared_ptr<Attribute>> GetAttributes(ARGS... args);
    };

    template<typename... ARGS>
    std::vector<std::shared_ptr<Attribute>> Attribute::GetAttributes(ARGS... args)
    {
        return { std::make_shared<ARGS>(args)... };
    }

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

#pragma endregion
}

#if ETI_TRIVIAL_POD

ETI_POD(bool);

ETI_POD_NAMED(std::int8_t, i8);
ETI_POD_NAMED(std::int16_t, i16);
ETI_POD_NAMED(std::int32_t, i32);
ETI_POD_NAMED(std::int64_t, i64);

ETI_POD_NAMED(std::uint8_t, s8);
ETI_POD_NAMED(std::uint16_t, s16);
ETI_POD_NAMED(std::uint32_t, s32);
ETI_POD_NAMED(std::uint64_t, s64);

ETI_POD_NAMED(std::float_t, f32);
ETI_POD_NAMED(std::double_t, f64);

#endif
