#ifndef E2ERUNTIME_FUSIMPLEREADER_HPP
#define E2ERUNTIME_FUSIMPLEREADER_HPP

#include "../String/ST_String.hpp"
class TY_Blob;

namespace functions::repository::simple
{

    struct Function {
        ST_String id;
        ST_String source;
    };

    using Functions = std::vector<Function>;

    Functions readRepo( const TY_Blob &data, std::string_view repo );

}//namespace functions::repository::simple

#endif //E2ERUNTIME_FUSIMPLEREADER_HPP
