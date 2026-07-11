#include <stdio.h>

#include "player.h"

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

static void test_two_same_units_do_not_merge(void)
{
    Player player;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_add_unit_to_bench(&player, unit_create(2, 1));

    EXPECT_EQ(2, player_count_active_units(&player));
    EXPECT_EQ(2, player_count_units_by_template_and_star(&player, 1, 1));
}

static void test_three_one_star_units_merge_to_two_star(void)
{
    Player player;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_add_unit_to_bench(&player, unit_create(2, 1));
    player_add_unit_to_bench(&player, unit_create(3, 1));

    EXPECT_EQ(1, player_count_active_units(&player));
    EXPECT_EQ(0, player_count_units_by_template_and_star(&player, 1, 1));
    EXPECT_EQ(1, player_count_units_by_template_and_star(&player, 1, 2));
    EXPECT_EQ(2, player.units[0].star);
    EXPECT_TRUE(unit_is_on_bench(&player.units[0]));
}

static void test_nine_one_star_units_merge_to_three_star(void)
{
    Player player;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    for (int i = 0; i < 9; ++i)
    {
        player_add_unit_to_bench(&player, unit_create(100 + i, 1));
    }

    EXPECT_EQ(1, player_count_active_units(&player));
    EXPECT_EQ(1, player_count_units_by_template_and_star(&player, 1, 3));
}

static void test_merge_keeps_deployed_unit_position(void)
{
    Player player;
    BoardPosition position = {7, 3};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_deploy_from_bench(&player, 0, position);
    player_add_unit_to_bench(&player, unit_create(2, 1));
    player_add_unit_to_bench(&player, unit_create(3, 1));

    EXPECT_EQ(1, player_count_deployed_units(&player));
    EXPECT_EQ(2, player.units[0].star);
    EXPECT_TRUE(unit_is_deployed(&player.units[0]));
    EXPECT_EQ(7, player.units[0].position.row);
    EXPECT_EQ(3, player.units[0].position.col);
}

static void test_two_star_unit_uses_scaled_battle_stats(void)
{
    Player player;
    BattleContext context = {0};
    BoardPosition position = {7, 3};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_add_unit_to_bench(&player, unit_create(2, 1));
    player_add_unit_to_bench(&player, unit_create(3, 1));
    player_deploy_from_bench(&player, 0, position);
    player_add_deployed_units_to_battle(&player, &context);

    EXPECT_EQ(1, context.unit_count);
    EXPECT_EQ(234, context.units[0].max_hp);
    EXPECT_EQ(27, context.units[0].attack);
}

int main(void)
{
    test_two_same_units_do_not_merge();
    test_three_one_star_units_merge_to_two_star();
    test_nine_one_star_units_merge_to_three_star();
    test_merge_keeps_deployed_unit_position();
    test_two_star_unit_uses_scaled_battle_stats();

    if (g_failed_tests == 0)
    {
        printf("All merge tests passed.\n");
        return 0;
    }

    printf("%d merge test(s) failed.\n", g_failed_tests);
    return 1;
}
