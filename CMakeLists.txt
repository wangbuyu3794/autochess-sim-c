cmake_minimum_required(VERSION 3.16)

project(AutoChessC
    VERSION 0.6.0
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
    src/economy.c
    src/hero.c
    src/logger.c
    src/player.c
    src/shop.c
    src/trait.c
    src/unit.c
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

add_executable(test_player
    tests/test_player.c
)

target_link_libraries(test_player PRIVATE autochess_core)
autochess_enable_warnings(test_player)

add_executable(test_economy
    tests/test_economy.c
)

target_link_libraries(test_economy PRIVATE autochess_core)
autochess_enable_warnings(test_economy)

add_executable(test_shop
    tests/test_shop.c
)

target_link_libraries(test_shop PRIVATE autochess_core)
autochess_enable_warnings(test_shop)

add_executable(test_merge
    tests/test_merge.c
)

target_link_libraries(test_merge PRIVATE autochess_core)
autochess_enable_warnings(test_merge)

add_executable(test_trait
    tests/test_trait.c
)

target_link_libraries(test_trait PRIVATE autochess_core)
autochess_enable_warnings(test_trait)

enable_testing()
add_test(NAME battle_tests COMMAND test_battle)
add_test(NAME board_tests COMMAND test_board)
add_test(NAME player_tests COMMAND test_player)
add_test(NAME economy_tests COMMAND test_economy)
add_test(NAME shop_tests COMMAND test_shop)
add_test(NAME merge_tests COMMAND test_merge)
add_test(NAME trait_tests COMMAND test_trait)
