cmake_minimum_required(VERSION 3.16)

project(AutoChessC
    VERSION 1.12.0
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
    src/ai.c
    src/board.c
    src/battle.c
    src/command.c
    src/economy.c
    src/game.c
    src/hero.c
    src/logger.c
    src/player.c
    src/shop.c
    src/skill.c
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

add_executable(test_skill
    tests/test_skill.c
)

target_link_libraries(test_skill PRIVATE autochess_core)
autochess_enable_warnings(test_skill)

add_executable(test_ai
    tests/test_ai.c
)

target_link_libraries(test_ai PRIVATE autochess_core)
autochess_enable_warnings(test_ai)

add_executable(test_game
    tests/test_game.c
)

target_link_libraries(test_game PRIVATE autochess_core)
autochess_enable_warnings(test_game)

add_executable(test_command
    tests/test_command.c
)

target_link_libraries(test_command PRIVATE autochess_core)
autochess_enable_warnings(test_command)

enable_testing()
add_test(NAME battle_tests COMMAND test_battle)
add_test(NAME board_tests COMMAND test_board)
add_test(NAME player_tests COMMAND test_player)
add_test(NAME economy_tests COMMAND test_economy)
add_test(NAME shop_tests COMMAND test_shop)
add_test(NAME merge_tests COMMAND test_merge)
add_test(NAME trait_tests COMMAND test_trait)
add_test(NAME skill_tests COMMAND test_skill)
add_test(NAME ai_tests COMMAND test_ai)
add_test(NAME game_tests COMMAND test_game)
add_test(NAME command_tests COMMAND test_command)
