#include <catch2/catch_test_macros.hpp>

#include "../String/ST_String.hpp"


TEST_CASE("ST_String: copy of default-constructed is null-safe", "[String][st_string]") {
    ST_String a;
    ST_String b = a;
    CHECK(b.isEmpty());
    CHECK(b.c_str() == nullptr);
}

TEST_CASE("ST_String: const char* string constructor is null-safe", "[String][st_string]") {
    const char* a = nullptr;
    ST_String b(a);
    CHECK(b.isEmpty());
    CHECK(b.c_str() == nullptr);
}