#ifndef CPP_TASK_SYSTEMERRORMESSAGE_HPP
#define CPP_TASK_SYSTEMERRORMESSAGE_HPP
#include "M_SystemMessage.hpp"

class SystemMessageError final : public std::runtime_error {
public:
    explicit SystemMessageError(M_SystemMessage msg)
        : std::runtime_error(toString(msg))
        , msg_(std::move(msg)) {}

    [[nodiscard]] const M_SystemMessage& message() const noexcept { return msg_; }

private:
    M_SystemMessage msg_;

    static std::string toString(const M_SystemMessage& m) {
        std::string s;
        s += m.getDomain();
        s += ":";
        s += m.getCode();
        s += ": ";
        s += m.getDescription();
        return s;
    }
};

#endif //CPP_TASK_SYSTEMERRORMESSAGE_HPP