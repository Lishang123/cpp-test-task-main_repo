#include "../XML/XML_xerces_String.hpp"
#include "XercesGuard.hpp"
#include <catch2/catch_test_macros.hpp>
#include <string_view>


TEST_CASE("XML_xerces_String: Default constructor with empty string", "[xml][xml-xerces-string]")
{
    ensure_xerces();

    XML_xerces_String s;

    REQUIRE(s.getLocalForm().empty());
    REQUIRE(s.getXMLForm() == nullptr);
}

TEST_CASE("XML_xerces_String: Construct from local form", "[xml][xml-xerces-string]")
{
    ensure_xerces();

    XML_xerces_String s("Hello");

    REQUIRE(s.getLocalForm() == "Hello");
    REQUIRE(s.getXMLForm() != nullptr);
}

TEST_CASE("XML_xerces_String: Construct from XML form", "[xml][xml-xerces-string]")
{
    ensure_xerces();

    // Use convertToXMLForm from your class
    XMLCh* xml_chars = XML_xerces_String::convertToXMLForm("Hello");

    XML_xerces_String s(xml_chars);
    // xml_chars is already replicated by the constructor, we can release it.
    xercesc::XMLString::release(&xml_chars);

    REQUIRE(s.getXMLForm() != nullptr);
    REQUIRE(s.getLocalForm() == "Hello");

}

TEST_CASE("XML_xerces_String: Move constructor transfers ownership", "[xml][xml-xerces-string]")
{
    ensure_xerces();

    XML_xerces_String a("Hello");

    XML_xerces_String b(std::move(a));

    REQUIRE(b.getLocalForm() == "Hello");

    // moved-from must be safe and empty
    REQUIRE(a.getLocalForm().empty());
    REQUIRE(a.getXMLForm() == nullptr);
}

TEST_CASE("XML_xerces_String: Move assignment transfers ownership", "[xml][xml-xerces-string]")
{
    ensure_xerces();

    XML_xerces_String a("first");
    XML_xerces_String b("second");

    b = std::move(a);

    REQUIRE(b.getLocalForm() == "first");

    REQUIRE(a.getLocalForm().empty());
    REQUIRE(a.getXMLForm() == nullptr);
}

TEST_CASE("XML_xerces_String: compare")
{
    ensure_xerces();

    XML_xerces_String s("abc");

    REQUIRE(s.compare(static_cast<const char*>(nullptr)) == 1);
    REQUIRE(s.compare(static_cast<const XMLCh*>(nullptr)) == 1);
    REQUIRE(s.compare("aBc") != 0);

    XML_xerces_String empty;

    REQUIRE(empty.compare("abc") == -1);
    REQUIRE(empty.compare(static_cast<const char*>(nullptr)) == 0);
}

TEST_CASE("XML_xerces_String: compareNoCase", "[xml][xml-xerces-string]")
{
    ensure_xerces();

    XML_xerces_String s("AbC");

    XML_xerces_String empty;
    REQUIRE(empty.compareNoCase("abc") == -1);

    REQUIRE(s.compareNoCase(static_cast<const char*>(nullptr)) == 1);
    REQUIRE(s.compareNoCase("aBc") == 0);
}