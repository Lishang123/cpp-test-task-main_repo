// ST_String.hpp
//
// Copyright (c) 2002-2015 e2e technologies ltd

#ifndef ST_STRING_HPP
#define ST_STRING_HPP

#include "generic.hpp"
#include <string_view>
#include <vector>
#include <type_traits>
#include <fmt/format.h>

class ST_String
{
	private:
		char* m_String;

	public:
		ST_String();
		explicit ST_String( const char* String);
		ST_String( const char* String, size_t Length);
		explicit ST_String( std::string_view sv );

		ST_String( const ST_String& String);
		ST_String( ST_String&& String ) noexcept;
		~ST_String();

		ST_String& operator=( const char* String);
		ST_String& operator=( std::string_view String);
		ST_String& operator=( const ST_String& String);
		ST_String& operator=( ST_String&& String) noexcept ;

        std::strong_ordering operator<=>( const M::String::StringType auto &rhs ) const noexcept
        {
            return M::String::compare( *this, rhs ) <=> 0;
        }

		[[nodiscard]] bool isEmpty() const;
		[[nodiscard]] size_t length() const;

		/** Set new value.
		  * @param String New value.
		  */
		void set( const char* String);
		void set( std::string_view str );
		void consume( char* String);
		/** Set new value.
		  * @param String New value.
		  * @param Length Lenght to copy.
		  */
		void set( const char* String, size_t Length);

		[[nodiscard]] const char* c_str() const noexcept;
        [[nodiscard]] std::string_view view() const noexcept;
        explicit operator std::string_view() const noexcept { return view(); }

		void reset();

		template<typename Functor>
		auto modifyInPlace( Functor&& f ) -> typename std::invoke_result_t<Functor, char*> { return f( m_String ); };
};

inline bool operator==( const ST_String &lhs, const M::String::StringType auto &rhs ) noexcept
{
    // FIXME: Equality handling is inconsistent in regard to null values among
    //        string types. This has to be normalized at some point.
    using namespace M::String;
    return ( lhs.isEmpty() && empty( rhs ) ) || equal( lhs, rhs );
}

typedef std::vector<ST_String> ST_StringArray;

namespace ST
{

struct KVPairKeyComparator
{
	template<class T>
	bool operator()(const std::pair<ST_String, ST_String>& x, const T& y)
	{
		return x.first < y;
	}

	template<class T>
	bool operator()(const T& x, const std::pair<ST_String, ST_String>& y)
	{
		return x < y.first;
	}

	bool operator()(const std::pair<ST_String, ST_String>& x, const std::pair<ST_String, ST_String>& y)
	{
		return x.first < y.first;
	}
};
}

namespace std {
template <>
struct hash<ST_String> {
    std::size_t operator()( ST_String const& text) const
    {
        return std::hash<std::string_view>{}( text.view() );
    }
};
}

inline std::size_t hash_value( const ST_String& text)
{
	std::hash<ST_String> hasher;
	return hasher( text);
}

namespace fmt
{

template<>
struct formatter<ST_String> : public formatter<std::string_view>
{
    template<typename FormatContext>
    auto format( const ST_String &v, FormatContext &ctx ) const
    {
        return formatter<std::string_view>::format( v.view(), ctx );
    }
};

} // namespace fmt

#endif
