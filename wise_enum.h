#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/vmd/is_tuple.hpp>

#include <algorithm>
#include <array>
#include <utility>

#include <string.h>

#define WISE_ENUM_OPTIONAL boost::optional
#include <boost/optional/optional.hpp>

#define WISE_ENUM_XSTR(s) #s
#define WISE_ENUM_STR(s) WISE_ENUM_XSTR(s)

#define WISE_ENUM_FIRST_OR_ELEM(r, d, enum_value)                              \
  BOOST_PP_IIF(BOOST_VMD_IS_TUPLE(enum_value),                                 \
               BOOST_PP_TUPLE_ELEM(0, enum_value), enum_value)

#define WISE_ENUM_FIRST_OR_CONC(r, d, enum_value)                              \
  BOOST_PP_IIF(BOOST_VMD_IS_TUPLE(enum_value),                                 \
               BOOST_PP_TUPLE_ELEM(0, enum_value) =                            \
                   BOOST_PP_TUPLE_ELEM(1, enum_value),                         \
               enum_value)

#define WISE_ENUM_ENUM_NAMES(seq)                                              \
  BOOST_PP_SEQ_TRANSFORM(WISE_ENUM_FIRST_OR_ELEM, _, seq)

#define WISE_ENUM_SWITCH_TO_STRING(r, enum_name, enum_value)                   \
  case enum_name::enum_value:                                                  \
    return WISE_ENUM_STR(enum_value);

#define WISE_ENUM_ENUM_DESC_PAIR(r, enum_name, enum_value)                     \
  std::make_pair(enum_name::enum_value, WISE_ENUM_STR(enum_value))

#define WISE_ENUM_TO_STRING(name, seq)                                         \
  const char *wise_enum_to_string(name e) {                                    \
    switch (e) {                                                               \
      BOOST_PP_SEQ_FOR_EACH(WISE_ENUM_SWITCH_TO_STRING, name, seq)             \
    }                                                                          \
  }

#define WISE_ENUM_DESC_PAIR_ARRAY(name, seq)                                   \
  constexpr auto wise_enum_descriptor_pair_array(::wise_enum::Tag<name>) {     \
    return std::array<std::pair<name, const char *>, BOOST_PP_SEQ_SIZE(seq)>{  \
        BOOST_PP_SEQ_ENUM(                                                     \
            BOOST_PP_SEQ_TRANSFORM(WISE_ENUM_ENUM_DESC_PAIR, name, seq))};     \
  }

#define WISE_ENUM_NAME_SEQ(name, enum_names)                                   \
  WISE_ENUM_TO_STRING(name, enum_names)                                        \
  WISE_ENUM_DESC_PAIR_ARRAY(name, enum_names)

#define WISE_ENUM_SEQ(name, seq)                                               \
  enum class name {                                                            \
    BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(WISE_ENUM_FIRST_OR_CONC, _, seq)) \
  };                                                                           \
  WISE_ENUM_NAME_SEQ(name, WISE_ENUM_ENUM_NAMES(seq))

#define WISE_ENUM(name, ...)                                                   \
  WISE_ENUM_SEQ(name, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

namespace wise_enum {
namespace detail {

template <class T, std::size_t N>
constexpr std::array<T, N>
desc_array_to_array(const std::array<std::pair<T, const char *>, N> &a) {
  std::array<T, N> r;
  for (std::size_t i = 0; i != N; ++i) {
    r[i] = a[i].first;
  }
  return r;
}
}

template <class T> struct Tag {};

template <class T> const char *to_string(T t) { return wise_enum_to_string(t); }

template <class T>
constexpr auto descriptor_range = wise_enum_descriptor_pair_array(Tag<T>{});

template <class T> constexpr std::size_t size = descriptor_range<T>.size();

template <class T>
constexpr auto range = detail::desc_array_to_array(descriptor_range<T>);

template <class T>
constexpr WISE_ENUM_OPTIONAL<T> from_string(const char *arg) {
  auto it =
      std::find_if(descriptor_range<T>.begin(), descriptor_range<T>.end(),
                   [=](const auto &x) { return ::strcmp(x.second, arg) == 0; });
  if (it == descriptor_range<T>.end())
    return {};

  return it->first;
}
}
