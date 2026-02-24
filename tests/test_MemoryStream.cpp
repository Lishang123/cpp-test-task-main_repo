#include <catch2/catch_test_macros.hpp>
#define private public
#include "../Misc/M_MemoryStream.hpp"
#undef private

TEST_CASE("M_MemoryStream::write(const char* Content, T_uint64 Size) reuses last unflushed fragment", "[misc][memoryStream]")
{
    constexpr unsigned FRAGMENT_SIZE = 2048;

    M_MemoryStream s;

    // First write: Size < FRAGMENT_SIZE => new fragment allocates FRAGMENT_SIZE*2
    constexpr T_uint64 firstSize = FRAGMENT_SIZE - 1;
    std::string a(firstSize, 'A');
    s.write(a.data(), firstSize);

    REQUIRE(s.m_UnflushedContent.size() == 1);

    const T_uint64 freeSize = s.m_UnflushedContent.back().getFreeSize();
    REQUIRE(freeSize == FRAGMENT_SIZE*2 - firstSize);

    // Second write: try to exactly fill the free space in the first fragment
    std::string b(freeSize, 'N');
    s.write(b.data(), freeSize);

    // In the original implementation, a new fragment is created even if the old fragment can store the new content
    REQUIRE(s.m_UnflushedContent.size() == 1);

    // The result after flush should be the correct concatenation
    const char* p = nullptr;
    T_uint64 n = 0;
    s.getContent(&p, &n); // call flush

    REQUIRE(n == firstSize + freeSize);
    REQUIRE(std::string_view{p, firstSize} == std::string_view{a});
    REQUIRE(std::string_view{p + firstSize, freeSize} == std::string_view{b});
}


TEST_CASE("M_MemoryStream::write(long)","[misc][memoryStream]")
{
    M_MemoryStream s;

    SECTION("zero")
    {
        s.write(0L);

        const char* p = nullptr;
        T_uint64 n = 0;
        s.getContent(&p, &n);

        REQUIRE(p != nullptr);
        REQUIRE(n == std::to_string(0L).size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == "0");
    }

    SECTION("positive")
    {
        const long v = 42L;
        s.write(v);

        const char* p = nullptr;
        T_uint64 n = 0;
        s.getContent(&p, &n);

        const std::string expected = std::to_string(v);
        REQUIRE(n == expected.size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == expected);
    }

    SECTION("negative")
    {
        const long v = -42L;
        s.write(v);

        const char* p = nullptr;
        T_uint64 n = 0;
        s.getContent(&p, &n);

        const std::string expected = std::to_string(v);
        REQUIRE(n == expected.size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == expected);
    }

    SECTION("LONG_MAX")
    {
        s.write(LONG_MAX);

        const char* p = nullptr;
        T_uint64 n = 0;
        s.getContent(&p, &n);

        const std::string expected = std::to_string(LONG_MAX);
        REQUIRE(n == expected.size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == expected);
    }

    SECTION("LONG_MIN")
    {
        s.write(LONG_MIN);

        const char* p = nullptr;
        T_uint64 n = 0;
        s.getContent(&p, &n);

        const std::string expected = std::to_string(LONG_MIN);
        REQUIRE(n == expected.size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == expected);
    }

    SECTION("no embedded NULs are written")
    {
        const long v = 1002003L;
        s.write(v);

        const char* p = nullptr;
        T_uint64 n = 0;
        s.getContent(&p, &n);

        for (size_t i = 0; i < static_cast<size_t>(n); ++i)
            REQUIRE(p[i] != '\0');
    }
}