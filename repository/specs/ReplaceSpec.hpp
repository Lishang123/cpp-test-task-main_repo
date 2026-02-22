#ifndef CPP_TASK_REPLACESPEC_HPP
#define CPP_TASK_REPLACESPEC_HPP

#include "../generics/FUSReader.hpp"
#include "../FUSReplaceReader.hpp"

#include <span>

using namespace std::string_view_literals;
using namespace std::string_literals;

struct ReplaceSpec {

    enum class Element { document, functions, function, id, source, pattern, replacement, unknown };
    using Result = functions::repository::replace::Functions;

    struct State {
        std::optional<std::size_t> current{};
    };

    static constexpr Element documentRoot() { return Element::document; }

    static constexpr std::string_view elementName(const Element e) {
        switch (e) {
            case Element::functions: return "Functions"sv;
            case Element::function:  return "Function"sv;
            case Element::id:        return "ID"sv;
            case Element::source:    return "Source"sv;
            case Element::pattern:    return "Pattern"sv;
            case Element::replacement: return "Replacement"sv;
            default:                 return "?"sv;
        }
    }

    static Element toElement(const std::string_view name) {
        const auto n = utils::lowercaseUntilCamelBoundary(name);
        if (n == "functions"s) return Element::functions;
        if (n == "function"s)  return Element::function;
        if (n == "id"s)        return Element::id;
        if (n == "source"s)    return Element::source;
        if (n == "pattern"s)   return Element::pattern;
        if (n == "replacement"s)  return Element::replacement;
        return Element::unknown;
    }

    static bool isKnown(const Element e) { return e != Element::unknown; }

    static std::span<Element> allowedChildren(const Element p) {
        static Element doc[]  = { Element::functions };
        static Element funcs[] = { Element::function };
        static Element func[]  = { Element::id, Element::source, Element::pattern, Element::replacement };
        switch (p) {
            case Element::document:  return doc;
            case Element::functions: return funcs;
            case Element::function:  return func;
            case Element::id:
            case Element::source:
            case Element::pattern:
            case Element::replacement: return {};
            default:           return {};
        }
    }

    static void onText(const Element e, const char* text, Result& out, State& state, std::string_view repo) {
        using Function = functions::repository::replace::Function;

        if (e == Element::id) {
            // duplicate check
            if (auto it = std::ranges::find_if(out,
                [text](const Function& f){ return !std::strcmp(text, f.id.c_str()); });
                it != out.end())
            {
                utils::fatal("lm::duplicated_function_id",
                             fmt::format("Function with ID '{}' duplicated in {}.", text, repo));
            }
            out.emplace_back();
            state.current = out.size() - 1;
            out[*state.current].id.set(text);
            return;
        }
        if (e == Element::source || e == Element::pattern || e == Element::replacement) {
            if (!state.current) {
                utils::fatal("lm::other_function_child_before_id",
                         fmt::format("Found <{}> before <ID> while parsing {}.", elementName(e), repo));
            }
            if ( e == Element::source )
                out[*state.current].source.set(text);
            else if( e == Element::pattern )
                out[*state.current].pattern.set(text);
            else
                out[*state.current].replacement.set(text);
        }
    }
};
#endif //CPP_TASK_REPLACESPEC_HPP