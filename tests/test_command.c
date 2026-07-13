#include <stdio.h>

#include "command.h"

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

static void test_buy_command_adds_unit(void)
{
    GameContext game;
    int gold_before = 0;

    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    gold_before = game.player.gold;

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "buy 1", NULL));
    EXPECT_EQ(1, player_count_active_units(&game.player));
    EXPECT_TRUE(game.player.gold < gold_before);
    EXPECT_EQ(2, game.player_next_instance_id);
}

static void test_refresh_command_costs_gold(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    game.player.gold = 10;

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "refresh", NULL));
    EXPECT_EQ(8, game.player.gold);
}

static void test_lock_command_toggles_shop_lock(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "lock", NULL));
    EXPECT_EQ(1, game.player_shop.is_locked);
    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "lock", NULL));
    EXPECT_EQ(0, game.player_shop.is_locked);
}

static void test_odds_and_pool_commands(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "odds", NULL));
    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "pool", NULL));
}

static void test_traits_command(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);
    command_execute_preparation(&game, "deploy 1 6 3", NULL);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "traits", NULL));
}

static void test_items_command_lists_equipment(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "items", NULL));
}

static void test_equip_command_equips_unit(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "equip 0 1", NULL));
    EXPECT_EQ(EQUIPMENT_BROADSWORD, game.player.units[0].equipment_id);
    EXPECT_EQ(0, player_count_equipment(&game.player, EQUIPMENT_BROADSWORD));
}

static void test_unequip_command_returns_equipment(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);
    command_execute_preparation(&game, "equip 0 1", NULL);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "unequip 0", NULL));
    EXPECT_EQ(EQUIPMENT_NONE, game.player.units[0].equipment_id);
    EXPECT_EQ(1, player_count_equipment(&game.player, EQUIPMENT_BROADSWORD));
    EXPECT_EQ(COMMAND_RESULT_ERROR, command_execute_preparation(&game, "unequip 0", NULL));
}

static void test_equipfit_command_reports_scores(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "equipfit 0", NULL));
    EXPECT_EQ(COMMAND_RESULT_ERROR, command_execute_preparation(&game, "equipfit 99", NULL));
}

static void test_buyxp_command_spends_gold_and_levels_up(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    game.player.gold = 10;

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "buyxp", NULL));
    EXPECT_EQ(6, game.player.gold);
    EXPECT_EQ(4, game.player.level);
    EXPECT_EQ(0, game.player.experience);
}

static void test_auto_command_deploys_units(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "auto", NULL));
    EXPECT_EQ(1, player_count_deployed_units(&game.player));
    EXPECT_TRUE(game.player.units[0].equipment_id != EQUIPMENT_NONE);
}

static void test_deploy_command_places_bench_unit(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "deploy 1 6 3", NULL));
    EXPECT_EQ(1, player_count_deployed_units(&game.player));
    EXPECT_TRUE(player_is_position_occupied(&game.player, (BoardPosition){6, 3}));
}

static void test_move_command_moves_deployed_unit(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);
    command_execute_preparation(&game, "deploy 1 6 3", NULL);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "move 6 3 7 3", NULL));
    EXPECT_TRUE(!player_is_position_occupied(&game.player, (BoardPosition){6, 3}));
    EXPECT_TRUE(player_is_position_occupied(&game.player, (BoardPosition){7, 3}));
}

static void test_recall_command_returns_unit_to_bench(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);
    command_execute_preparation(&game, "deploy 1 6 3", NULL);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "recall 6 3", NULL));
    EXPECT_EQ(0, player_count_deployed_units(&game.player));
    EXPECT_TRUE(player_count_active_units(&game.player) == 1);
}

static void test_sell_command_removes_unit_and_refunds_gold(void)
{
    GameContext game;
    int gold_before = 0;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);
    gold_before = game.player.gold;

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "sell 0", NULL));
    EXPECT_EQ(0, player_count_active_units(&game.player));
    EXPECT_TRUE(game.player.gold > gold_before);
}

static void test_sell_rejects_missing_unit(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    EXPECT_EQ(COMMAND_RESULT_ERROR, command_execute_preparation(&game, "sell 99", NULL));
    EXPECT_EQ(10, game.player.gold);
}

static void test_deploy_rejects_enemy_side(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    shop_refresh(&game.player_shop, game.player.level);
    command_execute_preparation(&game, "buy 1", NULL);

    EXPECT_EQ(COMMAND_RESULT_ERROR, command_execute_preparation(&game, "deploy 1 0 3", NULL));
    EXPECT_EQ(0, player_count_deployed_units(&game.player));
}

static void test_ready_and_quit_commands(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    EXPECT_EQ(COMMAND_RESULT_READY, command_execute_preparation(&game, "ready", NULL));
    EXPECT_EQ(COMMAND_RESULT_QUIT, command_execute_preparation(&game, "quit", NULL));
}

static void test_command_accepts_utf8_bom(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    EXPECT_EQ(COMMAND_RESULT_CONTINUE, command_execute_preparation(&game, "\xEF\xBB\xBFshop", NULL));
}

static void test_invalid_buy_reports_error(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    EXPECT_EQ(COMMAND_RESULT_ERROR, command_execute_preparation(&game, "buy 9", NULL));
    EXPECT_EQ(0, player_count_active_units(&game.player));
}

int main(void)
{
    test_buy_command_adds_unit();
    test_refresh_command_costs_gold();
    test_lock_command_toggles_shop_lock();
    test_odds_and_pool_commands();
    test_traits_command();
    test_items_command_lists_equipment();
    test_equip_command_equips_unit();
    test_unequip_command_returns_equipment();
    test_equipfit_command_reports_scores();
    test_buyxp_command_spends_gold_and_levels_up();
    test_auto_command_deploys_units();
    test_deploy_command_places_bench_unit();
    test_move_command_moves_deployed_unit();
    test_recall_command_returns_unit_to_bench();
    test_sell_command_removes_unit_and_refunds_gold();
    test_sell_rejects_missing_unit();
    test_deploy_rejects_enemy_side();
    test_ready_and_quit_commands();
    test_command_accepts_utf8_bom();
    test_invalid_buy_reports_error();

    if (g_failed_tests == 0)
    {
        printf("All command tests passed.\n");
        return 0;
    }

    printf("%d command test(s) failed.\n", g_failed_tests);
    return 1;
}
