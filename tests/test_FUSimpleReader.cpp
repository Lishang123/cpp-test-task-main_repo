#include <catch2/catch_test_macros.hpp>

#include "../repository/FUSimpleReader.hpp"
#include "../Types/TY_Blob.hpp"
#include "../Misc/SystemMessageError.hpp"
#include "../repository/specs/SimpleSpec.hpp"
#include "./XercesGuard.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "catch2/generators/catch_generators.hpp"

namespace fs = std::filesystem;

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

TEST_CASE("FUSimpleReader parses tabfiles/simple.tab", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );

    const fs::path simple_path = fs::path(PROJECT_SOURCE_DIR) / "tabfiles" / "simple.tab";
    // parse the XML to string.
    const std::string xml = slurp(simple_path);

    // TY_Blob stores pointer+size (it copies here)
    TY_Blob blob(xml.data(), xml.size());

    auto functions = reader(blob, "simple.tab");

    REQUIRE(functions.size() == 4);

    CHECK(std::string(functions[0].id.c_str())     == "stringLength...f2128203875h-1761480648.5_1");
    CHECK(std::string(functions[0].source.c_str()) == "0.1");

    CHECK(std::string(functions[1].id.c_str())     == "stringLength...f2128203875h-1761480648.6_1");
    CHECK(std::string(functions[1].source.c_str()) == "1.1");

    CHECK(std::string(functions[2].id.c_str())     == "stringLength...RIT_f2128203875h-1761480648.6_1");
    CHECK(std::string(functions[2].source.c_str()) == "1.1");

    CHECK(std::string(functions[3].id.c_str())     == "stringLength...f2128203875h-1761480648.7_1");
    CHECK(std::string(functions[3].source.c_str()) == "0.1");
}

TEST_CASE("FUSimpleReader parses simple doc", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <FunctioN>
                <ID>stringLength...f2128203875h-1761480648.5_1</ID>
                <Source>0.1</Source>
            </FunctioN>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto functions = reader(blob, "test-simple-doc");

    REQUIRE(functions.size() == 1);

    CHECK(std::string(functions[0].id.c_str())     == "stringLength...f2128203875h-1761480648.5_1");
    CHECK(std::string(functions[0].source.c_str()) == "0.1");
}


TEST_CASE("FUSimpleReader rejects duplicated function IDs", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function><ID>dup</ID><Source>a</Source></Function>
             <Function><ID>dup</ID><Source>b</Source></Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)reader(blob, "dup-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "lm::duplicated_function_id");
    }
}

TEST_CASE("FUSimpleReader rejects unknown elements", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <FunctionA><ID>stringLength...f2128203875h-1761480648.5_1</ID><Source>0.1</Source></FunctionA>
             <FunctionB><ID>stringLength...f2128203875h-1761480648.6_1</ID><Source>1.1</Source></FunctionB>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)reader(blob, "unknown-elem-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "lm::unexpected_element");
        CHECK(std::string(msg.getDescription()).starts_with("Found unexpected element 'FunctionA' while parsing"));
    }
}

TEST_CASE("FUSimpleReader rejects unexpected element while expecting Functions", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Function>
             <Function><ID>stringLength...f2128203875h-1761480648.5_1</ID><Source>0.1</Source></Function>
             <Function><ID>stringLength...f2128203875h-1761480648.6_1</ID><Source>1.1</Source></Function>
           </Function>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)reader(blob, "unexpected-elem-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "lm::unexpected_element");
        CHECK(std::string(msg.getDescription()).starts_with("Expected 'Functions' while parsing "));
    }
}

TEST_CASE("FUSimpleReader rejects unexpected elements while expecting Function", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Functions><ID>stringLength...f2128203875h-1761480648.5_1</ID><Source>0.1</Source></Functions>
             <Functions><ID>stringLength...f2128203875h-1761480648.6_1</ID><Source>1.1</Source></Functions>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)reader(blob, "unexpected-elem-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "lm::unexpected_element");
        CHECK(std::string(msg.getDescription()).starts_with("Expected 'Function' while parsing "));
    }
}

