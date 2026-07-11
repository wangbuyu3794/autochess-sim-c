#include <stdio.h>

#include "hero.h"
#include "shop.h"

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

static void test_refresh_fills_shop(void)
{
    Shop shop;
    shop_init(&shop, 123u);

    shop_refresh(&shop, 3);

    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        EXPECT_TRUE(shop.slots[i].is_available);
        EXPECT_TRUE(shop.slots[i].template_id > 0);
    }
}

static void test_refresh_for_player_costs_gold(void)
{
    Shop shop;
    Player player;
    shop_init(&shop, 123u);
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    player.gold = 10;

    EXPECT_EQ(SHOP_OK, shop_refresh_for_player(&shop, &player));
    EXPECT_EQ(8, player.gold);
}

static void test_buy_slot_adds_unit_to_bench_and_spends_gold(void)
{
    Shop shop;
    Player player;
    const HeroTemplate *hero = 0;
    shop_init(&shop, 123u);
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    player.gold = 10;

    shop_refresh(&shop, player.level);
    hero = hero_get_template(shop.slots[0].template_id);

    EXPECT_EQ(SHOP_OK, shop_buy_slot(&shop, &player, 0, 1001));
    EXPECT_EQ(1, player.unit_count);
    EXPECT_EQ(1001, player.units[0].instance_id);
    EXPECT_EQ(hero->id, player.units[0].template_id);
    EXPECT_EQ(10 - hero->cost, player.gold);
    EXPECT_TRUE(!shop.slots[0].is_available);
}

static void test_buy_rejects_empty_slot(void)
{
    Shop shop;
    Player player;
    shop_init(&shop, 123u);
    player_init(&player, 1, BATTLE_SIDE_PLAYER);

    EXPECT_EQ(SHOP_ERROR_EMPTY_SLOT, shop_buy_slot(&shop, &player, 0, 1001));
}

static void test_buy_rejects_not_enough_gold(void)
{
    Shop shop;
    Player player;
    shop_init(&shop, 123u);
    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    player.gold = 0;
    shop_refresh(&shop, player.level);

    EXPECT_EQ(SHOP_ERROR_NOT_ENOUGH_GOLD, shop_buy_slot(&shop, &player, 0, 1001));
    EXPECT_EQ(0, player.unit_count);
}

int main(void)
{
    test_refresh_fills_shop();
    test_refresh_for_player_costs_gold();
    test_buy_slot_adds_unit_to_bench_and_spends_gold();
    test_buy_rejects_empty_slot();
    test_buy_rejects_not_enough_gold();

    if (g_failed_tests == 0)
    {
        printf("All shop tests passed.\n");
        return 0;
    }

    printf("%d shop test(s) failed.\n", g_failed_tests);
    return 1;
}
