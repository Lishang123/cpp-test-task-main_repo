#include <catch2/catch_test_macros.hpp>

#include <string_view>
#include <cstring>
#include <array>

#include "../Types/TY_Blob.hpp"

using namespace std::literals;

static std::string_view blob_view(const TY_Blob& b)
{
    const char* p = nullptr;
    T_uint64 n = 0;
    b.getContent(&p, &n);
    return std::string_view{p, static_cast<size_t>(n)};
}

TEST_CASE("TY_Blob: move constructor", "[Types][TY_Blob]")
{
    TY_Blob src("abc");
    REQUIRE(src.getSize() == 3);
    REQUIRE(blob_view(src) == "abc"sv);

    TY_Blob dest(std::move(src));

    REQUIRE(dest.getSize() == 3);
    REQUIRE(blob_view(dest) == "abc"sv);

    REQUIRE(src.getSize() == 0);
    REQUIRE(src.getContent() == nullptr);
}

TEST_CASE("TY_Blob: move assignment", "[Types][TY_Blob]")
{
    TY_Blob src("abc");
    TY_Blob dest("def");

    REQUIRE(blob_view(src) == "abc"sv);
    REQUIRE(blob_view(dest) == "def"sv);

    dest = std::move(src);

    REQUIRE(blob_view(dest) == "abc"sv);

    REQUIRE(src.getSize() == 0);
    REQUIRE(src.getContent() == nullptr);
}
