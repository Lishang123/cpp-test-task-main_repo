#ifndef CPP_TASK_M_SYSTEMMESSAGE_HPP
#define CPP_TASK_M_SYSTEMMESSAGE_HPP

//Simplified version of M_SystemMessage

#include "../String/ST_String.hpp"

class M_SystemMessage
{
private:
    ST_String m_Domain;
    ST_String m_Code;
    ST_String m_Description;

public:
    explicit M_SystemMessage( std::string_view domain, std::string_view code, std::string_view description );
    const char* getDomain() const;
    const char* getCode() const;
    const char* getDescription() const;
};

#endif //CPP_TASK_M_SYSTEMMESSAGE_HPP
