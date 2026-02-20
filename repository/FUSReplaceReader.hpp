#ifndef E2ERUNTIME_FUSREPLACEREADER_HPP
#define E2ERUNTIME_FUSREPLACEREADER_HPP

#include "../String/ST_String.hpp"

class TY_Blob;


namespace functions::repository::replace
{
    struct Function
    {
        ST_String id;
        ST_String source;
        ST_String pattern;
        ST_String replacement;
    };

    using Functions = std::vector<Function>;


    Functions  readRepo( const TY_Blob &data, std::string_view repo );

}//namespace functions::repository::fus::replace

#endif //E2ERUNTIME_FUSREPLACEREADER_HPP
