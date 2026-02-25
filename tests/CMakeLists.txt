add_executable(unit_tests
        test_FUSimpleReader.cpp
        test_FUSReplaceReader.cpp
        test_utils.cpp
        test_string_generics.cpp
        test_ST_String.cpp
        test_xerces_string.cpp
        test_MemoryStream.cpp
        test_TY_Blob.cpp
        test_XML_Parser.cpp
)

target_link_libraries(unit_tests PRIVATE
        parser
        Catch2::Catch2WithMain
)

target_compile_definitions(unit_tests PRIVATE
        PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

add_test(NAME unit_tests COMMAND unit_tests)