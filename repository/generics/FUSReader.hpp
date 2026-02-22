#ifndef CPP_TASK_FUSREADER_HPP
#define CPP_TASK_FUSREADER_HPP
#include "../utils.hpp"
#include "../../XML/XML_Parser.hpp"

#include <ranges>

template <class Spec>
class FUSReader : public XML_Parser
{
public:
    using Element = Spec::Element; // readers can have different element sets
    using Result  = Spec::Result; // readers can have different results
    using State   = Spec::State; // extensible, current index solution is buggy for sax parser

    static_assert(std::is_enum_v<Element>, "Spec::Element should be an enum type.");

    Result read(const TY_Blob& data, std::string_view repo) && {
        m_repository.set(repo);
        m_elementStack.push_back(Spec::documentRoot()); // e.g. Element::document

        parseBlob(&data);
        return std::move(m_functions);
    }

private:
    Result m_functions{};
    State m_state{};
    std::vector<Element> m_elementStack{};
    ST_String m_repository{};

    void errorMessage(const M_SystemMessage& message) override { throw message; }

    bool startElementChar(const char*, const char* localName, const char*, const xercesc::Attributes&) override {
        const Element parent = m_elementStack.back();
        const Element current_elem = Spec::toElement(localName);
        m_elementStack.push_back(current_elem);

        if (!Spec::isKnown(current_elem)) {
            utils::unexpectedElement(localName, m_repository.view());
        }
        // validate parent
        auto allowedChildren = Spec::allowedChildren(parent);

        if (allowedChildren.empty()) {
            utils::unexpectedChild(localName, m_repository.view());
        }
        if (std::ranges::find(allowedChildren, current_elem) == allowedChildren.end()) {
            std::vector<std::string_view> allowedChildrenView;
            allowedChildrenView.resize(allowedChildren.size());
            std::ranges::transform(allowedChildren, allowedChildrenView.begin(), Spec::elementName);
            utils::unexpectedElement(localName,
               allowedChildrenView, m_repository.view());
        }
        return true;
    }

    bool charactersChar(const char*, const char*, const char*, const char* text, const unsigned int) override {
        const Element cur = m_elementStack.back();
        Spec::onText(cur, text, m_functions, m_state, m_repository.view());
        return true;
    }

    bool endElementChar(const char*, const char*, const char*) override {
        const Element cur = m_elementStack.back();
        m_elementStack.pop_back();
        return true;
    }
};

#endif