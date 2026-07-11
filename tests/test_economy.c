#include <stdio.h>

#include "economy.h"

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

static void test_interest_is_capped(void)
{
    EXPECT_EQ(0, economy_calculate_interest(0));
    EXPECT_EQ(1, economy_calculate_interest(10));
    EXPECT_EQ(5, economy_calculate_interest(50));
    EXPECT_EQ(5, economy_calculate_interest(90));
}

static void test_round_income_includes_interest(void)
{
    EXPECT_EQ(5, economy_calculate_round_income(0));
    EXPECT_EQ(7, economy_calculate_round_income(25));
}

static void test_apply_round_income(void)
{
    Player player;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    player.gold = 20;

    economy_apply_round_income(&player);

    EXPECT_EQ(27, player.gold);
}

static void test_spend_gold(void)
{
    Player player;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    player.gold = 3;

    EXPECT_TRUE(!economy_spend_gold(&player, 4));
    EXPECT_EQ(3, player.gold);
    EXPECT_TRUE(economy_spend_gold(&player, 2));
    EXPECT_EQ(1, player.gold);
}

static void test_buy_experience_levels_up(void)
{
    Player player;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    player.gold = 10;

    EXPECT_TRUE(economy_buy_experience(&player));
    EXPECT_EQ(6, player.gold);
    EXPECT_EQ(4, player.level);
    EXPECT_EQ(0, player.experience);
}

int main(void)
{
    test_interest_is_capped();
    test_round_income_includes_interest();
    test_apply_round_income();
    test_spend_gold();
    test_buy_experience_levels_up();

    if (g_failed_tests == 0)
    {
        printf("All economy tests passed.\n");
        return 0;
    }

    printf("%d economy test(s) failed.\n", g_failed_tests);
    return 1;
}
