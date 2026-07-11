cmake_minimum_required(VERSION 3.16)

project(AutoChessC
    VERSION 0.2.0
    DESCRIPTION "A terminal auto chess simulator for learning C"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

function(autochess_enable_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_library(autochess_core
    src/board.c
    src/battle.c
    src/hero.c
    src/logger.c
)

target_include_directories(autochess_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

autochess_enable_warnings(autochess_core)

add_executable(autochess
    src/main.c
)

target_link_libraries(autochess PRIVATE autochess_core)
autochess_enable_warnings(autochess)

add_executable(test_battle
    tests/test_battle.c
)

target_link_libraries(test_battle PRIVATE autochess_core)
autochess_enable_warnings(test_battle)

add_executable(test_board
    tests/test_board.c
)

target_link_libraries(test_board PRIVATE autochess_core)
autochess_enable_warnings(test_board)

enable_testing()
add_test(NAME battle_tests COMMAND test_battle)
add_test(NAME board_tests COMMAND test_board)
