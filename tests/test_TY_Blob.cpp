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

TEST_CASE("TY_Blob: copyContent", "[Types][TY_Blob]")
{
    const TY_Blob blob("abcdef");

    std::array<char, 10> out{};
    out.fill('N');

    // request more than available from offset 4 -> only 2 bytes ("ef")
    const auto copied = blob.copyContent(out.data(),  4,  100);
    REQUIRE(copied == 2);
    REQUIRE(std::string_view(out.data(), copied) == "ef"sv);
}

TEST_CASE("TY_Blob: copyContent unsigned underflow", "[Types][TY_Blob]")
{
    const TY_Blob blob("abcd");
    REQUIRE(blob.getSize() == 4);

    std::array<char, 8> out{};
    out.fill('A'); // sentinel pattern

    // offset beyond end -> 0 (where legacy code underflows!)
    REQUIRE(blob.copyContent(out.data(), 999, 2) == 0);
    for (const char c : out) REQUIRE(c == 'A');
}

