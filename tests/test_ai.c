#include <stdio.h>

#include "ai.h"

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

static void test_scores_higher_cost_hero_higher(void)
{
    int low_score = ai_score_hero_template(hero_get_template(1));
    int high_score = ai_score_hero_template(hero_get_template(6));

    EXPECT_TRUE(high_score > low_score);
}

static void test_choose_shop_slot_picks_best_affordable(void)
{
    Player player;
    Shop shop;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    shop_init(&shop, 1u);
    player.gold = 10;

    shop.slots[0].template_id = 1;
    shop.slots[0].is_available = 1;
    shop.slots[1].template_id = 6;
    shop.slots[1].is_available = 1;

    EXPECT_EQ(1, ai_choose_shop_slot(&shop, &player));
}

static void test_buy_best_affordable_unit_uses_shop_and_bench(void)
{
    Player player;
    Shop shop;
    int next_instance_id = 100;
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    shop_init(&shop, 1u);
    player.gold = 10;

    shop.slots[0].template_id = 1;
    shop.slots[0].is_available = 1;

    EXPECT_TRUE(ai_buy_best_affordable_unit(&player, &shop, &next_instance_id));
    EXPECT_EQ(1, player.unit_count);
    EXPECT_EQ(100, player.units[0].instance_id);
    EXPECT_EQ(101, next_instance_id);
    EXPECT_TRUE(!shop.slots[0].is_available);
}

static void test_ai_deploys_enemy_units_to_enemy_area(void)
{
    Player enemy;
    player_init(&enemy, 2, BATTLE_SIDE_ENEMY);

    player_add_unit_to_bench(&enemy, unit_create(101, 1));
    player_add_unit_to_bench(&enemy, unit_create(102, 6));
    ai_deploy_best_units(&enemy);

    EXPECT_EQ(2, player_count_deployed_units(&enemy));
    EXPECT_TRUE(board_is_enemy_deploy_position(enemy.units[0].position) ||
                board_is_enemy_deploy_position(enemy.units[1].position));
}

static void test_ai_respects_deploy_limit(void)
{
    Player enemy;
    player_init(&enemy, 2, BATTLE_SIDE_ENEMY);
    enemy.level = 2;

    player_add_unit_to_bench(&enemy, unit_create(101, 1));
    player_add_unit_to_bench(&enemy, unit_create(102, 2));
    player_add_unit_to_bench(&enemy, unit_create(103, 3));
    ai_deploy_best_units(&enemy);

    EXPECT_EQ(2, player_count_deployed_units(&enemy));
}

static void test_ai_run_preparation_buys_and_deploys(void)
{
    Player enemy;
    Shop shop;
    int next_instance_id = 100;
    player_init(&enemy, 2, BATTLE_SIDE_ENEMY);
    shop_init(&shop, 42u);

    ai_run_preparation(&enemy, &shop, &next_instance_id);

    EXPECT_TRUE(player_count_active_units(&enemy) > 0);
    EXPECT_TRUE(player_count_deployed_units(&enemy) > 0);
}

int main(void)
{
    test_scores_higher_cost_hero_higher();
    test_choose_shop_slot_picks_best_affordable();
    test_buy_best_affordable_unit_uses_shop_and_bench();
    test_ai_deploys_enemy_units_to_enemy_area();
    test_ai_respects_deploy_limit();
    test_ai_run_preparation_buys_and_deploys();

    if (g_failed_tests == 0)
    {
        printf("All ai tests passed.\n");
        return 0;
    }

    printf("%d ai test(s) failed.\n", g_failed_tests);
    return 1;
}
