#include <catch2/catch_test_macros.hpp>

#include "../Misc/Memory.hpp"

using namespace M::Memory;

TEST_CASE("Memory: allocate(0)", "[Misc][Memory]")
{
    void* p = allocate(0);
    REQUIRE(p != nullptr);
    release(p);
}

TEST_CASE("Memory: calloc zero-initializes memory", "[Misc][Memory]")
{
    constexpr size_t size  = 4;
    constexpr size_t count = 8;

    void* p = callocate(size, count);
    REQUIRE(p != nullptr);

    // test zero-initialized memory guaranteed by calloc
    // Any object’s memory can be safely examined as unsigned char
    const auto* bytes = static_cast<const unsigned char*>(p);
    for (size_t i = 0; i < size * count; ++i)
        REQUIRE(bytes[i] == 0);

    release(p);
}

TEST_CASE("Memory: reAllocate preserves prefix when growing", "[Misc][Memory]")
{
    constexpr size_t oldSize = 8;
    constexpr size_t newSize = 32;

    void* old_buffer = allocate(oldSize);
    REQUIRE(old_buffer != nullptr);

    std::memset(old_buffer, 0xAB, oldSize);

    void* grown_buffer = reAllocate(old_buffer, newSize);
    REQUIRE(grown_buffer != nullptr);

    const auto* bytes = static_cast<const unsigned char*>(grown_buffer);
    for (size_t i = 0; i < oldSize; ++i)
        REQUIRE(bytes[i] == 0xAB);

    release(grown_buffer);
}

TEST_CASE("Memory: outOfMemoryHandler throws std::bad_alloc", "[Misc][Memory]")
{
    REQUIRE_THROWS_AS( M::Memory::outOfMemoryHandler(), std::bad_alloc);
}

TEST_CASE("Memory: release(nullptr) is safe", "[Misc][Memory]")
{
    release(nullptr);
    SUCCEED();
}

