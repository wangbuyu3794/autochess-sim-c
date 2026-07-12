#include "shop.h"

#include "economy.h"
#include "hero.h"

static unsigned int shop_next_random(Shop *shop)
{
    shop->rng_state = shop->rng_state * 1103515245u + 12345u;
    return (shop->rng_state / 65536u) % 32768u;
}

static int shop_pick_cost(Shop *shop, int player_level)
{
    unsigned int roll = shop_next_random(shop) % 100u;
    int cumulative = 0;

    for (int cost = 1; cost <= 4; ++cost)
    {
        cumulative += shop_get_cost_probability(player_level, cost);
        if (roll < (unsigned int)cumulative)
        {
            return cost;
        }
    }

    return 1;
}

static int shop_pick_template_id_by_cost(Shop *shop, int cost)
{
    const HeroTemplate *templates = 0;
    size_t count = 0;
    int matching_count = 0;
    int selected_index = 0;

    templates = hero_get_templates(&count);

    for (size_t i = 0; i < count; ++i)
    {
        if (templates[i].cost == cost)
        {
            matching_count += 1;
        }
    }

    if (matching_count <= 0)
    {
        const HeroTemplate *fallback = hero_get_first_template_by_cost(1);
        return fallback != 0 ? fallback->id : 0;
    }

    selected_index = (int)(shop_next_random(shop) % (unsigned int)matching_count);

    for (size_t i = 0; i < count; ++i)
    {
        if (templates[i].cost == cost)
        {
            if (selected_index == 0)
            {
                return templates[i].id;
            }

            selected_index -= 1;
        }
    }

    return 0;
}

int shop_get_cost_probability(int player_level, int cost)
{
    static const int odds_by_level[][4] = {
        {70, 30, 0, 0},
        {55, 35, 10, 0},
        {40, 35, 20, 5},
        {25, 35, 30, 10},
        {15, 30, 35, 20},
        {10, 25, 35, 30},
    };
    int level_index = 0;

    if (player_level <= 3)
    {
        level_index = 0;
    }
    else if (player_level == 4)
    {
        level_index = 1;
    }
    else if (player_level == 5)
    {
        level_index = 2;
    }
    else if (player_level == 6)
    {
        level_index = 3;
    }
    else if (player_level == 7)
    {
        level_index = 4;
    }
    else
    {
        level_index = 5;
    }

    if (cost < 1 || cost > 4)
    {
        return 0;
    }

    return odds_by_level[level_index][cost - 1];
}

void shop_init(Shop *shop, unsigned int seed)
{
    if (shop == 0)
    {
        return;
    }

    shop->rng_state = seed == 0u ? 1u : seed;
    shop->is_locked = 0;

    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        shop->slots[i].template_id = 0;
        shop->slots[i].is_available = 0;
    }
}

void shop_refresh(Shop *shop, int player_level)
{
    if (shop == 0)
    {
        return;
    }

    shop->is_locked = 0;

    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        int cost = shop_pick_cost(shop, player_level);
        shop->slots[i].template_id = shop_pick_template_id_by_cost(shop, cost);
        shop->slots[i].is_available = shop->slots[i].template_id != 0;
    }
}

int shop_prepare_round(Shop *shop, int player_level)
{
    if (shop == 0)
    {
        return 0;
    }

    if (shop->is_locked)
    {
        shop->is_locked = 0;
        return 0;
    }

    shop_refresh(shop, player_level);
    return 1;
}

void shop_set_locked(Shop *shop, int is_locked)
{
    if (shop == 0)
    {
        return;
    }

    shop->is_locked = is_locked ? 1 : 0;
}

ShopResult shop_refresh_for_player(Shop *shop, Player *player)
{
    if (shop == 0 || player == 0)
    {
        return SHOP_ERROR_INVALID_ARGUMENT;
    }

    if (!economy_spend_gold(player, AUTOCHESS_REFRESH_COST))
    {
        return SHOP_ERROR_NOT_ENOUGH_GOLD;
    }

    shop_refresh(shop, player->level);
    return SHOP_OK;
}

ShopResult shop_buy_slot(Shop *shop, Player *player, int slot_index, int instance_id)
{
    const HeroTemplate *hero = 0;
    PlayerResult player_result = PLAYER_OK;

    if (shop == 0 || player == 0)
    {
        return SHOP_ERROR_INVALID_ARGUMENT;
    }

    if (slot_index < 0 || slot_index >= AUTOCHESS_SHOP_SIZE)
    {
        return SHOP_ERROR_INVALID_SLOT;
    }

    if (!shop->slots[slot_index].is_available)
    {
        return SHOP_ERROR_EMPTY_SLOT;
    }

    hero = hero_get_template(shop->slots[slot_index].template_id);
    if (hero == 0)
    {
        return SHOP_ERROR_NO_TEMPLATE;
    }

    if (!economy_can_afford(player, hero->cost))
    {
        return SHOP_ERROR_NOT_ENOUGH_GOLD;
    }

    player_result = player_add_unit_to_bench(player, unit_create(instance_id, hero->id));
    if (player_result != PLAYER_OK)
    {
        return SHOP_ERROR_BENCH_FULL;
    }

    economy_spend_gold(player, hero->cost);
    shop->slots[slot_index].is_available = 0;
    shop->slots[slot_index].template_id = 0;

    return SHOP_OK;
}

const char *shop_result_name(ShopResult result)
{
    switch (result)
    {
    case SHOP_OK:
        return "成功";
    case SHOP_ERROR_INVALID_ARGUMENT:
        return "参数无效";
    case SHOP_ERROR_INVALID_SLOT:
        return "商店位置无效";
    case SHOP_ERROR_EMPTY_SLOT:
        return "商店位置为空";
    case SHOP_ERROR_NOT_ENOUGH_GOLD:
        return "金币不足";
    case SHOP_ERROR_BENCH_FULL:
        return "备战席已满";
    case SHOP_ERROR_NO_TEMPLATE:
        return "英雄模板不存在";
    default:
        return "未知结果";
    }
}
