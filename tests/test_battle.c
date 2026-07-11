#include <stdio.h>

#include "battle.h"

static int g_failed_tests = 0;

#define EXPECT_TRUE(condition)                                                                      \
    do                                                                                              \
    {                                                                                               \
        if (!(condition))                                                                           \
        {                                                                                           \
            printf("FAILED: %s:%d: %s\n", __FILE__, __LINE__, #condition);                         \
            g_failed_tests += 1;                                                                    \
        }                                                                                           \
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

static BattleContext make_empty_context(void)
{
    BattleContext context = {0};
    return context;
}

static void test_damage_reduces_hp(void)
{
    BattleUnit unit = battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER);

    battle_apply_damage(&unit, 30);

    EXPECT_EQ(unit.max_hp - 30, unit.current_hp);
    EXPECT_TRUE(unit.is_alive);
}

static void test_damage_clamps_hp_and_kills_unit(void)
{
    BattleUnit unit = battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER);

    battle_apply_damage(&unit, unit.max_hp + 50);

    EXPECT_EQ(0, unit.current_hp);
    EXPECT_EQ(0, unit.is_alive);
}

static void test_selects_lowest_hp_target(void)
{
    BattleContext context = make_empty_context();
    BattleUnit attacker = battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER);
    BattleUnit enemy_a = battle_create_unit(101, hero_get_template(4), BATTLE_SIDE_ENEMY);
    BattleUnit enemy_b = battle_create_unit(102, hero_get_template(5), BATTLE_SIDE_ENEMY);

    enemy_a.current_hp = 40;
    enemy_b.current_hp = 20;

    battle_add_unit(&context, attacker);
    battle_add_unit(&context, enemy_a);
    battle_add_unit(&context, enemy_b);

    EXPECT_EQ(2, battle_select_target_lowest_hp(&context, BATTLE_SIDE_PLAYER));
}

static void test_selects_smaller_instance_id_when_hp_ties(void)
{
    BattleContext context = make_empty_context();
    BattleUnit attacker = battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER);
    BattleUnit enemy_a = battle_create_unit(103, hero_get_template(4), BATTLE_SIDE_ENEMY);
    BattleUnit enemy_b = battle_create_unit(102, hero_get_template(5), BATTLE_SIDE_ENEMY);

    enemy_a.current_hp = 20;
    enemy_b.current_hp = 20;

    battle_add_unit(&context, attacker);
    battle_add_unit(&context, enemy_a);
    battle_add_unit(&context, enemy_b);

    EXPECT_EQ(2, battle_select_target_lowest_hp(&context, BATTLE_SIDE_PLAYER));
}

static void test_battle_result_when_one_side_is_defeated(void)
{
    BattleContext context = make_empty_context();
    BattleUnit player = battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER);
    BattleUnit enemy = battle_create_unit(101, hero_get_template(4), BATTLE_SIDE_ENEMY);

    enemy.current_hp = 0;
    enemy.is_alive = 0;

    battle_add_unit(&context, player);
    battle_add_unit(&context, enemy);

    EXPECT_EQ(BATTLE_RESULT_PLAYER_WIN, battle_check_result(&context));
}

static void test_demo_battle_finishes(void)
{
    BattleContext context = battle_create_demo_context();
    BattleResult result = battle_run(&context, NULL);

    EXPECT_TRUE(result == BATTLE_RESULT_PLAYER_WIN ||
                result == BATTLE_RESULT_ENEMY_WIN ||
                result == BATTLE_RESULT_DRAW);
    EXPECT_TRUE(context.current_round <= AUTOCHESS_MAX_BATTLE_ROUNDS);
}

int main(void)
{
    test_damage_reduces_hp();
    test_damage_clamps_hp_and_kills_unit();
    test_selects_lowest_hp_target();
    test_selects_smaller_instance_id_when_hp_ties();
    test_battle_result_when_one_side_is_defeated();
    test_demo_battle_finishes();

    if (g_failed_tests == 0)
    {
        printf("All battle tests passed.\n");
        return 0;
    }

    printf("%d battle test(s) failed.\n", g_failed_tests);
    return 1;
}
