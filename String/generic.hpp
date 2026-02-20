// Copyright Scheer PAS Schweiz AG

#ifndef SRC_CORE_MISC_STRING_GENERIC_HPP
#define SRC_CORE_MISC_STRING_GENERIC_HPP

#include <numeric>
#include <string_view>

namespace M::String
{
char* create( size_t Length);

inline std::string_view sv( const char *str ) noexcept
{
    return str ? std::string_view { str } : std::string_view {};
}

// This will take anything convertible to `std::string_view` except
// (potentially cv-qualified) `char*` because we have to guard that one against nulls
template<typename S> requires
    ( !std::is_same_v<char, std::remove_reference_t<std::remove_cv_t<std::remove_pointer_t<S>>>> ) &&
    requires(S s) { std::string_view{ s }; }
inline std::string_view sv( const S &str ) noexcept { return std::string_view { str }; }

// A string is anything, that we have a `sv()` function yielding `std::string_view` for.
template<typename S>
concept StringType =
    ( !std::is_same_v<std::nullptr_t, S> ) &&
    requires( S s ) {
        { sv( s ) } -> std::same_as<std::string_view>;
    };

[[nodiscard]]
inline int compare( const StringType auto& lhs, const StringType auto& rhs ) noexcept
{
    return sv( lhs ).compare( sv( rhs ) );
}

[[nodiscard]]
inline int compare( const StringType auto& lhs, const StringType auto& rhs, size_t length ) noexcept
{
    return sv( lhs ).compare( 0, length, sv( rhs ), 0, length );
}

/**
 * Checks two cstrings for equality up to Length character.
 * Strings are considered equal when:
 *		- both pointers are equal (including both being NULL)
 *		- neither is NULL and they contain the same values at first Length positions
 *
 * @param lhs		First string to compare
 * @param rhs		Second string to compare
 * @param length	Number of characters to compare.
 *					Only positions 0 to ( Length-1) will be compared.
 *
 * @return Whether first Length letters of Left equals first Length letters of Right
 */

[[nodiscard]]
inline bool equal( const StringType auto& lhs, const StringType auto& rhs ) noexcept
{
    // FIXME: At some point we need to stop distinguishing between null-string
    //        and an empty string. For now we have places in the code
    //        where this makes a difference :(
    auto lsv = sv( lhs );
    auto rsv = sv( rhs );
    return ( !lsv.data() == !rsv.data() ) && ( compare( lsv, rsv ) == 0 );
}

[[nodiscard]]
inline bool equal( const StringType auto& lhs, const StringType auto& rhs, size_t length ) noexcept
{
    // FIXME: At some point we need to stop distinguishing between null-string
    //        and an empty string. For now we have places in the code
    //        where this makes a difference :(
    auto lsv = sv( lhs );
    auto rsv = sv( rhs );
    return ( !lsv.data() == !rsv.data() ) && ( compare( lsv, rsv, length ) == 0 );
}

/**
 * Checks if given string is empty (NULL is empty)
 * @param str String to check (could be NULL)
 * @return true if String is NULL or if it's length is 0.
 */
[[nodiscard]]
inline bool empty( const StringType auto &str ) noexcept { return sv( str ).empty(); }

/**
 * Computes the length of the given string
 * @param str String to measure
 * @return The length of the string.
 */
[[nodiscard]]
inline size_t length( const StringType auto &str ) noexcept { return sv( str ).length(); }



} // namespace M::String

#endif //SRC_CORE_MISC_STRING_GENERIC_HPP