TEST_CASE("FUSimpleReader rejects unexpected elements while expecting ID/Source", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function>
                <Function>stringLength...f2128203875h-1761480648.5_1</Function>
                <Source>0.1</Source>
            </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    try {
        (void)reader(blob, "unexpected-elem-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "lm::unexpected_element");
        CHECK(std::string(msg.getDescription()).starts_with("Expected one of ['ID', 'Source']"));
    }
}

TEST_CASE("FUSimpleReader rejects unexpected child of ID/Source", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
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
        (void)reader(blob, "unexpected-child-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "lm::unexpected_child_element");
        CHECK(std::string(msg.getDescription()).starts_with("Found unexpected child element 'id'"));
    }
}


TEST_CASE("FUSimpleReader: <Source> before <ID>", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
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
        (void)reader(blob, "source-before-id-test");
        FAIL("Expected M_SystemMessage to be thrown");
    } catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "lm::source_before_id");
    }
}

TEST_CASE("FUSimpleReader parses simple doc without <source>", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function>
                <ID>stringLength...f2128203875h-1761480648.5_1</ID>
            </Function>
            <Function>
                <ID>stringLength...f2128203875h-1761480648.6_1</ID>
            </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto functions = reader(blob, "test-simple-doc-no-source");

    REQUIRE(functions.size() == 2);

    CHECK(std::string(functions[0].id.c_str()) == "stringLength...f2128203875h-1761480648.5_1");
    CHECK(std::string(functions[1].id.c_str()) == "stringLength...f2128203875h-1761480648.6_1");
}

TEST_CASE("FUSimpleReader parses multiple IDs inside a Function", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function>
                <ID>5_1</ID>
                <ID>6_1</ID>
            </Function>
            <Function>
                <ID>6_2</ID>
            </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto functions = reader(blob, "test-simple-doc-no-source");

    REQUIRE(functions.size() == 3);

    CHECK(std::string(functions[0].id.c_str()) == "5_1");
    CHECK(std::string(functions[1].id.c_str()) == "6_1");
    CHECK(std::string(functions[2].id.c_str()) == "6_2");
}

TEST_CASE("FUSimpleReader parses empty IDs", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function>
                <ID/>
            </Function>
            <Function>
                <ID>1</ID>
            </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));
    auto functions = reader(blob, "test-empty-ids");
    REQUIRE(functions.size() == 2);

    CHECK(std::string(functions[0].id.c_str()).empty());
    CHECK(std::string(functions[1].id.c_str()) == "1");
}

TEST_CASE("FUSimpleReader parses simple doc without function child", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function/>
             <Function/>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto functions = reader(blob, "test-no-function-child");

    REQUIRE(functions.empty());
}

TEST_CASE("FUSimpleReader preserves whitespaces", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function>
                <ID>
                     dup
                </ID>
            </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto functions = reader(blob, "test-preserve-whitespaces");
    // The current implementation doesn't trim whitespaces for text content.
    // Most parsers do trim the whitespaces. If this is the intended behavior is unknown.
    REQUIRE(functions.size() == 1);
    CHECK_FALSE(std::string(functions[0].id.c_str()) == "dup");
}

TEST_CASE("FUSimpleReader validates closing tags", "[repository][simple]") {
    ensure_xerces();

    auto reader = GENERATE(
        &functions::repository::simple::readRepo_legacy,
        &functions::repository::simple::readRepo
    );
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8"?>
           <Functions>
             <Function>
                <ID>dup
             </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));
    try {
        auto functions = reader(blob, "test-validates-closing-tags");
        FAIL("Expected M_SystemMessage to be thrown");
    }
    catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "XMLLM_XR_REPOSITORYPARSER_NATIVE_FATAL_PARSE_ERROR");
        CHECK(std::string(msg.getDescription()).find("expected end of tag 'ID'") != std::string::npos);
    }
}