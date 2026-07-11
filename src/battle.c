#include "battle.h"

#include <stddef.h>

#include "logger.h"

BattleUnit battle_create_unit(int instance_id, const HeroTemplate *hero, BattleSide side)
{
    BattleUnit unit;

    unit.instance_id = instance_id;
    unit.template_id = hero != NULL ? hero->id : 0;
    unit.side = side;
    unit.name = hero != NULL ? hero->name : "未知单位";
    unit.max_hp = hero != NULL ? hero->base_hp : 1;
    unit.current_hp = unit.max_hp;
    unit.attack = hero != NULL ? hero->base_attack : 1;
    unit.is_alive = 1;

    return unit;
}

void battle_add_unit(BattleContext *context, BattleUnit unit)
{
    if (context == NULL)
    {
        return;
    }

    if (context->unit_count >= AUTOCHESS_MAX_BATTLE_UNITS)
    {
        return;
    }

    context->units[context->unit_count] = unit;
    context->unit_count += 1;
}

void battle_apply_damage(BattleUnit *target, int damage)
{
    if (target == NULL || target->is_alive == 0)
    {
        return;
    }

    if (damage < 0)
    {
        damage = 0;
    }

    target->current_hp -= damage;

    if (target->current_hp <= 0)
    {
        target->current_hp = 0;
        target->is_alive = 0;
    }
}

int battle_select_target_lowest_hp(const BattleContext *context, BattleSide attacker_side)
{
    int best_index = -1;

    if (context == NULL)
    {
        return best_index;
    }

    for (int i = 0; i < context->unit_count; ++i)
    {
        const BattleUnit *candidate = &context->units[i];

        if (candidate->side == attacker_side || candidate->is_alive == 0)
        {
            continue;
        }

        if (best_index < 0)
        {
            best_index = i;
            continue;
        }

        const BattleUnit *best = &context->units[best_index];

        if (candidate->current_hp < best->current_hp ||
            (candidate->current_hp == best->current_hp &&
             candidate->instance_id < best->instance_id))
        {
            best_index = i;
        }
    }

    return best_index;
}

BattleResult battle_check_result(const BattleContext *context)
{
    int player_alive = 0;
    int enemy_alive = 0;

    if (context == NULL)
    {
        return BATTLE_RESULT_DRAW;
    }

    for (int i = 0; i < context->unit_count; ++i)
    {
        const BattleUnit *unit = &context->units[i];

        if (unit->is_alive == 0)
        {
            continue;
        }

        if (unit->side == BATTLE_SIDE_PLAYER)
        {
            player_alive = 1;
        }
        else
        {
            enemy_alive = 1;
        }
    }

    if (player_alive && enemy_alive)
    {
        return BATTLE_RESULT_ONGOING;
    }

    if (player_alive)
    {
        return BATTLE_RESULT_PLAYER_WIN;
    }

    if (enemy_alive)
    {
        return BATTLE_RESULT_ENEMY_WIN;
    }

    return BATTLE_RESULT_DRAW;
}

BattleResult battle_run(BattleContext *context, FILE *log_stream)
{
    BattleResult result = BATTLE_RESULT_ONGOING;

    if (context == NULL)
    {
        return BATTLE_RESULT_DRAW;
    }

    logger_battle_start(log_stream);

    for (int round = 1; round <= AUTOCHESS_MAX_BATTLE_ROUNDS; ++round)
    {
        context->current_round = round;
        logger_round_start(log_stream, round);

        for (int i = 0; i < context->unit_count; ++i)
        {
            BattleUnit *attacker = &context->units[i];

            if (attacker->is_alive == 0)
            {
                continue;
            }

            int target_index = battle_select_target_lowest_hp(context, attacker->side);

            if (target_index < 0)
            {
                break;
            }

            BattleUnit *target = &context->units[target_index];
            battle_apply_damage(target, attacker->attack);
            logger_attack(log_stream, attacker->name, target->name, attacker->attack, target->current_hp, target->max_hp);

            if (target->is_alive == 0)
            {
                logger_defeated(log_stream, target->name);
            }

            result = battle_check_result(context);
            if (result != BATTLE_RESULT_ONGOING)
            {
                logger_battle_end(log_stream, battle_result_name(result));
                return result;
            }
        }

        result = battle_check_result(context);
        if (result != BATTLE_RESULT_ONGOING)
        {
            logger_battle_end(log_stream, battle_result_name(result));
            return result;
        }
    }

    logger_battle_end(log_stream, battle_result_name(BATTLE_RESULT_DRAW));
    return BATTLE_RESULT_DRAW;
}

BattleContext battle_create_demo_context(void)
{
    BattleContext context = {0};

    battle_add_unit(&context, battle_create_unit(1, hero_get_template(1), BATTLE_SIDE_PLAYER));
    battle_add_unit(&context, battle_create_unit(2, hero_get_template(2), BATTLE_SIDE_PLAYER));
    battle_add_unit(&context, battle_create_unit(3, hero_get_template(3), BATTLE_SIDE_PLAYER));

    battle_add_unit(&context, battle_create_unit(101, hero_get_template(4), BATTLE_SIDE_ENEMY));
    battle_add_unit(&context, battle_create_unit(102, hero_get_template(5), BATTLE_SIDE_ENEMY));
    battle_add_unit(&context, battle_create_unit(103, hero_get_template(6), BATTLE_SIDE_ENEMY));

    return context;
}

const char *battle_result_name(BattleResult result)
{
    switch (result)
    {
    case BATTLE_RESULT_PLAYER_WIN:
        return "玩家获胜";
    case BATTLE_RESULT_ENEMY_WIN:
        return "敌方获胜";
    case BATTLE_RESULT_DRAW:
        return "平局";
    case BATTLE_RESULT_ONGOING:
    default:
        return "战斗进行中";
    }
}
