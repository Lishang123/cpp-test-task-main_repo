#include <catch2/catch_test_macros.hpp>

#include "../repository/FUSReplaceReader.hpp"
#include "../Types/TY_Blob.hpp"
#include "../Misc/M_SystemMessage.hpp"

#include <xercesc/util/PlatformUtils.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
namespace rpl = functions::repository::replace;

// Initialize Xerces
struct XercesGuard {
    XercesGuard() { xercesc::XMLPlatformUtils::Initialize(); }
    ~XercesGuard() { xercesc::XMLPlatformUtils::Terminate(); }
};

/**
 * Ensure xerces is initialized for the tests.
 */
static void ensure_xerces() {
    static XercesGuard guard{};
}

/**
 * Read the file into a string (for TY_Blob).
 * @param p file path
 * @return the string read.
 */
static std::string slurp(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    REQUIRE(in.good());

    return std::string{
        std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>()
    };
}

TEST_CASE("FUSReplaceReader parses tabfiles/replace.tab", "[repository][replace]") {
    ensure_xerces();

    const fs::path simple_path = fs::path(PROJECT_SOURCE_DIR) / "tabfiles" / "replace.tab";
    // parse the XML to string.
    const std::string xml = slurp(simple_path);

    // TY_Blob stores pointer+size (it copies here)
    TY_Blob blob(xml.data(), xml.size());

    auto functions = rpl::readRepo(blob, "replace.tab");

    REQUIRE(functions.size() == 2);

    CHECK(std::string(functions[0].id.c_str())     == "replace...f_653960456h1121717696.7_1");
    CHECK(std::string(functions[0].source.c_str()) == "0");

    CHECK(std::string(functions[1].id.c_str())     == "replace...f_653960456h771489301.7_1");
    CHECK(std::string(functions[1].source.c_str()) == "0");
}

TEST_CASE("FUSReplaceReader parses replace doc", "[repository][replace]") {
    ensure_xerces();

    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <FunctioN>
                <ID>stringLength...f2128203875h-1761480648.5_1</ID>
                <Source>0.1</Source>
            </FunctioN>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto functions = rpl::readRepo(blob, "test-simple-doc");

    REQUIRE(functions.size() == 1);

    CHECK(std::string(functions[0].id.c_str())     == "stringLength...f2128203875h-1761480648.5_1");
    CHECK(std::string(functions[0].source.c_str()) == "0.1");
}


TEST_CASE("FUSReplaceReader rejects duplicated function IDs", "[repository][replace]") {
    ensure_xerces();

    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function><ID>dup</ID><Source>a</Source></Function>
             <Function><ID>dup</ID><Source>b</Source></Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)rpl::readRepo(blob, "dup-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const M_SystemMessage& msg) {
        CHECK(std::string(msg.getCode()) == "lm::duplicated_function_id");
    }
}

TEST_CASE("FUSReplaceReader rejects unknown elements", "[repository][replace]") {
    ensure_xerces();

    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <FunctionA><ID>stringLength...f2128203875h-1761480648.5_1</ID><Source>0.1</Source></FunctionA>
             <FunctionB><ID>stringLength...f2128203875h-1761480648.6_1</ID><Source>1.1</Source></FunctionB>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)rpl::readRepo(blob, "unknown-elem-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const M_SystemMessage& msg) {
        CHECK(std::string(msg.getCode()) == "lm::unexpected_element");
        CHECK(std::string(msg.getDescription()).starts_with("Found unexpected element 'FunctionA' while parsing"));
    }
}

TEST_CASE("FUSReplaceReader rejects unexpected element while expecting Functions", "[repository][replace]") {
    ensure_xerces();

    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Function>
             <Function><ID>stringLength...f2128203875h-1761480648.5_1</ID><Source>0.1</Source></Function>
             <Function><ID>stringLength...f2128203875h-1761480648.6_1</ID><Source>1.1</Source></Function>
           </Function>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)rpl::readRepo(blob, "unexpected-elem-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const M_SystemMessage& msg) {
        std::cout << msg.getDescription() << std::endl;
        CHECK(std::string(msg.getCode()) == "lm::unexpected_element");
        CHECK(std::string(msg.getDescription()).starts_with("Expected 'Functions' while parsing "));
    }
}

TEST_CASE("FUSReplaceReader rejects unexpected elements while expecting Function", "[repository][replace]") {
    ensure_xerces();

    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Functions><ID>stringLength...f2128203875h-1761480648.5_1</ID><Source>0.1</Source></Functions>
             <Functions><ID>stringLength...f2128203875h-1761480648.6_1</ID><Source>1.1</Source></Functions>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)rpl::readRepo(blob, "unexpected-elem-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const M_SystemMessage& msg) {
        std::cout << msg.getDescription() << std::endl;
        CHECK(std::string(msg.getCode()) == "lm::unexpected_element");
        CHECK(std::string(msg.getDescription()).starts_with("Expected 'Function' while parsing "));
    }
}


TEST_CASE("FUSReplaceReader rejects unexpected child of ID/Source", "[repository][replace]") {
    ensure_xerces();

    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function>
                <id><id>stringLength...f2128203875h-1761480648.5_1</id></id>
                <source>0.1</source>
            </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)rpl::readRepo(blob, "unexpected-child-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const M_SystemMessage& msg) {
        CHECK(std::string(msg.getCode()) == "lm::unexpected_child_element");
        CHECK(std::string(msg.getDescription()).starts_with("Found unexpected child element 'id'"));
    }
}

TEST_CASE("FUSReplaceReader: <Source> before <ID>", "[repository][replace]") {
    ensure_xerces();

    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function>
                <source>0.1</source>
                <id>stringLength...f2128203875h-1761480648.5_1</id>
            </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)rpl::readRepo(blob, "source-before-id-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const M_SystemMessage& msg) {
        CHECK(std::string(msg.getCode()) == "lm::other_function_child_before_id");
        CHECK(std::string(msg.getDescription()).starts_with("Found <Source> before <ID> while parsing"));
    }
}


