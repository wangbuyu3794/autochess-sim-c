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

static void test_shield_absorbs_damage_before_hp(void)
{
    BattleUnit unit = battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER);

    battle_add_shield(&unit, 20);
    battle_apply_damage(&unit, 30);

    EXPECT_EQ(0, unit.shield);
    EXPECT_EQ(unit.max_hp - 10, unit.current_hp);
    EXPECT_TRUE(unit.is_alive);
}

static void test_burn_deals_damage_at_turn_start(void)
{
    BattleUnit unit = battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER);

    battle_apply_burn(&unit, 6, 2);
    EXPECT_TRUE(battle_process_status_start(&unit, NULL));

    EXPECT_EQ(unit.max_hp - 6, unit.current_hp);
    EXPECT_EQ(1, unit.burn_turns);
}

static void test_stun_skips_action_at_turn_start(void)
{
    BattleUnit unit = battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER);

    battle_apply_stun(&unit, 1);

    EXPECT_TRUE(!battle_process_status_start(&unit, NULL));
    EXPECT_EQ(0, unit.stun_turns);
}

static void test_mitigated_damage_uses_resistance(void)
{
    EXPECT_EQ(40, battle_calculate_mitigated_damage(50, 25));
    EXPECT_EQ(1, battle_calculate_mitigated_damage(1, 300));
    EXPECT_EQ(0, battle_calculate_mitigated_damage(0, 25));
}

static void test_attack_damage_uses_armor(void)
{
    BattleUnit attacker = battle_create_unit(1, hero_get_template(2), BATTLE_SIDE_PLAYER);
    BattleUnit target = battle_create_unit(101, hero_get_template(1), BATTLE_SIDE_ENEMY);

    attacker.attack = 50;
    attacker.crit_chance = 0;
    target.armor = 25;

    EXPECT_EQ(40, battle_calculate_attack_damage(&attacker, &target, 1));
}

static void test_attack_damage_can_crit(void)
{
    BattleUnit attacker = battle_create_unit(1, hero_get_template(2), BATTLE_SIDE_PLAYER);
    BattleUnit target = battle_create_unit(101, hero_get_template(1), BATTLE_SIDE_ENEMY);

    attacker.attack = 40;
    attacker.crit_chance = 100;
    attacker.crit_damage = 175;
    target.armor = 0;

    EXPECT_TRUE(battle_is_critical_hit(&attacker, &target, 1));
    EXPECT_EQ(70, battle_calculate_attack_damage(&attacker, &target, 1));
}

static void test_spell_damage_uses_magic_resist(void)
{
    BattleUnit target = battle_create_unit(101, hero_get_template(1), BATTLE_SIDE_ENEMY);

    target.magic_resist = 20;

    EXPECT_EQ(37, battle_calculate_spell_damage(45, &target));
}

static void test_selects_lowest_hp_target(void)
{
    BattleContext context = make_empty_context();
    BoardPosition attacker_position = {7, 3};
    BoardPosition enemy_a_position = {5, 3};
    BoardPosition enemy_b_position = {6, 2};
    BattleUnit attacker = battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, attacker_position);
    BattleUnit enemy_a = battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, enemy_a_position);
    BattleUnit enemy_b = battle_create_unit_at(102, hero_get_template(5), BATTLE_SIDE_ENEMY, enemy_b_position);

    enemy_a.current_hp = 40;
    enemy_b.current_hp = 20;

    battle_add_unit(&context, attacker);
    battle_add_unit(&context, enemy_a);
    battle_add_unit(&context, enemy_b);

    EXPECT_EQ(2, battle_select_target_nearest(&context, 0));
}

