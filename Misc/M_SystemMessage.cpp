#include "M_SystemMessage.hpp"

M_SystemMessage::M_SystemMessage(std::string_view domain, std::string_view code, std::string_view description)
    : m_Domain(domain), m_Code(code), m_Description(description)
{

}

const char* M_SystemMessage::getCode() const
{
    return m_Code.c_str();
}

const char* M_SystemMessage::getDomain() const
{
    return m_Domain.c_str();
}

const char* M_SystemMessage::getDescription() const
{
    return m_Description.c_str();
}