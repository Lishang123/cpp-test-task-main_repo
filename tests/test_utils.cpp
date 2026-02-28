#include <catch2/catch_test_macros.hpp>

#include "../repository/utils.hpp"

#include <filesystem>
#include <fstream>

using namespace utils;
using namespace std::string_view_literals;

TEST_CASE("toLower special-case ID", "[repository][utils]") {
    REQUIRE(lowercaseUntilCamelBoundary("ID") == "id");
}

TEST_CASE("toLower lowers leading acronym but keeps camel boundary", "[repository][utils]") {
    REQUIRE(lowercaseUntilCamelBoundary("URLValue") == "urlValue");
    REQUIRE(lowercaseUntilCamelBoundary("HTTPRequest") == "httpRequest");
}

TEST_CASE("toLower lowercases char", "[repository][utils]") {
    REQUIRE(lowercaseUntilCamelBoundary("A") == "a");
    REQUIRE(lowercaseUntilCamelBoundary("a") == "a");
}

TEST_CASE("toLower throws on empty string", "[repository][utils]") {
    REQUIRE_THROWS(lowercaseUntilCamelBoundary(""));
}

TEST_CASE("toLower throws on upper-case string", "[repository][utils]") {
    REQUIRE(lowercaseUntilCamelBoundary("ABC") == "abc");
}

TEST_CASE("toLower throws on trailing uppercase with no i+1", "[repository][utils]") {
    REQUIRE(lowercaseUntilCamelBoundary("functioN") == "function");
}

TEST_CASE("listToString: list of multiple elements", "[repository][utils]") {
    REQUIRE(listToString(std::vector{"A"sv,"B"sv,"C"sv}) == "['A', 'B', 'C']");
}

TEST_CASE("listToString: list of single element", "[repository][utils]") {
    REQUIRE(listToString(std::vector{"A"sv}) == "['A']");
}