cmake_minimum_required(VERSION 3.25)
project(cpp-task)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(Sources
        ./XML/XML_Parser.cpp
        ./XML/XML_xerces_String.cpp
        ./String/ST_String.cpp
        ./Misc/M_SystemMessage.cpp
        ./Misc/M_MemoryStream.cpp
        ./Misc/Memory.cpp
        ./Types/TY_Blob.cpp
        ./Types/TY_Base.cpp
        ./repository/utils.cpp
        ./repository/FUSimpleReader.cpp
        ./repository/FUSReplaceReader.cpp
)

add_library(parser STATIC ${Sources})

target_include_directories(parser
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(parser PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(parser PRIVATE -fsanitize=address)

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 12.1.0
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

include(CTest)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
