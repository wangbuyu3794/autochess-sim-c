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

static void test_add_unit_to_bench(void)
{
    Player player;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    EXPECT_EQ(PLAYER_OK, player_add_unit_to_bench(&player, unit_create(1, 1)));
    EXPECT_EQ(1, player.unit_count);
    EXPECT_EQ(0, player.units[0].bench_index);
    EXPECT_TRUE(unit_is_on_bench(&player.units[0]));
}

static void test_deploy_from_bench(void)
{
    Player player;
    BoardPosition position = {7, 3};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));

    EXPECT_EQ(PLAYER_OK, player_deploy_from_bench(&player, 0, position));
    EXPECT_EQ(1, player_count_deployed_units(&player));
    EXPECT_TRUE(unit_is_deployed(&player.units[0]));
    EXPECT_EQ(-1, player.bench_slots[0]);
    EXPECT_EQ(7, player.units[0].position.row);
    EXPECT_EQ(3, player.units[0].position.col);
}

static void test_rejects_enemy_area_for_player(void)
{
    Player player;
    BoardPosition enemy_area = {0, 3};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));

    EXPECT_EQ(PLAYER_ERROR_INVALID_DEPLOY_POSITION, player_deploy_from_bench(&player, 0, enemy_area));
    EXPECT_TRUE(unit_is_on_bench(&player.units[0]));
}

static void test_rejects_occupied_position(void)
{
    Player player;
    BoardPosition position = {7, 3};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_add_unit_to_bench(&player, unit_create(2, 2));
    player_deploy_from_bench(&player, 0, position);

    EXPECT_EQ(PLAYER_ERROR_POSITION_OCCUPIED, player_deploy_from_bench(&player, 1, position));
}

static void test_respects_deploy_limit(void)
{
    Player player;
    BoardPosition p0 = {7, 1};
    BoardPosition p1 = {7, 2};
    BoardPosition p2 = {7, 3};
    BoardPosition p3 = {7, 4};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    player.level = 3;

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_add_unit_to_bench(&player, unit_create(2, 2));
    player_add_unit_to_bench(&player, unit_create(3, 3));
    player_add_unit_to_bench(&player, unit_create(4, 4));

    EXPECT_EQ(PLAYER_OK, player_deploy_from_bench(&player, 0, p0));
    EXPECT_EQ(PLAYER_OK, player_deploy_from_bench(&player, 1, p1));
    EXPECT_EQ(PLAYER_OK, player_deploy_from_bench(&player, 2, p2));
    EXPECT_EQ(PLAYER_ERROR_DEPLOY_LIMIT_REACHED, player_deploy_from_bench(&player, 3, p3));
}

static void test_move_deployed_unit(void)
{
    Player player;
    BoardPosition from = {7, 3};
    BoardPosition to = {6, 3};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_deploy_from_bench(&player, 0, from);

    EXPECT_EQ(PLAYER_OK, player_move_deployed_unit(&player, from, to));
    EXPECT_EQ(6, player.units[0].position.row);
    EXPECT_EQ(3, player.units[0].position.col);
}

static void test_return_unit_to_bench(void)
{
    Player player;
    BoardPosition position = {7, 3};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_deploy_from_bench(&player, 0, position);

    EXPECT_EQ(PLAYER_OK, player_return_unit_to_bench(&player, position));
    EXPECT_EQ(0, player_count_deployed_units(&player));
    EXPECT_TRUE(unit_is_on_bench(&player.units[0]));
}

static void test_deployed_units_can_create_battle_context(void)
{
    Player player;
    BattleContext context = {0};
    BoardPosition position = {7, 3};
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    player_add_unit_to_bench(&player, unit_create(1, 1));
    player_deploy_from_bench(&player, 0, position);
    player_add_deployed_units_to_battle(&player, &context);

    EXPECT_EQ(1, context.unit_count);
    EXPECT_EQ(1, context.units[0].instance_id);
    EXPECT_EQ(BATTLE_SIDE_PLAYER, context.units[0].side);
    EXPECT_EQ(7, context.units[0].position.row);
    EXPECT_EQ(3, context.units[0].position.col);
}

int main(void)
{
    test_add_unit_to_bench();
    test_deploy_from_bench();
    test_rejects_enemy_area_for_player();
    test_rejects_occupied_position();
    test_respects_deploy_limit();
    test_move_deployed_unit();
    test_return_unit_to_bench();
    test_deployed_units_can_create_battle_context();

    if (g_failed_tests == 0)
    {
        printf("All player tests passed.\n");
        return 0;
    }

    printf("%d player test(s) failed.\n", g_failed_tests);
    return 1;
}
