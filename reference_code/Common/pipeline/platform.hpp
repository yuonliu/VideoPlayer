//
// Created by 于承业 on 2022/4/13.
//

#ifndef GLMP_PLATFORM_HPP
#define GLMP_PLATFORM_HPP
#include<iostream>

#if defined(__clang__) || defined(__GNUC__)

#define CPP_STANDARD __cplusplus

#elif defined(_MSC_VER)

#define CPP_STANDARD _MSVC_LANG

#endif

#if CPP_STANDARD >= 199711L
#define HAS_CPP_03 1
#endif
#if CPP_STANDARD >= 201103L
#define HAS_CPP_11 1
#endif
#if CPP_STANDARD >= 201402L
#define HAS_CPP_14 1
#endif
#if CPP_STANDARD >= 201703L
#define HAS_CPP_17 1
#endif

#if defined(HAS_CPP_17)
template<typename... Ts> struct select_last {
    template<typename T> struct tag{
        using type = T;
    };
    using type = typename decltype((tag<Ts>{}, ...))::type;
};
#else
template< class ...Args> struct select_last;
template< class A> struct select_last<A> { using type = A; };
template< class A, class... Args> struct select_last<A, Args...>{
    using type = typename select_last<Args...>::type;
};
#endif

template<std::size_t I, class... Args> struct select_type{
    using type = typename std::tuple_element<I, std::tuple<Args...>>::type;
};

template <typename T, typename S>
struct pop_back;

template <template <typename...> class C, typename... Args, std::size_t... Is>
struct pop_back<C<Args...>, std::index_sequence<Is...>>
{
    using type = C<std::tuple_element_t<Is, std::tuple<Args...>>...>;
};

//template<class... Args> struct zip {
//    template<class T> struct zip_functions{};
//    template<std::size_t ...N> struct zip_functions<std::index_sequence<N...>>{
//        using type = std::tuple<std::function<std::tuple_element_t<N+1, std::tuple<Args...>>(std::tuple_element_t<N, std::tuple<Args...>>)>...>;
//    };
//    using tuple_functions = zip_functions<std::make_index_sequence<sizeof...(Args) - 1>>;
//};

template<class... Args> struct zip {
    template<class T> struct zip_functions{};
    template<std::size_t ...N> struct zip_functions<std::index_sequence<N...>>{
        using type = std::tuple<std::function<std::tuple_element_t<N+1, std::tuple<Args...>>(std::tuple_element_t<N, std::tuple<Args...>>)>...>;
    };
    using tuple_functions = zip_functions<std::make_index_sequence<sizeof...(Args) - 1>>;
};

template <typename T>
constexpr auto type_name() {
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}


// helpers used to get return type or type of first argument

template<typename F, typename Ret, typename A, typename... Rest>
Ret helper_lambda_ret(Ret (F::*)(A, Rest...));

template<typename F, typename Ret, typename A, typename... Rest>
Ret helper_lambda_ret(Ret (F::*)(A, Rest...) const);

template<typename F, typename Ret, typename A, typename... Rest>
A helper_lambda_arg0(Ret (F::*)(A, Rest...));

template<typename F, typename Ret, typename A, typename... Rest>
A helper_lambda_arg0(Ret (F::*)(A, Rest...) const);


template<typename F>
struct lambda_traits {
    using arg0 = decltype( helper_lambda_arg0(&F::operator()) );
    using ret = decltype( helper_lambda_ret(&F::operator()) );
};

#define PRINT_TYPE_OF_VAR(X) std::cout<<type_name<decltype(X)>()<<std::endl
#define PRINT_TYPE(X) std::cout<<type_name<X>()<<std::endl

#endif //GLMP_PLATFORM_HPP
