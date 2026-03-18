#include "XercesGuard.hpp"
#include <xercesc/util/PlatformUtils.hpp>

// Initialize Xerces
struct XercesGuard {
    XercesGuard() { xercesc::XMLPlatformUtils::Initialize(); }
    ~XercesGuard() { xercesc::XMLPlatformUtils::Terminate(); }
};

void ensure_xerces() {
    static XercesGuard guard{};
}