#include <catch2/catch_test_macros.hpp>
#define private public
#include "../Misc/M_MemoryStream.hpp"
#include "../Misc/Memory.hpp"
#undef private

#define FRAGMENT_SIZE 2048

TEST_CASE("M_MemoryStreamFragment: Copy constructor allocates and copies", "[misc][streamFragment]")
{
    const std::string src = "Content";
    M_MemoryStreamFragment fragment(src.data(), src.size());

    const char* p = nullptr;
    const auto n = fragment.getContent(&p);

    REQUIRE(n == src.size());
    REQUIRE(p != nullptr);
    REQUIRE(std::string_view{p, static_cast<size_t>(n)} == src);

    // For Size < FRAGMENT_SIZE, copy constructor allocates FRAGMENT_SIZE*2
    if (src.size() < FRAGMENT_SIZE)
    {
        REQUIRE(fragment.getFreeSize() == (FRAGMENT_SIZE * 2u) - static_cast<T_uint64>(src.size()));
    }
}

TEST_CASE("M_MemoryStreamFragment: normal constructor does not copy", "[misc][streamFragment]")
{
    // create a non const char* so that the normal constructor is called
    char* non_const_chars = M::Memory::allocate<char>(7);
    std::memcpy(non_const_chars, "Content", 7);

    const M_MemoryStreamFragment fragment(non_const_chars, 7);

    const char* p = nullptr;
    const T_uint64 content_size = fragment.getContent(&p);

    REQUIRE(content_size == 7);
    REQUIRE(p == non_const_chars); // should still be the same pointer (no copy)
    REQUIRE(fragment.getFreeSize() == 0); // copy constructor would allocate more
    REQUIRE(std::string_view{p, content_size} == "Content");
}

TEST_CASE("M_MemoryStreamFragment: move constructor", "[misc][streamFragment]")
{

    // create a non const char* so that the normal constructor is called
    char* non_const_chars = M::Memory::allocate<char>(7);
    std::memcpy(non_const_chars, "Content", 7);

    M_MemoryStreamFragment fragment_a(non_const_chars, 7);
    M_MemoryStreamFragment fragment_b(std::move(fragment_a));

    // destination owns original pointer
    const char* ptr_b = nullptr;
    REQUIRE(fragment_b.getContent(&ptr_b) == 7);
    REQUIRE(ptr_b == non_const_chars); // should be the same pointer as the original

    // source should be empty after move
    const char* ptr_a = nullptr;
    REQUIRE(fragment_a.getContent(&ptr_a) == 0); // m_UsedSize should be reset
    REQUIRE(ptr_a == nullptr);
}

TEST_CASE("M_MemoryStreamFragment: move assignment operator", "[misc][streamFragment]")
{
    // First fragment owns buffer1
    char* buffer1 = M::Memory::allocate<char>(7);
    std::memcpy(buffer1, "Content", 7);
    M_MemoryStreamFragment fragment1(buffer1, 7);

    // Second fragment owns buffer2
    char* buffer2 = M::Memory::allocate<char>(10);
    std::memcpy(buffer2, "ContentNew", 10);
    M_MemoryStreamFragment fragment2(buffer2, 10);

    fragment1 = std::move(fragment2);

    // fragment1 should own buffer2, and fragment2 should be empty.
    const char* ptr1 = nullptr;
    REQUIRE(fragment1.getContent(&ptr1) == 10);
    REQUIRE(ptr1 == buffer2);
    REQUIRE(std::string_view{ptr1, 10} == "ContentNew");

    const char* ptr2 = nullptr;
    REQUIRE(fragment2.getContent(&ptr2) == 0); // the original code would fail here!
    REQUIRE(ptr2 == nullptr);
}