static void test_selects_smaller_instance_id_when_hp_ties(void)
{
    BattleContext context = make_empty_context();
    BoardPosition attacker_position = {7, 3};
    BoardPosition enemy_a_position = {5, 3};
    BoardPosition enemy_b_position = {6, 2};
    BattleUnit attacker = battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, attacker_position);
    BattleUnit enemy_a = battle_create_unit_at(103, hero_get_template(4), BATTLE_SIDE_ENEMY, enemy_a_position);
    BattleUnit enemy_b = battle_create_unit_at(102, hero_get_template(5), BATTLE_SIDE_ENEMY, enemy_b_position);

    enemy_a.current_hp = 20;
    enemy_b.current_hp = 20;

    battle_add_unit(&context, attacker);
    battle_add_unit(&context, enemy_a);
    battle_add_unit(&context, enemy_b);

    EXPECT_EQ(2, battle_select_target_nearest(&context, 0));
}

static void test_selects_nearest_target_before_lowest_hp(void)
{
    BattleContext context = make_empty_context();
    BoardPosition attacker_position = {7, 3};
    BoardPosition near_position = {6, 3};
    BoardPosition far_position = {0, 3};
    BattleUnit attacker = battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, attacker_position);
    BattleUnit near_enemy = battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, near_position);
    BattleUnit far_enemy = battle_create_unit_at(102, hero_get_template(5), BATTLE_SIDE_ENEMY, far_position);

    near_enemy.current_hp = 90;
    far_enemy.current_hp = 1;

    battle_add_unit(&context, attacker);
    battle_add_unit(&context, near_enemy);
    battle_add_unit(&context, far_enemy);

    EXPECT_EQ(1, battle_select_target_nearest(&context, 0));
}

static void test_range_check_uses_manhattan_distance(void)
{
    BoardPosition attacker_position = {7, 3};
    BoardPosition target_position = {5, 3};
    BattleUnit attacker = battle_create_unit_at(1, hero_get_template(3), BATTLE_SIDE_PLAYER, attacker_position);
    BattleUnit target = battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, target_position);

    EXPECT_TRUE(battle_is_target_in_range(&attacker, &target));
}

static void test_unit_moves_toward_target(void)
{
    BattleContext context = make_empty_context();
    BoardPosition start = {7, 3};
    BoardPosition target_position = {4, 3};
    BattleUnit mover = battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, start);
    BattleUnit enemy = battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, target_position);

    battle_add_unit(&context, mover);
    battle_add_unit(&context, enemy);

    EXPECT_TRUE(battle_try_move_toward(&context, 0, target_position));
    EXPECT_EQ(6, context.units[0].position.row);
    EXPECT_EQ(3, context.units[0].position.col);
}

static void test_unit_does_not_move_into_occupied_cell(void)
{
    BattleContext context = make_empty_context();
    BoardPosition start = {7, 3};
    BoardPosition blocker_position = {6, 3};
    BoardPosition target_position = {4, 3};
    BattleUnit mover = battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, start);
    BattleUnit blocker = battle_create_unit_at(2, hero_get_template(2), BATTLE_SIDE_PLAYER, blocker_position);
    BattleUnit enemy = battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, target_position);

    battle_add_unit(&context, mover);
    battle_add_unit(&context, blocker);
    battle_add_unit(&context, enemy);

    EXPECT_TRUE(!battle_try_move_toward(&context, 0, target_position));
    EXPECT_EQ(7, context.units[0].position.row);
    EXPECT_EQ(3, context.units[0].position.col);
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
    test_shield_absorbs_damage_before_hp();
    test_burn_deals_damage_at_turn_start();
    test_stun_skips_action_at_turn_start();
    test_mitigated_damage_uses_resistance();
    test_attack_damage_uses_armor();
    test_attack_damage_can_crit();
    test_spell_damage_uses_magic_resist();
    test_selects_lowest_hp_target();
    test_selects_smaller_instance_id_when_hp_ties();
    test_selects_nearest_target_before_lowest_hp();
    test_range_check_uses_manhattan_distance();
    test_unit_moves_toward_target();
    test_unit_does_not_move_into_occupied_cell();
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
