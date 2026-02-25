#include <catch2/catch_test_macros.hpp>

#include "../String/ST_String.hpp"

TEST_CASE("ST_String: default is empty/null", "[String][st_string]")
{
    const ST_String s;
    REQUIRE(s.c_str() == nullptr);
    REQUIRE(s.isEmpty());
    REQUIRE(s.length() == 0);
    REQUIRE(s.view().empty());
}

TEST_CASE("ST_String: constructs from C string", "[String][st_string]")
{
    const ST_String s("hello");
    REQUIRE_FALSE(s.isEmpty());
    REQUIRE(s.length() == 5);
    REQUIRE(s.view() == "hello");
    REQUIRE(std::strcmp(s.c_str(), "hello") == 0);
}


TEST_CASE("ST_String: constructs from string_view", "[String][st_string]")
{
    std::string_view sv = "hello";
    ST_String s(sv);
    REQUIRE(s.view() == sv);
    REQUIRE(s.length() == sv.size());
}


TEST_CASE("ST_String: copy of default-constructed is null-safe", "[String][st_string]") {
    ST_String a;
    ST_String b = a;
    CHECK(b.isEmpty());
    CHECK(b.view().empty());
    CHECK(b.c_str() == nullptr);
}

TEST_CASE("ST_String: const char* string constructor is null-safe", "[String][st_string]") {
    const char* a = nullptr;
    ST_String b(a);
    CHECK(b.isEmpty());
    CHECK(b.view().empty());
    CHECK(b.c_str() == nullptr);
}

TEST_CASE("ST_String: const char* string constructor with empty string", "[String][st_string]") {
    ST_String s("");
    CHECK(s.isEmpty());
    CHECK(s.view().empty());
    CHECK(s.length() == 0);
}

TEST_CASE("ST_String: const char* string set", "[String][st_string]") {
    const char* a = "hello";
    ST_String b("");
    b.set(a);
    CHECK(b.view() == "hello");
}

TEST_CASE("ST_String: copy constructor does deep copy", "[String][st_string]")
{
    ST_String a("abc");
    ST_String b(a);

    REQUIRE(a.view() == "abc");
    REQUIRE(b.view() == "abc");

    // deep copy: pointers must differ
    REQUIRE(a.c_str() != nullptr);
    REQUIRE(b.c_str() != nullptr);
    REQUIRE(a.c_str() != b.c_str());

    // modifying a should not affect b
    a.modifyInPlace([](char* p) {
        p[2] = 'N';
        return 0;
    });

    REQUIRE(a.view() == "abN");
    REQUIRE(b.view() == "abc");
}

TEST_CASE("ST_String: move constructor", "[String][st_string]")
{
    ST_String src("abc");
    const char* srcPtr = src.c_str();
    REQUIRE(srcPtr != nullptr);

    ST_String dest(std::move(src));

    REQUIRE(dest.view() == "abc");
    REQUIRE(dest.c_str() == srcPtr);

    // source should be empty/null
    REQUIRE(src.c_str() == nullptr);
    REQUIRE(src.isEmpty());
    REQUIRE(src.view().empty());
}

TEST_CASE("ST_String: set reuses buffer", "[String][st_string]")
{
    ST_String s("abcdefg");
    const char* p0 = s.c_str();
    REQUIRE(p0 != nullptr);

    s.set("abc", 3);

    REQUIRE(s.view() == "abc");
    // no reallocation if new_length <= old_length
    REQUIRE(s.c_str() == p0);
}