cmake_minimum_required(VERSION 3.15)
project(tinyparse)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_BINARY_DIR})
file(GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/source/*.c"
    "${PROJECT_SOURCE_DIR}/source/generator/*.c"
)

add_executable(template "templates/template.c")
add_custom_target(template_files)
add_custom_command(TARGET template_files
    POST_BUILD
    COMMAND ./template ${PROJECT_SOURCE_DIR}/templates
    COMMENT "Generating templates"
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
)
add_dependencies(template_files template)

add_custom_command(OUTPUT source/lex.tinylex
    COMMAND 
		tinylex source/lex.tinylex -o include/tinylex.h -l c
    COMMENT "Generating lexer"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
add_custom_command(OUTPUT source/tinylex_intergration.tinylex
    COMMAND 
		tinylex source/tinylex_intergration.tinylex -o include/tinylex_intergration_lexer.h -l c
    COMMENT "Generating tinylex intergration lexer"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)


add_custom_target(tinylex 
    DEPENDS source/lex.tinylex source/tinylex_intergration.tinylex
)

add_executable(tinyparse ${all_SRCS})
add_dependencies(tinyparse template_files)
add_dependencies(tinyparse tinylex)

install(TARGETS tinyparse
    RUNTIME 
        DESTINATION bin
)
