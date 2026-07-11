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
            BattleUnit battle_unit = battle_create_unit_at(unit->instance_id, hero, player->side, unit->position);
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
