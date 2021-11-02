#pragma once


#include "vec_type.h"
#include "vec_traits.h"


ZENO_NAMESPACE_BEGIN
namespace math {


template <size_t N, is_not_vec T1, class Func>
constexpr auto vec_wise(T1 const &t1, Func &&func) {
    return func(t1);
}

template <size_t N, is_not_vec T1, class Func>
constexpr auto vec_wise(vec<N, T1> const &t1, Func &&func) {
    using T0 = std::remove_cvref_t<std::invoke_result_t<Func, T1>>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1[i]);
    }
    return t0;
}

template <is_not_vec T1, is_not_vec T2, class Func>
constexpr auto vec_wise(T1 const &t1, T2 const &t2, Func &&func) {
    return func(t1, t2);
}

template <size_t N, is_not_vec T1, is_not_vec T2, class Func>
constexpr auto vec_wise(vec<N, T1> const &t1, T2 const &t2, Func &&func) {
    using T0 = std::remove_cvref_t<std::invoke_result_t<Func, T1, T2>>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1[i], t2);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, class Func>
constexpr auto vec_wise(T1 const &t1, vec<N, T2> const &t2, Func &&func) {
    using T0 = std::remove_cvref_t<std::invoke_result_t<Func, T1, T2>>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1, t2[i]);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, class Func>
constexpr auto vec_wise(vec<N, T1> const &t1, vec<N, T2> const &t2, Func &&func) {
    using T0 = std::remove_cvref_t<std::invoke_result_t<Func, T1, T2>>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1[i], t2[i]);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, is_not_vec T3, class Func>
constexpr auto vec_wise(vec<N, T1> const &t1, T2 const &t2, T3 const &t3, Func &&func) {
    using T0 = std::invoke_result_t<Func, T1, T2, T3>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1[i], t2, t3);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, is_not_vec T3, class Func>
constexpr auto vec_wise(T1 const &t1, vec<N, T2> const &t2, T3 const &t3, Func &&func) {
    using T0 = std::invoke_result_t<Func, T1, T2, T3>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1, t2[i], t3);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, is_not_vec T3, class Func>
constexpr auto vec_wise(T1 const &t1, T2 const &t2, vec<N, T3> const &t3, Func &&func) {
    using T0 = std::invoke_result_t<Func, T1, T2, T3>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1, t2, t3[i]);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, is_not_vec T3, class Func>
constexpr auto vec_wise(T1 const &t1, vec<N, T2> const &t2, vec<N, T3> const &t3, Func &&func) {
    using T0 = std::invoke_result_t<Func, T1, T2, T3>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1, t2[i], t3[i]);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, is_not_vec T3, class Func>
constexpr auto vec_wise(vec<N, T1> const &t1, T2 const &t2, vec<N, T3> const &t3, Func &&func) {
    using T0 = std::invoke_result_t<Func, T1, T2, T3>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1[i], t2, t3[i]);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, is_not_vec T3, class Func>
constexpr auto vec_wise(vec<N, T1> const &t1, vec<N, T2> const &t2, T3 const &t3, Func &&func) {
    using T0 = std::invoke_result_t<Func, T1, T2, T3>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1[i], t2[i], t3);
    }
    return t0;
}

template <size_t N, is_not_vec T1, is_not_vec T2, is_not_vec T3, class Func>
constexpr auto vec_wise(vec<N, T1> const &t1, vec<N, T2> const &t2, vec<N, T3> const &t3, Func &&func) {
    using T0 = std::invoke_result_t<Func, T1, T2, T3>;
    vec<N, T0> t0;
    for (size_t i = 0; i < N; i++) {
        t0[i] = func(t1[i], t2[i], t3[i]);
    }
    return t0;
}

template <is_not_vec T1, is_not_vec T2, class Func>
constexpr auto &vec_wise_assign(T1 &t1, T2 const &t2, Func &&func) {
    func(t1, t2);
    return t1;
}

template <size_t N, is_not_vec T1, is_not_vec T2, class Func>
constexpr auto &vec_wise_assign(vec<N, T1> &t1, T2 const &t2, Func &&func) {
    using T0 = std::remove_cvref_t<std::invoke_result_t<Func, T1, T2>>;
    for (size_t i = 0; i < N; i++) {
        func(t1[i], t2);
    }
    return t1;
}

template <size_t N, is_not_vec T1, is_not_vec T2, class Func>
constexpr auto &vec_wise_assign(vec<N, T1> &t1, vec<N, T2> const &t2, Func &&func) {
    for (size_t i = 0; i < N; i++) {
        func(t1[i], t2[i]);
    }
    return t1;
}


}
ZENO_NAMESPACE_END