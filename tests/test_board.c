#include <stdio.h>

#include "board.h"
#include "config.h"

static int g_failed_tests = 0;

#define EXPECT_TRUE(condition)                                              \
    do                                                                      \
    {                                                                       \
        if (!(condition))                                                   \
        {                                                                   \
            printf("FAILED: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            g_failed_tests += 1;                                            \
        }                                                                   \
    } while (0)

#define EXPECT_EQ(expected, actual)                                                                 \
    do                                                                                              \
    {                                                                                               \
        int expected_value = (expected);                                                            \
        int actual_value = (actual);                                                                \
        if (expected_value != actual_value)                                                         \
        {                                                                                           \
            printf("FAILED: %s:%d: expected %d, got %d\n", __FILE__, __LINE__, expected_value, actual_value); \
            g_failed_tests += 1;                                                                    \
        }                                                                                           \
    } while (0)

static void test_valid_position(void)
{
    BoardPosition position = {0, 0};
    BoardPosition edge = {AUTOCHESS_BOARD_ROWS - 1, AUTOCHESS_BOARD_COLS - 1};

    EXPECT_TRUE(board_is_position_valid(position));
    EXPECT_TRUE(board_is_position_valid(edge));
}

static void test_invalid_position(void)
{
    BoardPosition negative_row = {-1, 0};
    BoardPosition too_large_col = {0, AUTOCHESS_BOARD_COLS};

    EXPECT_TRUE(!board_is_position_valid(negative_row));
    EXPECT_TRUE(!board_is_position_valid(too_large_col));
}

static void test_deploy_regions(void)
{
    BoardPosition enemy_position = {0, 3};
    BoardPosition player_position = {AUTOCHESS_BOARD_ROWS - 1, 3};

    EXPECT_TRUE(board_is_enemy_deploy_position(enemy_position));
    EXPECT_TRUE(!board_is_player_deploy_position(enemy_position));
    EXPECT_TRUE(board_is_player_deploy_position(player_position));
    EXPECT_TRUE(!board_is_enemy_deploy_position(player_position));
}

static void test_manhattan_distance(void)
{
    BoardPosition a = {7, 3};
    BoardPosition b = {4, 1};

    EXPECT_EQ(5, board_manhattan_distance(a, b));
}

int main(void)
{
    test_valid_position();
    test_invalid_position();
    test_deploy_regions();
    test_manhattan_distance();

    if (g_failed_tests == 0)
    {
        printf("All board tests passed.\n");
        return 0;
    }

    printf("%d board test(s) failed.\n", g_failed_tests);
    return 1;
}
