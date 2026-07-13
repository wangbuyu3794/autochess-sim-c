#include <stdio.h>

#include "battle.h"
#include "skill.h"

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

static BattleContext make_context_with_two_units(int player_template_id, int enemy_template_id)
{
    BattleContext context = {0};
    BoardPosition player_position = {7, 3};
    BoardPosition enemy_position = {6, 3};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(player_template_id), BATTLE_SIDE_PLAYER, player_position));
    battle_add_unit(&context, battle_create_unit_at(101, hero_get_template(enemy_template_id), BATTLE_SIDE_ENEMY, enemy_position));

    return context;
}

static void test_mana_gain_is_capped(void)
{
    BattleUnit unit = battle_create_unit(1, hero_get_template(4), BATTLE_SIDE_PLAYER);

    battle_gain_mana(&unit, 40);
    battle_gain_mana(&unit, 40);

    EXPECT_EQ(unit.max_mana, unit.current_mana);
}

static void test_skill_definitions_are_available(void)
{
    const SkillDefinition *shield = skill_get_definition(SKILL_IRON_SHIELD);
    const SkillDefinition *strike = skill_get_definition(SKILL_POWER_STRIKE);
    const SkillDefinition *fireball = skill_get_definition(SKILL_FIREBALL);
    const SkillDefinition *execute = skill_get_definition(SKILL_EXECUTE_STRIKE);

    EXPECT_TRUE(shield != 0);
    EXPECT_TRUE(strike != 0);
    EXPECT_TRUE(fireball != 0);
    EXPECT_TRUE(execute != 0);
    EXPECT_EQ(SKILL_TARGET_SELF, shield->target_type);
    EXPECT_EQ(SKILL_DAMAGE_PHYSICAL, strike->damage_type);
    EXPECT_EQ(SKILL_DAMAGE_MAGICAL, fireball->damage_type);
    EXPECT_EQ(40, execute->execute_threshold_percent);
    EXPECT_EQ(20, shield->shield);
    EXPECT_EQ(6, fireball->burn_damage);
    EXPECT_EQ(1, execute->stun_turns);
}

static void test_skill_definition_calculates_physical_damage(void)
{
    BattleContext context = make_context_with_two_units(2, 1);
    const SkillDefinition *definition = skill_get_definition(SKILL_POWER_STRIKE);

    EXPECT_EQ(20, skill_calculate_damage(definition, &context.units[0], &context.units[1]));
}

static void test_skill_definition_calculates_magical_damage(void)
{
    BattleContext context = make_context_with_two_units(4, 1);
    const SkillDefinition *definition = skill_get_definition(SKILL_FIREBALL);

    EXPECT_EQ(40, skill_calculate_damage(definition, &context.units[0], &context.units[1]));
}

static void test_skill_definition_calculates_healing(void)
{
    const SkillDefinition *definition = skill_get_definition(SKILL_IRON_SHIELD);

    EXPECT_EQ(35, skill_calculate_healing(definition));
}

static void test_arcane_bolt_uses_base_damage_and_attack_percent(void)
{
    BattleContext context = make_context_with_two_units(8, 1);
    const SkillDefinition *definition = skill_get_definition(SKILL_ARCANE_BOLT);

    EXPECT_EQ(44, skill_calculate_damage(definition, &context.units[0], &context.units[1]));
}

static void test_holy_guard_heals_more_than_iron_shield(void)
{
    const SkillDefinition *definition = skill_get_definition(SKILL_HOLY_GUARD);

    EXPECT_EQ(55, skill_calculate_healing(definition));
    EXPECT_EQ(SKILL_TARGET_LOWEST_HP_ALLY, definition->target_type);
}

static void test_execute_strike_bonus_triggers_on_low_hp(void)
{
    BattleContext context = make_context_with_two_units(10, 1);
    const SkillDefinition *definition = skill_get_definition(SKILL_EXECUTE_STRIKE);

    context.units[1].current_hp = 50;

    EXPECT_EQ(70, skill_calculate_damage(definition, &context.units[0], &context.units[1]));
}

static void test_execute_strike_bonus_does_not_trigger_on_high_hp(void)
{
    BattleContext context = make_context_with_two_units(10, 1);
    const SkillDefinition *definition = skill_get_definition(SKILL_EXECUTE_STRIKE);

    EXPECT_EQ(49, skill_calculate_damage(definition, &context.units[0], &context.units[1]));
}

static void test_fireball_deals_damage_and_resets_mana(void)
{
    BattleContext context = make_context_with_two_units(4, 1);
    int before_hp = context.units[1].current_hp;

    context.units[0].current_mana = context.units[0].max_mana;

    EXPECT_TRUE(skill_cast(&context, 0, 1, NULL));
    EXPECT_EQ(0, context.units[0].current_mana);
    EXPECT_EQ(before_hp - 40, context.units[1].current_hp);
}

