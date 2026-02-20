cmake_minimum_required(VERSION 3.25)
project(cpp-task)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")


set(Sources
        ./XML/XML_Parser.hpp
        ./XML/XML_Parser.cpp
        ./XML/XML_xerces_String.hpp
        ./XML/XML_xerces_String.cpp
        ./String/ST_String.hpp
        ./String/ST_String.cpp
        ./String/generic.hpp
        ./Misc/M_SystemMessage.cpp
        ./Misc/M_SystemMessage.hpp
        ./Misc/M_MemoryStream.hpp
        ./Misc/M_MemoryStream.cpp
        ./Misc/Memory.hpp
        ./Misc/Memory.cpp
        ./Types/TY_Blob.hpp
        ./Types/TY_Blob.cpp
        ./Types/TY_Base.hpp
        ./Types/TY_Base.cpp
        ./repository/utils.hpp
        ./repository/utils.cpp
        ./repository/FUSimpleReader.hpp
        ./repository/FUSimpleReader.cpp
        ./repository/FUSReplaceReader.hpp
        ./repository/FUSReplaceReader.cpp)

add_library(parser STATIC ${Sources})

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
)

FetchContent_MakeAvailable(fmt)


FetchContent_Declare(
        XercesC
        URL "https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.2.4.tar.gz"
        URL_HASH SHA256=3d8ec1c7f94e38fee0e4ca5ad1e1d9db23cbf3a10bba626f6b4afa2dedafe5ab
)

FetchContent_MakeAvailable(XercesC)

get_target_property(_inc_dir xerces-c INCLUDE_DIRECTORIES)
target_include_directories(xerces-c SYSTEM BEFORE INTERFACE "$<BUILD_INTERFACE:${_inc_dir}>")

target_link_libraries(parser PUBLIC xerces-c fmt::fmt)
