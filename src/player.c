#include "player.h"

#include "hero.h"

static void player_clear_bench(Player *player)
{
    for (int i = 0; i < AUTOCHESS_BENCH_SIZE; ++i)
    {
        player->bench_slots[i] = -1;
    }
}

static int player_find_empty_bench_slot(const Player *player)
{
    if (player == 0)
    {
        return -1;
    }

    for (int i = 0; i < AUTOCHESS_BENCH_SIZE; ++i)
    {
        if (player->bench_slots[i] < 0)
        {
            return i;
        }
    }

    return -1;
}

static void player_clear_bench_slot_for_unit(Player *player, int unit_index)
{
    if (player == 0)
    {
        return;
    }

    for (int i = 0; i < AUTOCHESS_BENCH_SIZE; ++i)
    {
        if (player->bench_slots[i] == unit_index)
        {
            player->bench_slots[i] = -1;
        }
    }
}

static int player_find_merge_candidates(const Player *player, int template_id, int star, int candidates[3])
{
    int count = 0;

    if (player == 0 || candidates == 0)
    {
        return 0;
    }

    for (int i = 0; i < player->unit_count; ++i)
    {
        const Unit *unit = &player->units[i];

        if (unit->is_active && unit->template_id == template_id && unit->star == star)
        {
            candidates[count] = i;
            count += 1;

            if (count == 3)
            {
                return 1;
            }
        }
    }

    return 0;
}

static int player_is_valid_deploy_position(const Player *player, BoardPosition position)
{
    if (player == 0)
    {
        return 0;
    }

    if (player->side == BATTLE_SIDE_PLAYER)
    {
        return board_is_player_deploy_position(position);
    }

    return board_is_enemy_deploy_position(position);
}

void player_init(Player *player, int id, BattleSide side)
{
    if (player == 0)
    {
        return;
    }

    player->id = id;
    player->side = side;
    player->health = 100;
    player->gold = 10;
    player->level = 3;
    player->experience = 0;
    player->unit_count = 0;
    player_clear_bench(player);
}

PlayerResult player_add_unit_to_bench(Player *player, Unit unit)
{
    int bench_index = player_find_empty_bench_slot(player);

    if (player == 0)
    {
        return PLAYER_ERROR_INVALID_ARGUMENT;
    }

    if (player->unit_count >= AUTOCHESS_MAX_PLAYER_UNITS)
    {
        return PLAYER_ERROR_BENCH_FULL;
    }

    if (bench_index < 0)
    {
        return PLAYER_ERROR_BENCH_FULL;
    }

    unit_place_on_bench(&unit, bench_index);
    player->units[player->unit_count] = unit;
    player->bench_slots[bench_index] = player->unit_count;
    player->unit_count += 1;
    player_try_merge_units(player);

    return PLAYER_OK;
}

PlayerResult player_deploy_from_bench(Player *player, int bench_index, BoardPosition position)
{
    int unit_index = -1;

    if (player == 0)
    {
        return PLAYER_ERROR_INVALID_ARGUMENT;
    }

    if (bench_index < 0 || bench_index >= AUTOCHESS_BENCH_SIZE)
    {
        return PLAYER_ERROR_INVALID_BENCH_SLOT;
    }

    unit_index = player->bench_slots[bench_index];
    if (unit_index < 0)
    {
        return PLAYER_ERROR_EMPTY_BENCH_SLOT;
    }

    if (!player_is_valid_deploy_position(player, position))
    {
        return PLAYER_ERROR_INVALID_DEPLOY_POSITION;
    }

    if (player_is_position_occupied(player, position))
    {
        return PLAYER_ERROR_POSITION_OCCUPIED;
    }

    if (player_count_deployed_units(player) >= player->level)
    {
        return PLAYER_ERROR_DEPLOY_LIMIT_REACHED;
    }

    unit_deploy_to_board(&player->units[unit_index], position);
    player->bench_slots[bench_index] = -1;

    return PLAYER_OK;
}

PlayerResult player_move_deployed_unit(Player *player, BoardPosition from, BoardPosition to)
{
    int unit_index = -1;

    if (player == 0)
    {
        return PLAYER_ERROR_INVALID_ARGUMENT;
    }

    unit_index = player_find_deployed_unit_index(player, from);
    if (unit_index < 0)
    {
        return PLAYER_ERROR_UNIT_NOT_FOUND;
    }

    if (!player_is_valid_deploy_position(player, to))
    {
        return PLAYER_ERROR_INVALID_DEPLOY_POSITION;
    }

    if (player_is_position_occupied(player, to))
    {
        return PLAYER_ERROR_POSITION_OCCUPIED;
    }

    unit_deploy_to_board(&player->units[unit_index], to);
    return PLAYER_OK;
}

PlayerResult player_return_unit_to_bench(Player *player, BoardPosition position)
{
    int unit_index = -1;
    int bench_index = player_find_empty_bench_slot(player);

    if (player == 0)
    {
        return PLAYER_ERROR_INVALID_ARGUMENT;
    }

    if (bench_index < 0)
    {
        return PLAYER_ERROR_BENCH_FULL;
    }

    unit_index = player_find_deployed_unit_index(player, position);
    if (unit_index < 0)
    {
        return PLAYER_ERROR_UNIT_NOT_FOUND;
    }

    unit_place_on_bench(&player->units[unit_index], bench_index);
    player->bench_slots[bench_index] = unit_index;

    return PLAYER_OK;
}

