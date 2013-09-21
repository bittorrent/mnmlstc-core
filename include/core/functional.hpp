#ifndef CORE_FUNCTIONAL_HPP
#define CORE_FUNCTIONAL_HPP

#include <core/type_traits.hpp>
#include <core/utility.hpp>
#include <functional>
#include <tuple>
#include <array>

namespace core {
inline namespace v1 {

template <class F> struct function_traits;

template <class R, class... Args>
struct function_traits<R(*)(Args...)> : function_traits<R(Args...)> { };

template <class C, class R>
struct function_traits<R(C::*)> : function_traits<R(C&)> { };

template <class C, class R, class... Args>
struct function_traits<R(C::*)(Args...)> : function_traits<R(C&, Args...)> { };

template <class C, class R, class... Args>
struct function_traits<R(C::*)(Args...) const> :
  function_traits<R(C const&, Args...)>
{ };

template <class R, class... Args>
struct function_traits<std::function<R(Args...)>> :
  function_traits<R(Args...)>
{ };

template <class R, class... Args>
struct function_traits<R(Args...)> {
  using return_type = R;

  static constexpr std::size_t arity = sizeof...(Args);

  template <std::size_t N>
  using argument = typename std::tuple_element<N, std::tuple<Args...>>::type;
};

template <class Functor, class Object, class... Args>
constexpr auto invoke (Functor&& functor, Object&& object, Args&&... args) ->
enable_if_t<
  invokable<Functor, Object, Args...>::value,
  decltype((object.*functor)(std::forward<Args>(args)...))
> { return (object.*functor)(std::forward<Args>(args)...); }

template <class Functor, class Object, class... Args>
constexpr auto invoke (Functor&& functor, Object&& object, Args&&... args) ->
enable_if_t<
  invokable<Functor, Object, Args...>::value,
  decltype(
    ((*std::forward<Object>(object)).*functor)(std::forward<Args>(args)...)
  )
> {
  return (
    (*std::forward<Object>(object)).*functor
  )(std::forward<Args>(args)...);
}

template <class Functor, class Object>
constexpr auto invoke (Functor&& functor, Object&& object) -> enable_if_t<
  invokable<Functor, Object>::value,
  decltype(object.*functor)
> { return object.*functor; }

template <class Functor, class Object>
constexpr auto invoke (Functor&& functor, Object&& object) -> enable_if_t<
  invokable<Functor, Object>::value,
  decltype((*std::forward<Object>(object)).*functor)
> { return (*std::forward<Object>(object)).*functor; }

template <class Functor, class... Args>
constexpr auto invoke (Functor&& functor, Args&&... args) -> invoke_of_t<
  Functor, Args...
> { return std::forward<Functor>(functor)(std::forward<Args>(args)...); }

namespace impl {

template <class Functor, class U, std::size_t... I>
auto unpack (Functor&& functor, U&& unpackable, index_sequence<I...>&&) ->
invoke_of_t<Functor, decltype(std::get<I>(std::forward<U>(unpackable)))...> {
  return ::core::v1::invoke(std::forward<Functor>(functor),
    std::get<I>(std::forward<U>(unpackable))...
  );
}

template <class U, std::size_t... I>
auto unpack (U&& unpackable, index_sequence<I...>&&) -> invoke_of_t<
  decltype(std::get<I>(std::forward<U>(unpackable)))...
> { return ::core::v1::invoke(std::get<I>(std::forward<U>(unpackable))...); }

template <class Functor, class U, std::size_t... I>
auto runpack (Functor&& functor, U&& runpackable, index_sequence<I...>&&) ->
invoke_of_t<Functor, decltype(std::forward<U>(runpackable).at(I))...>
{ return ::core::v1::invoke(std::forward<U>(runpackable).at(I)...); }

template <class U, std::size_t... I>
auto runpack (U&& runpackable, index_sequence<I...>&&) -> invoke_of_t<
  decltype(std::forward<U>(runpackable).at(I))...
> { return ::core::v1::invoke(std::forward<U>(runpackable).at(I)...); }

} /* namespace impl */

struct unpack_t final { };
constexpr unpack_t unpack { };

template <std::size_t N>
using runpack = std::integral_constant<std::size_t, N>;

template <class Functor, class Unpackable>
constexpr auto invoke (unpack_t, Functor&& functor, Unpackable&& unpackable) ->
enable_if_t<
  is_unpackable<decay_t<Unpackable>>::value,
  decltype(
    impl::unpack(
      std::forward<Functor>(functor),
      std::forward<Unpackable>(unpackable),
      make_index_sequence<std::tuple_size<decay_t<Unpackable>>::value> { }
    )
  )
> {
  return impl::unpack(
    std::forward<Functor>(functor),
    std::forward<Unpackable>(unpackable),
    make_index_sequence<std::tuple_size<decay_t<Unpackable>>::value> { }
  );
}

template <class Unpackable>
constexpr auto invoke (unpack_t, Unpackable&& unpackable) ->
enable_if_t<
  is_unpackable<decay_t<Unpackable>>::value,
  decltype(
    impl::unpack(
      std::forward<Unpackable>(unpackable),
      make_index_sequence<std::tuple_size<decay_t<Unpackable>>::value> { }
    )
  )
> {
  return impl::unpack(
    std::forward<Unpackable>(unpackable),
    make_index_sequence<std::tuple_size<decay_t<Unpackable>>::value> { }
  );
}

template <class RuntimeUnpackable, std::size_t N>
constexpr auto invoke (runpack<N>, RuntimeUnpackable&& unpackable) ->
enable_if_t<
  is_runpackable<decay_t<RuntimeUnpackable>>::value,
  decltype(
    impl::runpack(
      std::forward<RuntimeUnpackable>(unpackable),
      make_index_sequence<N> { }
    )
  )
> {
  return impl::runpack(
    std::forward<RuntimeUnpackable>(unpackable),
    make_index_sequence<N> { }
  );
}

}} /* namespace core::v1 */

#endif /* CORE_FUNCTIONAL_HPP */
