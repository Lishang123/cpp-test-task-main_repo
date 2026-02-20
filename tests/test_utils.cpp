#include <catch2/catch_test_macros.hpp>

#include "../repository/utils.hpp"

#include <filesystem>
#include <fstream>

using namespace utils;

TEST_CASE("toLower special-case ID", "[repository][utils]") {
    REQUIRE(toLower("ID") == "id");
}

TEST_CASE("toLower lowers leading acronym but keeps camel boundary", "[repository][utils]") {
    REQUIRE(toLower("URLValue") == "urlValue");
    REQUIRE(toLower("HTTPRequest") == "httpRequest");
}

TEST_CASE("toLower lowercases char", "[repository][utils]") {
    REQUIRE(toLower("A") == "a");
    REQUIRE(toLower("a") == "a");
}

TEST_CASE("toLower throws on empty string (current behavior)", "[repository][utils]") {
    REQUIRE_THROWS(toLower(""));
}

TEST_CASE("toLower throws on trailing uppercase with no i+1 (current behavior)", "[repository][utils]") {
    REQUIRE_THROWS(toLower("functionA"));
    REQUIRE_THROWS(toLower("ABC"));
}