cmake_minimum_required(VERSION 3.15)
project(tinyparse)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/source/*.cpp"
    "${PROJECT_SOURCE_DIR}/source/generator/*.cpp"
)

add_custom_command(OUTPUT source/lex.tinylex
    COMMAND tinylex source/lex.tinylex -o include/tinylex.hpp
    COMMENT "Generating lexer"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_target(tinylex 
    DEPENDS source/lex.tinylex
)

add_executable(tinyparse ${all_SRCS})
add_dependencies(tinyparse tinylex)
set_target_properties(tinyparse PROPERTIES PUBLIC_HEADER 
    "tinyparse/cpp/cpp_header.txt;tinyparse/cpp/cpp_implement.txt")

install(TARGETS tinyparse 
    RUNTIME 
        DESTINATION bin
    PUBLIC_HEADER 
        DESTINATION include/tinyparse
)
