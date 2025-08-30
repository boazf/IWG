#ifndef RESTRICTIONS_H
#define RESTRICTIONS_H
#include <type_traits>

template <std::size_t N>
inline void require_string_literal(const char (&)[N]) noexcept {
    (void)N; // N includes the '\0' (use N-1 for logical length)
}

// 2) Fallback for everything else, with a custom error
// helper for a dependent-false static_assert
template<class> struct dependent_false : std::false_type {};
template <typename T>
inline void require_string_literal(const T&) {
    static_assert(dependent_false<T>::value,
                  "only string literals (char arrays) are allowed here");
}

#define REQUIRE_STRING_LITERAL(s) require_string_literal(s)

#endif // RESTRICTIONS_H