PlayerResult player_sell_unit(Player *player, int unit_index, int *refund)
{
    Unit *unit = 0;
    const HeroTemplate *hero = 0;
    int multiplier = 1;
    int sell_value = 0;

    if (player == 0 || refund == 0)
    {
        return PLAYER_ERROR_INVALID_ARGUMENT;
    }

    *refund = 0;

    if (unit_index < 0 || unit_index >= player->unit_count)
    {
        return PLAYER_ERROR_UNIT_NOT_FOUND;
    }

    unit = &player->units[unit_index];
    if (!unit->is_active)
    {
        return PLAYER_ERROR_UNIT_NOT_FOUND;
    }

    hero = hero_get_template(unit->template_id);
    if (hero == 0)
    {
        return PLAYER_ERROR_UNIT_NOT_FOUND;
    }

    if (unit->star == 2)
    {
        multiplier = 3;
    }
    else if (unit->star >= 3)
    {
        multiplier = 9;
    }

    sell_value = hero->cost * multiplier;
    player_clear_bench_slot_for_unit(player, unit_index);

    unit->is_active = 0;
    unit->location = UNIT_LOCATION_NONE;
    unit->bench_index = -1;
    unit->position.row = -1;
    unit->position.col = -1;

    player->gold += sell_value;
    *refund = sell_value;
    return PLAYER_OK;
}

int player_try_merge_units(Player *player)
{
    int merged_count = 0;
    int did_merge = 1;

    if (player == 0)
    {
        return 0;
    }

    while (did_merge)
    {
        did_merge = 0;

        for (int i = 0; i < player->unit_count; ++i)
        {
            Unit *unit = &player->units[i];
            int candidates[3] = {-1, -1, -1};

            if (!unit->is_active || unit->star >= AUTOCHESS_MAX_STAR)
            {
                continue;
            }

            if (player_find_merge_candidates(player, unit->template_id, unit->star, candidates))
            {
                Unit *kept = &player->units[candidates[0]];
                Unit *removed_a = &player->units[candidates[1]];
                Unit *removed_b = &player->units[candidates[2]];

                kept->star += 1;

                player_clear_bench_slot_for_unit(player, candidates[1]);
                player_clear_bench_slot_for_unit(player, candidates[2]);

                removed_a->is_active = 0;
                removed_a->location = UNIT_LOCATION_NONE;
                removed_a->bench_index = -1;

                removed_b->is_active = 0;
                removed_b->location = UNIT_LOCATION_NONE;
                removed_b->bench_index = -1;

                merged_count += 1;
                did_merge = 1;
                break;
            }
        }
    }

    return merged_count;
}

int player_count_deployed_units(const Player *player)
{
    int count = 0;

    if (player == 0)
    {
        return count;
    }

    for (int i = 0; i < player->unit_count; ++i)
    {
        if (unit_is_deployed(&player->units[i]))
        {
            count += 1;
        }
    }

    return count;
}

int player_count_active_units(const Player *player)
{
    int count = 0;

    if (player == 0)
    {
        return count;
    }

    for (int i = 0; i < player->unit_count; ++i)
    {
        if (player->units[i].is_active)
        {
            count += 1;
        }
    }

    return count;
}

int player_count_units_by_template_and_star(const Player *player, int template_id, int star)
{
    int count = 0;

    if (player == 0)
    {
        return count;
    }

    for (int i = 0; i < player->unit_count; ++i)
    {
        const Unit *unit = &player->units[i];

        if (unit->is_active && unit->template_id == template_id && unit->star == star)
        {
            count += 1;
        }
    }

    return count;
}

int player_is_position_occupied(const Player *player, BoardPosition position)
{
    return player_find_deployed_unit_index(player, position) >= 0;
}

int player_find_deployed_unit_index(const Player *player, BoardPosition position)
{
    if (player == 0)
    {
        return -1;
    }

    for (int i = 0; i < player->unit_count; ++i)
    {
        const Unit *unit = &player->units[i];

        if (unit_is_deployed(unit) && board_positions_equal(unit->position, position))
        {
            return i;
        }
    }

    return -1;
}

void player_add_deployed_units_to_battle(const Player *player, BattleContext *context)
{
    if (player == 0 || context == 0)
    {
        return;
    }

    for (int i = 0; i < player->unit_count; ++i)
    {
        const Unit *unit = &player->units[i];

        if (unit_is_deployed(unit))
        {
            const HeroTemplate *hero = hero_get_template(unit->template_id);
            BattleUnit battle_unit = battle_create_unit_at_star(unit->instance_id, hero, player->side, unit->position, unit->star);
            battle_add_unit(context, battle_unit);
        }
    }
}

const char *player_result_name(PlayerResult result)
{
    switch (result)
    {
    case PLAYER_OK:
        return "成功";
    case PLAYER_ERROR_INVALID_ARGUMENT:
        return "参数无效";
    case PLAYER_ERROR_BENCH_FULL:
        return "备战席已满";
    case PLAYER_ERROR_INVALID_BENCH_SLOT:
        return "备战席位置无效";
    case PLAYER_ERROR_EMPTY_BENCH_SLOT:
        return "备战席位置为空";
    case PLAYER_ERROR_INVALID_DEPLOY_POSITION:
        return "部署位置无效";
    case PLAYER_ERROR_POSITION_OCCUPIED:
        return "目标位置已被占用";
    case PLAYER_ERROR_DEPLOY_LIMIT_REACHED:
        return "上场人数已达等级上限";
    case PLAYER_ERROR_UNIT_NOT_FOUND:
        return "没有找到单位";
    default:
        return "未知结果";
    }
}
