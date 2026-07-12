#include <stdio.h>

#include "battle.h"
#include "player.h"
#include "trait.h"

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

static void test_trait_summary_counts_traits(void)
{
    TraitSummary summary;
    trait_summary_init(&summary);

    trait_summary_add(&summary, TRAIT_GUARDIAN);
    trait_summary_add(&summary, TRAIT_GUARDIAN);
    trait_summary_add(&summary, TRAIT_CITY);

    EXPECT_EQ(2, trait_summary_get_count(&summary, TRAIT_GUARDIAN));
    EXPECT_EQ(1, trait_summary_get_count(&summary, TRAIT_CITY));
    EXPECT_EQ(0, trait_summary_get_count(&summary, TRAIT_MAGE));
}

static void test_trait_thresholds(void)
{
    TraitSummary summary;
    trait_summary_init(&summary);

    EXPECT_EQ(0, trait_guardian_bonus_hp(&summary));
    trait_summary_add(&summary, TRAIT_GUARDIAN);
    trait_summary_add(&summary, TRAIT_GUARDIAN);
    EXPECT_EQ(25, trait_guardian_bonus_hp(&summary));

    trait_summary_init(&summary);
    trait_summary_add(&summary, TRAIT_BLADEMASTER);
    trait_summary_add(&summary, TRAIT_BLADEMASTER);
    EXPECT_EQ(15, trait_blademaster_attack_percent(&summary));

    trait_summary_init(&summary);
    trait_summary_add(&summary, TRAIT_RANGER);
    trait_summary_add(&summary, TRAIT_RANGER);
    EXPECT_EQ(1, trait_ranger_bonus_range(&summary));

    trait_summary_init(&summary);
    trait_summary_add(&summary, TRAIT_MAGE);
    trait_summary_add(&summary, TRAIT_MAGE);
    EXPECT_EQ(20, trait_mage_bonus_initial_mana(&summary));

    trait_summary_init(&summary);
    trait_summary_add(&summary, TRAIT_ELEMENT);
    trait_summary_add(&summary, TRAIT_ELEMENT);
    EXPECT_EQ(20, trait_element_bonus_hp(&summary));

    EXPECT_EQ(0, trait_current_threshold(1));
    EXPECT_EQ(2, trait_next_threshold(1));
    EXPECT_EQ(2, trait_current_threshold(2));
    EXPECT_EQ(4, trait_next_threshold(2));
    EXPECT_EQ(4, trait_current_threshold(4));
    EXPECT_EQ(0, trait_next_threshold(4));
}

static void test_guardian_bonus_applies_to_guardians_only(void)
{
    BattleContext context = {0};
    BoardPosition p0 = {7, 2};
    BoardPosition p1 = {7, 3};
    BoardPosition p2 = {7, 4};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(2, hero_get_template(6), BATTLE_SIDE_PLAYER, p1));
    battle_add_unit(&context, battle_create_unit_at(3, hero_get_template(2), BATTLE_SIDE_PLAYER, p2));

    battle_apply_trait_summary(&context);

    EXPECT_EQ(155, context.units[0].max_hp);
    EXPECT_EQ(165, context.units[1].max_hp);
    EXPECT_EQ(105, context.units[2].max_hp);
}

static void test_blademaster_bonus_applies_to_blademasters_only(void)
{
    BattleContext context = {0};
    BoardPosition p0 = {7, 2};
    BoardPosition p1 = {7, 3};
    BoardPosition p2 = {7, 4};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(2), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(2, hero_get_template(5), BATTLE_SIDE_PLAYER, p1));
    battle_add_unit(&context, battle_create_unit_at(3, hero_get_template(1), BATTLE_SIDE_PLAYER, p2));

    battle_apply_trait_summary(&context);

    EXPECT_EQ(27, context.units[0].attack);
    EXPECT_EQ(31, context.units[1].attack);
    EXPECT_EQ(16, context.units[2].attack);
}

static void test_trait_bonus_does_not_cross_sides(void)
{
    BattleContext context = {0};
    BoardPosition p0 = {7, 2};
    BoardPosition p1 = {0, 2};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(101, hero_get_template(6), BATTLE_SIDE_ENEMY, p1));

    battle_apply_trait_summary(&context);

    EXPECT_EQ(130, context.units[0].max_hp);
    EXPECT_EQ(140, context.units[1].max_hp);
}

static void test_ranger_bonus_applies_range(void)
{
    BattleContext context = {0};
    BoardPosition p0 = {7, 2};
    BoardPosition p1 = {7, 3};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(3), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(2, hero_get_template(7), BATTLE_SIDE_PLAYER, p1));

    battle_apply_trait_summary(&context);

    EXPECT_EQ(4, context.units[0].attack_range);
    EXPECT_EQ(4, context.units[1].attack_range);
}

static void test_mage_bonus_applies_initial_mana(void)
{
    BattleContext context = {0};
    BoardPosition p0 = {7, 2};
    BoardPosition p1 = {7, 3};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(4), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(2, hero_get_template(8), BATTLE_SIDE_PLAYER, p1));

    battle_apply_trait_summary(&context);

    EXPECT_EQ(20, context.units[0].current_mana);
    EXPECT_EQ(40, context.units[1].current_mana);
}

static void test_element_bonus_applies_hp(void)
{
    BattleContext context = {0};
    BoardPosition p0 = {7, 2};
    BoardPosition p1 = {7, 3};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(6), BATTLE_SIDE_PLAYER, p0));
    battle_add_unit(&context, battle_create_unit_at(2, hero_get_template(7), BATTLE_SIDE_PLAYER, p1));

    battle_apply_trait_summary(&context);

    EXPECT_EQ(160, context.units[0].max_hp);
    EXPECT_EQ(135, context.units[1].max_hp);
}

int main(void)
{
    test_trait_summary_counts_traits();
    test_trait_thresholds();
    test_guardian_bonus_applies_to_guardians_only();
    test_blademaster_bonus_applies_to_blademasters_only();
    test_trait_bonus_does_not_cross_sides();
    test_ranger_bonus_applies_range();
    test_mage_bonus_applies_initial_mana();
    test_element_bonus_applies_hp();

    if (g_failed_tests == 0)
    {
        printf("All trait tests passed.\n");
        return 0;
    }

    printf("%d trait test(s) failed.\n", g_failed_tests);
    return 1;
}
