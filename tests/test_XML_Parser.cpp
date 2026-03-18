#include <catch2/catch_test_macros.hpp>

#include "../XML/XML_Parser.hpp"
#include "../Types/TY_Blob.hpp"
#include "../Misc/SystemMessageError.hpp"
#include "./XercesGuard.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "catch2/catch_template_test_macros.hpp"

namespace fs = std::filesystem;



TEST_CASE("XML_Parser ignores (wrong) XML declaration", "[xml][parser]") {
    ensure_xerces();

    /*
     * in this test, encoding="UTF-8?" is wrong because of the extra '?'.
     * Since the old condition in 'm_RemoveXMLDeclaration' for advancing the buffer is wrong,
     * this header cannot be removed completely and will lead to error.
     * After fixing the problem, this header can be successfully ignored.
     */
    const char* xml =
        R"(<?xml version="1.0" encoding="UTF-8?"?>
           <Functions>
             <Function>
                <ID>stringLength...f2128203875h-1761480648.5_1</ID>
                <Source>0.1</Source>
            </Function>
           </Functions>)";

    TY_Blob blob(xml, std::strlen(xml));
    XML_Parser parser_ignore_header(false, false, true);
    // No error after ignoring the wrong header
    REQUIRE(parser_ignore_header.parseBlob(&blob));
    // Throw when not ignoring the wrong header
    try {
        XML_Parser parser_default;
        parser_default.parseBlob(&blob);
        FAIL("Expected M_SystemMessage to be thrown");
    }
    catch (const SystemMessageError& error) {
        const auto& msg = error.message();
        CHECK(std::string(msg.getCode()) == "XMLLM_XR_REPOSITORYPARSER_NATIVE_FATAL_PARSE_ERROR");
        CHECK(std::string(msg.getDescription()).find("invalid XML encoding declaration 'UTF-8?'") != std::string::npos);
    }
}

template <typename Parser>
static Parser makeParser()
{
    return Parser{};
}

class AttrTestParserLegacy final : public XML_Parser {
public:
    long attrValueLong = 0;
    bool attrValueBool = false;
    bool foundTag = false;
    bool attrLongExists = false;
    bool attrBoolExists = false;
    ST_String attrValue;

    bool startElementChar(const char* /*uri*/,
                          const char* /*localName*/,
                          const char* qName,
                          const xercesc::Attributes& attrs) override
    {
        if (std::string_view{qName} == "Functions") {
            foundTag = true;
            attrValueLong = getAttributeLong(attrs, "count", attrLongExists);
            attrValueBool = getAttributeBool(attrs, "optional", attrBoolExists);
            attrValue = getAttributeValue(attrs, "note");
        }
        return true;
    }
};

class AttrTestParserRefactored final : public XML_Parser {
public:
    long attrValueLong = 0;
    bool attrValueBool = false;
    bool foundTag = false;
    bool attrLongExists = false;
    bool attrBoolExists = false;

    bool startElementChar(const char* /*uri*/,
                          const char* /*localName*/,
                          const char* qName,
                          const xercesc::Attributes& attrs) override
    {
        if (std::string_view{qName} == "Functions") {
            foundTag = true;
            auto resLong = getAttributeLongOptional(attrs, "count");
            auto resBool = getAttributeBool(attrs, "optional");

            if (resLong.has_value()) {
                attrLongExists = true;
                attrValueLong = resLong.value();
            }

            if (resBool.has_value()) {
                attrBoolExists = true;
                attrValueBool = resBool.value();
            }
        }
        return true;
    }
};

TEMPLATE_TEST_CASE("XML_Parser accepts valid attribute value", "[xml][parser][attr][long]", AttrTestParserLegacy, AttrTestParserRefactored) {
    ensure_xerces();

    const char* xml =
        R"(<Functions count="1000"></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = makeParser<TestType>();
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE(parser.attrLongExists);
    REQUIRE(parser.attrValueLong == 1000);
}

TEMPLATE_TEST_CASE("XML_Parser ignores no attribute", "[xml][parser][attr][long]", AttrTestParserLegacy, AttrTestParserRefactored) {
    ensure_xerces();

    const char* xml =
        R"(<Functions></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = makeParser<TestType>();
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE_FALSE(parser.attrLongExists);
    REQUIRE_FALSE(parser.attrValueLong);
}

TEMPLATE_TEST_CASE("XML_Parser rejects invalid attribute value", "[xml][parser][attr][long]", AttrTestParserLegacy, AttrTestParserRefactored) {
    ensure_xerces();

    const char* xml =
        R"(<Functions count="abc"></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = makeParser<TestType>();
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE_FALSE(parser.attrLongExists); // fail for original implementation
    REQUIRE_FALSE(parser.attrValueLong);
}

TEMPLATE_TEST_CASE("XML_Parser ignores empty attribute value", "[xml][parser][attr][long]", AttrTestParserLegacy, AttrTestParserRefactored) {
    ensure_xerces();

    const char* xml =
        R"(<Functions count=""></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = makeParser<TestType>();
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE_FALSE(parser.attrLongExists); // fail for original implementation
    REQUIRE_FALSE(parser.attrValueLong);
}

TEMPLATE_TEST_CASE("XML_Parser parses boolean attribute", "[xml][parser][attr][bool]", AttrTestParserLegacy, AttrTestParserRefactored) {
    ensure_xerces();

    const char* xml =
        R"(<Functions optional="true"></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = makeParser<TestType>();
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE(parser.attrBoolExists);
    REQUIRE(parser.attrValueBool);
}

TEMPLATE_TEST_CASE("XML_Parser parses empty (boolean) attribute", "[xml][parser][attr][bool]", AttrTestParserLegacy, AttrTestParserRefactored) {
    ensure_xerces();

    const char* xml =
        R"(<Functions optional=""></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = makeParser<TestType>();
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE_FALSE(parser.attrBoolExists);
    REQUIRE_FALSE(parser.attrValueBool);
}

TEMPLATE_TEST_CASE("XML_Parser parses invalid boolean attribute", "[xml][parser][attr][bool]", AttrTestParserLegacy, AttrTestParserRefactored) {
    ensure_xerces();

    const char* xml =
        R"(<Functions optional="123"></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = makeParser<TestType>();
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE_FALSE(parser.attrBoolExists);
    REQUIRE_FALSE(parser.attrValueBool);
}

TEST_CASE("XML_Parser parses text attribute", "[xml][parser][attr][value]") {
    ensure_xerces();

    const char* xml =
        R"(<Functions note="description"></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = AttrTestParserLegacy{};
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE(parser.attrValue == "description");
}

TEST_CASE("XML_Parser parses empty text attribute", "[xml][parser][attr][value]") {
    ensure_xerces();

    const char* xml =
        R"(<Functions note=""></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = AttrTestParserLegacy{};
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE(parser.attrValue == "");
}

TEST_CASE("XML_Parser parses non-existing attribute", "[xml][parser][attr][value]") {
    ensure_xerces();

    const char* xml =
        R"(<Functions></Functions>)";

    TY_Blob blob(xml, std::strlen(xml));

    auto parser = AttrTestParserLegacy{};
    REQUIRE(parser.parseBlob(&blob));
    REQUIRE(parser.attrValue == "");
}

