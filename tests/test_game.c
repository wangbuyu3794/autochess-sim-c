#include <stdio.h>

#include "game.h"

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

static void test_game_init_sets_defaults(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    EXPECT_EQ(100, game.player.health);
    EXPECT_EQ(100, game.enemy.health);
    EXPECT_EQ(0, game.current_round);
    EXPECT_EQ(GAME_RESULT_ONGOING, game.result);
}

static void test_seed_player_demo_units(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);

    game_seed_player_demo_units(&game);

    EXPECT_TRUE(player_count_active_units(&game.player) > 0);
    EXPECT_TRUE(player_count_deployed_units(&game.player) > 0);
}

static void test_calculate_damage_counts_alive_winners(void)
{
    BattleContext context = {0};
    BoardPosition p0 = {7, 3};
    BoardPosition p1 = {7, 4};
    BoardPosition e0 = {0, 3};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(2, hero_get_template(2), BATTLE_SIDE_PLAYER, p1));
    battle_add_unit(&context, battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, e0));
    context.units[2].is_alive = 0;

    EXPECT_EQ(AUTOCHESS_BASE_PLAYER_DAMAGE + 2, game_calculate_damage(&context, BATTLE_SIDE_PLAYER));
}

static void test_settle_battle_reduces_loser_health(void)
{
    GameContext game;
    BattleContext context = {0};
    BoardPosition p0 = {7, 3};
    BoardPosition e0 = {0, 3};
    game_init(&game, 1u, 2u);

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, e0));
    context.units[1].is_alive = 0;

    EXPECT_EQ(GAME_RESULT_ONGOING, game_settle_battle(&game, &context, BATTLE_RESULT_PLAYER_WIN));
    EXPECT_TRUE(game.enemy.health < 100);
    EXPECT_EQ(100, game.player.health);
}

static void test_round_equipment_reward_rotation_is_stable(void)
{
    EXPECT_EQ(EQUIPMENT_BROADSWORD, game_select_round_equipment_reward(1));
    EXPECT_EQ(EQUIPMENT_GUARDIAN_VEST, game_select_round_equipment_reward(2));
    EXPECT_EQ(EQUIPMENT_MANA_GEM, game_select_round_equipment_reward(3));
    EXPECT_EQ(EQUIPMENT_CRIT_GLOVES, game_select_round_equipment_reward(4));
    EXPECT_EQ(EQUIPMENT_BROADSWORD, game_select_round_equipment_reward(5));
}

static void test_settle_battle_rewards_winner_equipment(void)
{
    GameContext game;
    BattleContext context = {0};
    BoardPosition p0 = {7, 3};
    BoardPosition e0 = {0, 3};
    int player_swords_before = 0;
    int enemy_swords_before = 0;
    game_init(&game, 1u, 2u);
    game.current_round = 1;

    player_swords_before = player_count_equipment(&game.player, EQUIPMENT_BROADSWORD);
    enemy_swords_before = player_count_equipment(&game.enemy, EQUIPMENT_BROADSWORD);

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, e0));
    context.units[1].is_alive = 0;

    game_settle_battle(&game, &context, BATTLE_RESULT_PLAYER_WIN);

    EXPECT_EQ(player_swords_before + 1, player_count_equipment(&game.player, EQUIPMENT_BROADSWORD));
    EXPECT_EQ(enemy_swords_before, player_count_equipment(&game.enemy, EQUIPMENT_BROADSWORD));
}

static void test_game_run_round_advances_round(void)
{
    GameContext game;
    game_init(&game, 1u, 2u);
    game_seed_player_demo_units(&game);

    game_run_round(&game, NULL);

    EXPECT_EQ(1, game.current_round);
    EXPECT_TRUE(game.player.health <= 100);
    EXPECT_TRUE(game.enemy.health <= 100);
}

static void test_game_run_until_over_finishes(void)
{
    GameContext game;
    GameResult result = GAME_RESULT_ONGOING;
    game_init(&game, 1u, 2u);
    game_seed_player_demo_units(&game);

    result = game_run_until_over(&game, NULL);

    EXPECT_TRUE(result == GAME_RESULT_PLAYER_WIN ||
                result == GAME_RESULT_ENEMY_WIN ||
                result == GAME_RESULT_DRAW);
    EXPECT_TRUE(game.current_round <= AUTOCHESS_MAX_GAME_ROUNDS);
}

static void test_game_run_until_over_with_custom_limit(void)
{
    GameContext game;
    GameResult result = GAME_RESULT_ONGOING;
    game_init(&game, 1u, 2u);
    game_seed_player_demo_units(&game);

    result = game_run_until_over_with_limit(&game, 1, NULL);

    EXPECT_EQ(GAME_RESULT_DRAW, result);
    EXPECT_EQ(1, game.current_round);
}

int main(void)
{
    test_game_init_sets_defaults();
    test_seed_player_demo_units();
    test_calculate_damage_counts_alive_winners();
    test_settle_battle_reduces_loser_health();
    test_round_equipment_reward_rotation_is_stable();
    test_settle_battle_rewards_winner_equipment();
    test_game_run_round_advances_round();
    test_game_run_until_over_finishes();
    test_game_run_until_over_with_custom_limit();

    if (g_failed_tests == 0)
    {
        printf("All game tests passed.\n");
        return 0;
    }

    printf("%d game test(s) failed.\n", g_failed_tests);
    return 1;
}