static void test_iron_shield_heals_self_and_resets_mana(void)
{
    BattleContext context = make_context_with_two_units(1, 4);

    context.units[0].current_hp = 50;
    context.units[0].current_mana = context.units[0].max_mana;

    EXPECT_TRUE(skill_cast(&context, 0, 1, NULL));
    EXPECT_EQ(0, context.units[0].current_mana);
    EXPECT_EQ(85, context.units[0].current_hp);
    EXPECT_EQ(20, context.units[0].shield);
}

static void test_fireball_applies_burn(void)
{
    BattleContext context = make_context_with_two_units(4, 1);

    context.units[0].current_mana = context.units[0].max_mana;

    EXPECT_TRUE(skill_cast(&context, 0, 1, NULL));
    EXPECT_EQ(6, context.units[1].burn_damage);
    EXPECT_EQ(2, context.units[1].burn_turns);
}

static void test_execute_strike_applies_stun(void)
{
    BattleContext context = make_context_with_two_units(10, 1);

    context.units[0].current_mana = context.units[0].max_mana;

    EXPECT_TRUE(skill_cast(&context, 0, 1, NULL));
    EXPECT_EQ(1, context.units[1].stun_turns);
}

static void test_holy_guard_targets_lowest_hp_ally(void)
{
    BattleContext context = {0};
    BoardPosition caster_position = {7, 3};
    BoardPosition ally_position = {7, 2};
    BoardPosition enemy_position = {6, 3};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(9), BATTLE_SIDE_PLAYER, caster_position));
    battle_add_unit(&context, battle_create_unit_at(2, hero_get_template(1), BATTLE_SIDE_PLAYER, ally_position));
    battle_add_unit(&context, battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, enemy_position));

    context.units[0].current_mana = context.units[0].max_mana;
    context.units[1].current_hp = 40;

    EXPECT_TRUE(skill_cast(&context, 0, 2, NULL));
    EXPECT_EQ(95, context.units[1].current_hp);
    EXPECT_EQ(35, context.units[1].shield);
    EXPECT_EQ(190, context.units[0].current_hp);
}

static void test_fireball_splashes_to_adjacent_enemy(void)
{
    BattleContext context = {0};
    BoardPosition caster_position = {7, 3};
    BoardPosition primary_position = {6, 3};
    BoardPosition splash_position = {6, 4};
    BoardPosition far_position = {4, 4};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(4), BATTLE_SIDE_PLAYER, caster_position));
    battle_add_unit(&context, battle_create_unit_at(101, hero_get_template(1), BATTLE_SIDE_ENEMY, primary_position));
    battle_add_unit(&context, battle_create_unit_at(102, hero_get_template(2), BATTLE_SIDE_ENEMY, splash_position));
    battle_add_unit(&context, battle_create_unit_at(103, hero_get_template(2), BATTLE_SIDE_ENEMY, far_position));

    context.units[0].current_mana = context.units[0].max_mana;

    EXPECT_TRUE(skill_cast(&context, 0, 1, NULL));
    EXPECT_EQ(90, context.units[1].current_hp);
    EXPECT_EQ(85, context.units[2].current_hp);
    EXPECT_EQ(105, context.units[3].current_hp);
}

static void test_skill_does_not_cast_without_full_mana(void)
{
    BattleContext context = make_context_with_two_units(4, 1);
    int before_hp = context.units[1].current_hp;

    context.units[0].current_mana = context.units[0].max_mana - 1;

    EXPECT_TRUE(!skill_cast(&context, 0, 1, NULL));
    EXPECT_EQ(before_hp, context.units[1].current_hp);
}

static void test_battle_attack_gains_mana_and_eventually_casts(void)
{
    BattleContext context = make_context_with_two_units(4, 1);
    int before_hp = context.units[1].current_hp;

    context.units[0].current_mana = context.units[0].max_mana - AUTOCHESS_ATTACK_MANA_GAIN;
    battle_run(&context, NULL);

    EXPECT_TRUE(context.units[1].current_hp < before_hp - context.units[0].attack ||
                !context.units[1].is_alive);
}

int main(void)
{
    test_mana_gain_is_capped();
    test_skill_definitions_are_available();
    test_skill_definition_calculates_physical_damage();
    test_skill_definition_calculates_magical_damage();
    test_skill_definition_calculates_healing();
    test_arcane_bolt_uses_base_damage_and_attack_percent();
    test_holy_guard_heals_more_than_iron_shield();
    test_execute_strike_bonus_triggers_on_low_hp();
    test_execute_strike_bonus_does_not_trigger_on_high_hp();
    test_fireball_deals_damage_and_resets_mana();
    test_iron_shield_heals_self_and_resets_mana();
    test_fireball_applies_burn();
    test_execute_strike_applies_stun();
    test_holy_guard_targets_lowest_hp_ally();
    test_fireball_splashes_to_adjacent_enemy();
    test_skill_does_not_cast_without_full_mana();
    test_battle_attack_gains_mana_and_eventually_casts();

    if (g_failed_tests == 0)
    {
        printf("All skill tests passed.\n");
        return 0;
    }

    printf("%d skill test(s) failed.\n", g_failed_tests);
    return 1;
}