TEST_CASE("M_MemoryStreamFragment: append exact free size", "[misc][streamFragment]")
{
    constexpr T_uint64 first_size = FRAGMENT_SIZE - 1;
    const std::string str_1(first_size, 'A');

    M_MemoryStreamFragment fragment(str_1.data(), first_size);
    const T_uint64 free_size = fragment.getFreeSize();
    REQUIRE(free_size  == FRAGMENT_SIZE + 1);

    // append exactly free space
    std::string str_2(free_size, 'B');
    fragment.append(str_2.data(), free_size);

    const char* p = nullptr;
    const T_uint64 total = fragment.getContent(&p);

    REQUIRE(total == first_size + free_size);
    REQUIRE(fragment.getFreeSize() == 0);

    REQUIRE(std::string_view{p, static_cast<size_t>(first_size)} ==
            std::string_view{str_1});
    REQUIRE(std::string_view{p + first_size, static_cast<size_t>(free_size)} ==
            std::string_view{str_2});
}

/* ------ M_MemoryStream -------- */

TEST_CASE("M_MemoryStream::write(const char* Content, T_uint64 Size) reuses last unflushed fragment", "[misc][memoryStream]")
{

    M_MemoryStream stream;

    // First write: Size < FRAGMENT_SIZE => new fragment allocates FRAGMENT_SIZE*2
    constexpr T_uint64 first_size = FRAGMENT_SIZE - 1;
    std::string str_a(first_size, 'A');
    stream.write(str_a.data(), first_size);

    REQUIRE(stream.m_UnflushedContent.size() == 1);

    const T_uint64 free_size = stream.m_UnflushedContent.back().getFreeSize();
    REQUIRE(free_size == FRAGMENT_SIZE*2 - first_size);

    // Second write: try to exactly fill the free space in the first fragment
    std::string str_b(free_size, 'N');
    stream.write(str_b.data(), free_size);

    // In the original implementation, a new fragment is created even if the old fragment can store the new content
    REQUIRE(stream.m_UnflushedContent.size() == 1);

    // The result after flush should be the correct concatenation
    const char* res_ptr = nullptr;
    T_uint64 res_size = 0;
    stream.getContent(&res_ptr, &res_size); // call flush

    REQUIRE(res_size == first_size + free_size);
    REQUIRE(std::string_view{res_ptr, first_size} == std::string_view{str_a});
    REQUIRE(std::string_view{res_ptr + first_size, free_size} == std::string_view{str_b});
}


TEST_CASE("M_MemoryStream::write(long)","[misc][memoryStream]")
{
    M_MemoryStream stream;

    SECTION("zero")
    {
        stream.write(0L);

        const char* p = nullptr;
        T_uint64 n = 0;
        stream.getContent(&p, &n);

        REQUIRE(p != nullptr);
        REQUIRE(n == std::to_string(0L).size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == "0");
    }

    SECTION("positive")
    {
        constexpr long val = 42L;
        stream.write(val);

        const char* p = nullptr;
        T_uint64 n = 0;
        stream.getContent(&p, &n);

        const std::string expected = std::to_string(val);
        REQUIRE(n == expected.size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == expected);
    }

    SECTION("negative")
    {
        constexpr  long val = -42L;
        stream.write(val);

        const char* p = nullptr;
        T_uint64 n = 0;
        stream.getContent(&p, &n);

        const std::string expected = std::to_string(val);
        REQUIRE(n == expected.size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == expected);
    }

    SECTION("LONG_MAX")
    {
        stream.write(LONG_MAX);

        const char* p = nullptr;
        T_uint64 n = 0;
        stream.getContent(&p, &n);

        const std::string expected = std::to_string(LONG_MAX);
        REQUIRE(n == expected.size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == expected);
    }

    SECTION("LONG_MIN")
    {
        stream.write(LONG_MIN);

        const char* p = nullptr;
        T_uint64 n = 0;
        stream.getContent(&p, &n);

        const std::string expected = std::to_string(LONG_MIN);
        REQUIRE(n == expected.size());
        REQUIRE(std::string_view{p, static_cast<size_t>(n)} == expected);
    }

    SECTION("no embedded NULs are written")
    {
        constexpr long val = 1002003L;
        stream.write(val);

        const char* p = nullptr;
        T_uint64 n = 0;
        stream.getContent(&p, &n);

        for (size_t i = 0; i < static_cast<size_t>(n); ++i)
            REQUIRE(p[i] != '\0');
    }
}