#include "ai.h"

#include "economy.h"

static const BoardPosition PLAYER_DEPLOY_POSITIONS[] = {
    {6, 3},
    {7, 2},
    {7, 4},
    {6, 2},
    {6, 4},
    {7, 3},
};

static const BoardPosition ENEMY_DEPLOY_POSITIONS[] = {
    {1, 3},
    {0, 2},
    {0, 4},
    {1, 2},
    {1, 4},
    {0, 3},
};

static int ai_position_count(void)
{
    return (int)(sizeof(PLAYER_DEPLOY_POSITIONS) / sizeof(PLAYER_DEPLOY_POSITIONS[0]));
}

static BoardPosition ai_get_deploy_position(BattleSide side, int index)
{
    if (index < 0 || index >= ai_position_count())
    {
        BoardPosition invalid = {-1, -1};
        return invalid;
    }

    return side == BATTLE_SIDE_PLAYER ? PLAYER_DEPLOY_POSITIONS[index] : ENEMY_DEPLOY_POSITIONS[index];
}

int ai_score_hero_template(const HeroTemplate *hero)
{
    if (hero == 0)
    {
        return -1;
    }

    return hero->cost * 100 +
           hero->base_attack * 3 +
           hero->base_hp +
           hero->armor * 2 +
           hero->magic_resist * 2 +
           hero->crit_chance +
           hero->crit_damage / 10 +
           hero->attack_range * 10;
}

int ai_score_unit(const Unit *unit)
{
    const HeroTemplate *hero = 0;

    if (unit == 0 || !unit->is_active)
    {
        return -1;
    }

    hero = hero_get_template(unit->template_id);
    return ai_score_hero_template(hero) + unit->star * 200;
}

int ai_choose_shop_slot(const Shop *shop, const Player *player)
{
    int best_slot = -1;
    int best_score = -1;

    if (shop == 0 || player == 0)
    {
        return -1;
    }

    for (int i = 0; i < AUTOCHESS_SHOP_SIZE; ++i)
    {
        const HeroTemplate *hero = 0;
        int score = 0;

        if (!shop->slots[i].is_available)
        {
            continue;
        }

        hero = hero_get_template(shop->slots[i].template_id);
        if (hero == 0 || !economy_can_afford(player, hero->cost))
        {
            continue;
        }

        score = ai_score_hero_template(hero);
        if (score > best_score)
        {
            best_score = score;
            best_slot = i;
        }
    }

    return best_slot;
}

int ai_buy_best_affordable_unit(Player *player, Shop *shop, int *next_instance_id)
{
    int slot = -1;

    if (player == 0 || shop == 0 || next_instance_id == 0)
    {
        return 0;
    }

    slot = ai_choose_shop_slot(shop, player);
    if (slot < 0)
    {
        return 0;
    }

    if (shop_buy_slot(shop, player, slot, *next_instance_id) != SHOP_OK)
    {
        return 0;
    }

    *next_instance_id += 1;
    return 1;
}

void ai_deploy_best_units(Player *player)
{
    if (player == 0)
    {
        return;
    }

    for (int deploy_index = player_count_deployed_units(player);
         deploy_index < player->level && deploy_index < ai_position_count();
         ++deploy_index)
    {
        int best_bench_slot = -1;
        int best_score = -1;
        BoardPosition position = ai_get_deploy_position(player->side, deploy_index);

        for (int bench_index = 0; bench_index < AUTOCHESS_BENCH_SIZE; ++bench_index)
        {
            int unit_index = player->bench_slots[bench_index];
            int score = -1;

            if (unit_index < 0)
            {
                continue;
            }

            score = ai_score_unit(&player->units[unit_index]);
            if (score > best_score)
            {
                best_score = score;
                best_bench_slot = bench_index;
            }
        }

        if (best_bench_slot < 0)
        {
            return;
        }

        if (player_deploy_from_bench(player, best_bench_slot, position) != PLAYER_OK)
        {
            return;
        }
    }
}

void ai_run_preparation(Player *player, Shop *shop, int *next_instance_id)
{
    if (player == 0 || shop == 0 || next_instance_id == 0)
    {
        return;
    }

    economy_apply_round_income(player);
    shop_refresh_for_player(shop, player);

    while (ai_buy_best_affordable_unit(player, shop, next_instance_id))
    {
    }

    ai_deploy_best_units(player);
}
