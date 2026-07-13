#include "battle.h"

#include <stddef.h>

#include "logger.h"
#include "skill.h"
#include "trait.h"

BattleUnit battle_create_unit(int instance_id, const HeroTemplate *hero, BattleSide side)
{
    BoardPosition default_position = {0, 0};
    return battle_create_unit_at(instance_id, hero, side, default_position);
}

BattleUnit battle_create_unit_at(int instance_id, const HeroTemplate *hero, BattleSide side, BoardPosition position)
{
    return battle_create_unit_at_star(instance_id, hero, side, position, 1);
}

BattleUnit battle_create_unit_at_star(int instance_id, const HeroTemplate *hero, BattleSide side, BoardPosition position, int star)
{
    BattleUnit unit;
    int hp_percent = 100;
    int attack_percent = 100;

    if (star == 2)
    {
        hp_percent = 180;
        attack_percent = 170;
    }
    else if (star >= 3)
    {
        hp_percent = 320;
        attack_percent = 300;
    }

    unit.instance_id = instance_id;
    unit.template_id = hero != NULL ? hero->id : 0;
    unit.side = side;
    unit.name = hero != NULL ? hero->name : "未知单位";
    unit.max_hp = ((hero != NULL ? hero->base_hp : 1) * hp_percent) / 100;
    unit.current_hp = unit.max_hp;
    unit.attack = ((hero != NULL ? hero->base_attack : 1) * attack_percent) / 100;
    unit.armor = hero != NULL ? hero->armor : 0;
    unit.magic_resist = hero != NULL ? hero->magic_resist : 0;
    unit.crit_chance = hero != NULL ? hero->crit_chance : 0;
    unit.crit_damage = hero != NULL ? hero->crit_damage : 150;
    unit.attack_range = hero != NULL ? hero->attack_range : 1;
    unit.current_mana = hero != NULL ? hero->initial_mana : 0;
    unit.max_mana = hero != NULL ? hero->max_mana : 0;
    unit.shield = 0;
    unit.burn_damage = 0;
    unit.burn_turns = 0;
    unit.stun_turns = 0;
    unit.equipment_id = EQUIPMENT_NONE;
    unit.skill_id = hero != NULL ? hero->skill_id : SKILL_NONE;
    unit.position = position;
    unit.is_alive = 1;
    unit.class_trait = hero != NULL ? hero->class_trait : TRAIT_NONE;
    unit.origin_trait = hero != NULL ? hero->origin_trait : TRAIT_NONE;

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

void battle_apply_trait_summary(BattleContext *context)
{
    TraitSummary player_summary;
    TraitSummary enemy_summary;

    if (context == NULL)
    {
        return;
    }

    trait_summary_init(&player_summary);
    trait_summary_init(&enemy_summary);

    for (int i = 0; i < context->unit_count; ++i)
    {
        const BattleUnit *unit = &context->units[i];
        TraitSummary *summary = unit->side == BATTLE_SIDE_PLAYER ? &player_summary : &enemy_summary;

        trait_summary_add(summary, unit->class_trait);
        trait_summary_add(summary, unit->origin_trait);
    }

    for (int i = 0; i < context->unit_count; ++i)
    {
        BattleUnit *unit = &context->units[i];
        const TraitSummary *summary = unit->side == BATTLE_SIDE_PLAYER ? &player_summary : &enemy_summary;
        int guardian_hp = trait_guardian_bonus_hp(summary);
        int blademaster_attack_percent = trait_blademaster_attack_percent(summary);
        int ranger_range = trait_ranger_bonus_range(summary);
        int mage_initial_mana = trait_mage_bonus_initial_mana(summary);
        int city_initial_mana = trait_city_bonus_initial_mana(summary);
        int forest_hp = trait_forest_bonus_hp(summary);
        int element_hp = trait_element_bonus_hp(summary);
        int shadow_attack_percent = trait_shadow_attack_percent(summary);

        if (unit->class_trait == TRAIT_GUARDIAN && guardian_hp > 0)
        {
            unit->max_hp += guardian_hp;
            unit->current_hp += guardian_hp;
        }

        if (unit->class_trait == TRAIT_BLADEMASTER && blademaster_attack_percent > 0)
        {
            unit->attack += (unit->attack * blademaster_attack_percent) / 100;
        }

        if (unit->class_trait == TRAIT_RANGER && ranger_range > 0)
        {
            unit->attack_range += ranger_range;
        }

        if (unit->class_trait == TRAIT_MAGE && mage_initial_mana > 0)
        {
            battle_gain_mana(unit, mage_initial_mana);
        }

        if (unit->origin_trait == TRAIT_CITY && city_initial_mana > 0)
        {
            battle_gain_mana(unit, city_initial_mana);
        }

        if (unit->origin_trait == TRAIT_FOREST && forest_hp > 0)
        {
            unit->max_hp += forest_hp;
            unit->current_hp += forest_hp;
        }

        if (unit->origin_trait == TRAIT_ELEMENT && element_hp > 0)
        {
            unit->max_hp += element_hp;
            unit->current_hp += element_hp;
        }

        if (unit->origin_trait == TRAIT_SHADOW && shadow_attack_percent > 0)
        {
            unit->attack += (unit->attack * shadow_attack_percent) / 100;
        }
    }
}

void battle_apply_damage(BattleUnit *target, int damage)
{
    int absorbed = 0;

    if (target == NULL || target->is_alive == 0)
    {
        return;
    }

    if (damage < 0)
    {
        damage = 0;
    }

    if (target->shield > 0 && damage > 0)
    {
        absorbed = target->shield < damage ? target->shield : damage;
        target->shield -= absorbed;
        damage -= absorbed;
    }

    if (damage <= 0)
    {
        return;
    }

    target->current_hp -= damage;

    if (target->current_hp <= 0)
    {
        target->current_hp = 0;
        target->is_alive = 0;
    }
}

void battle_add_shield(BattleUnit *unit, int amount)
{
    if (unit == NULL || unit->is_alive == 0 || amount <= 0)
    {
        return;
    }

    unit->shield += amount;
}

void battle_apply_burn(BattleUnit *unit, int damage, int turns)
{
    if (unit == NULL || unit->is_alive == 0 || damage <= 0 || turns <= 0)
    {
        return;
    }

    unit->burn_damage = damage;
    if (turns > unit->burn_turns)
    {
        unit->burn_turns = turns;
    }
}

void battle_apply_stun(BattleUnit *unit, int turns)
{
    if (unit == NULL || unit->is_alive == 0 || turns <= 0)
    {
        return;
    }

    if (turns > unit->stun_turns)
    {
        unit->stun_turns = turns;
    }
}

void battle_apply_equipment(BattleUnit *unit, EquipmentId equipment_id)
{
    const EquipmentTemplate *equipment = equipment_get_template(equipment_id);

    if (unit == NULL || equipment == NULL)
    {
        return;
    }

    unit->equipment_id = equipment_id;
    unit->max_hp += equipment->bonus_hp;
    unit->current_hp += equipment->bonus_hp;
    unit->attack += equipment->bonus_attack;
    unit->armor += equipment->bonus_armor;
    unit->magic_resist += equipment->bonus_magic_resist;
    unit->crit_chance += equipment->bonus_crit_chance;
    unit->crit_damage += equipment->bonus_crit_damage;
    battle_gain_mana(unit, equipment->bonus_initial_mana);

    if (unit->crit_chance > 100)
    {
        unit->crit_chance = 100;
    }
}

int battle_process_status_start(BattleUnit *unit, FILE *log_stream)
{
    if (unit == NULL || unit->is_alive == 0)
    {
        return 0;
    }

    if (unit->burn_turns > 0 && unit->burn_damage > 0)
    {
        unit->burn_turns -= 1;
        battle_apply_damage(unit, unit->burn_damage);
        logger_burn(log_stream, unit->name, unit->burn_damage, unit->burn_turns, unit->current_hp, unit->max_hp);
        if (unit->burn_turns == 0)
        {
            unit->burn_damage = 0;
        }
        if (unit->is_alive == 0)
        {
            logger_defeated(log_stream, unit->name);
            return 0;
        }
    }

    if (unit->stun_turns > 0)
    {
        unit->stun_turns -= 1;
        logger_stunned(log_stream, unit->name, unit->stun_turns);
        return 0;
    }

    return 1;
}

int battle_calculate_mitigated_damage(int raw_damage, int resistance)
{
    int damage = 0;

    if (raw_damage <= 0)
    {
        return 0;
    }

    if (resistance < 0)
    {
        resistance = 0;
    }

    damage = (raw_damage * 100) / (100 + resistance);
    if (damage < 1)
    {
        damage = 1;
    }

    return damage;
}

int battle_is_critical_hit(const BattleUnit *attacker, const BattleUnit *target, int round)
{
    int roll = 0;

    if (attacker == NULL || target == NULL || attacker->crit_chance <= 0)
    {
        return 0;
    }

    if (attacker->crit_chance >= 100)
    {
        return 1;
    }

    if (round < 0)
    {
        round = 0;
    }

    roll = (attacker->instance_id * 31 + target->instance_id * 17 + round * 13) % 100;
    return roll < attacker->crit_chance;
}

int battle_calculate_attack_damage(const BattleUnit *attacker, const BattleUnit *target, int round)
{
    int raw_damage = 0;

    if (attacker == NULL || target == NULL || attacker->attack <= 0)
    {
        return 0;
    }

    raw_damage = attacker->attack;
    if (battle_is_critical_hit(attacker, target, round))
    {
        int crit_damage = attacker->crit_damage > 0 ? attacker->crit_damage : 150;
        raw_damage = (raw_damage * crit_damage) / 100;
    }

    return battle_calculate_mitigated_damage(raw_damage, target->armor);
}

int battle_calculate_spell_damage(int raw_damage, const BattleUnit *target)
{
    if (target == NULL)
    {
        return battle_calculate_mitigated_damage(raw_damage, 0);
    }

    return battle_calculate_mitigated_damage(raw_damage, target->magic_resist);
}

void battle_gain_mana(BattleUnit *unit, int amount)
{
    if (unit == NULL || unit->is_alive == 0 || unit->max_mana <= 0 || amount <= 0)
    {
        return;
    }

    unit->current_mana += amount;
    if (unit->current_mana > unit->max_mana)
    {
        unit->current_mana = unit->max_mana;
    }
}

static int try_step(BattleContext *context, int mover_index, BoardPosition next_position)
{
    if (context == NULL || mover_index < 0 || mover_index >= context->unit_count)
    {
        return 0;
    }

    if (!board_is_position_valid(next_position) ||
        battle_is_position_occupied(context, next_position))
    {
        return 0;
    }

    context->units[mover_index].position = next_position;
    return 1;
}

int battle_is_position_occupied(const BattleContext *context, BoardPosition position)
{
    if (context == NULL)
    {
        return 0;
    }

    for (int i = 0; i < context->unit_count; ++i)
    {
        const BattleUnit *unit = &context->units[i];

        if (unit->is_alive && board_positions_equal(unit->position, position))
        {
            return 1;
        }
    }

    return 0;
}

int battle_is_target_in_range(const BattleUnit *attacker, const BattleUnit *target)
{
    if (attacker == NULL || target == NULL)
    {
        return 0;
    }

    return board_manhattan_distance(attacker->position, target->position) <= attacker->attack_range;
}

int battle_select_target_nearest(const BattleContext *context, int attacker_index)
{
    int best_index = -1;

    if (context == NULL || attacker_index < 0 || attacker_index >= context->unit_count)
    {
        return best_index;
    }

    const BattleUnit *attacker = &context->units[attacker_index];

    for (int i = 0; i < context->unit_count; ++i)
    {
        const BattleUnit *candidate = &context->units[i];

        if (candidate->side == attacker->side || candidate->is_alive == 0)
        {
            continue;
        }

        if (best_index < 0)
        {
            best_index = i;
            continue;
        }

        const BattleUnit *best = &context->units[best_index];
        int candidate_distance = board_manhattan_distance(attacker->position, candidate->position);
        int best_distance = board_manhattan_distance(attacker->position, best->position);

        if (candidate_distance < best_distance ||
            (candidate_distance == best_distance && candidate->current_hp < best->current_hp) ||
            (candidate_distance == best_distance &&
             candidate->current_hp == best->current_hp &&
             candidate->instance_id < best->instance_id))
        {
            best_index = i;
        }
    }

    return best_index;
}

int battle_try_move_toward(BattleContext *context, int mover_index, BoardPosition target_position)
{
    if (context == NULL || mover_index < 0 || mover_index >= context->unit_count)
    {
        return 0;
    }

    BattleUnit *mover = &context->units[mover_index];
    BoardPosition current = mover->position;
    BoardPosition next = current;

    if (target_position.col != current.col)
    {
        next.col += target_position.col > current.col ? 1 : -1;
        if (try_step(context, mover_index, next))
        {
            return 1;
        }
    }

    next = current;
    if (target_position.row != current.row)
    {
        next.row += target_position.row > current.row ? 1 : -1;
        if (try_step(context, mover_index, next))
        {
            return 1;
        }
    }

    return 0;
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

            if (!battle_process_status_start(attacker, log_stream))
            {
                result = battle_check_result(context);
                if (result != BATTLE_RESULT_ONGOING)
                {
                    logger_battle_end(log_stream, battle_result_name(result));
                    return result;
                }
                continue;
            }

            int target_index = battle_select_target_nearest(context, i);

            if (target_index < 0)
            {
                break;
            }

            BattleUnit *target = &context->units[target_index];
            if (!battle_is_target_in_range(attacker, target))
            {
                BoardPosition old_position = attacker->position;
                int moved = battle_try_move_toward(context, i, target->position);

                if (moved)
                {
                    logger_move(log_stream,
                                attacker->name,
                                old_position.row,
                                old_position.col,
                                attacker->position.row,
                                attacker->position.col);
                }
                else
                {
                    logger_wait(log_stream, attacker->name);
                }

                continue;
            }

            int is_critical = battle_is_critical_hit(attacker, target, context->current_round);
            int attack_damage = battle_calculate_attack_damage(attacker, target, context->current_round);
            battle_apply_damage(target, attack_damage);
            logger_attack_detail(log_stream, attacker->name, target->name, attack_damage, target->current_hp, target->max_hp, is_critical);
            battle_gain_mana(attacker, AUTOCHESS_ATTACK_MANA_GAIN);

            if (target->is_alive == 0)
            {
                logger_defeated(log_stream, target->name);
            }

            if (attacker->is_alive &&
                attacker->skill_id != SKILL_NONE &&
                attacker->max_mana > 0 &&
                attacker->current_mana >= attacker->max_mana)
            {
                skill_cast(context, i, target_index, log_stream);
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

    BoardPosition player_front = {6, 3};
    BoardPosition player_left = {7, 2};
    BoardPosition player_back = {7, 4};
    BoardPosition enemy_front = {1, 3};
    BoardPosition enemy_left = {0, 2};
    BoardPosition enemy_back = {0, 4};

    battle_add_unit(&context, battle_create_unit_at(1, hero_get_template(1), BATTLE_SIDE_PLAYER, player_front));
    battle_add_unit(&context, battle_create_unit_at(2, hero_get_template(2), BATTLE_SIDE_PLAYER, player_left));
    battle_add_unit(&context, battle_create_unit_at(3, hero_get_template(3), BATTLE_SIDE_PLAYER, player_back));

    battle_add_unit(&context, battle_create_unit_at(101, hero_get_template(4), BATTLE_SIDE_ENEMY, enemy_front));
    battle_add_unit(&context, battle_create_unit_at(102, hero_get_template(5), BATTLE_SIDE_ENEMY, enemy_left));
    battle_add_unit(&context, battle_create_unit_at(103, hero_get_template(6), BATTLE_SIDE_ENEMY, enemy_back));

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
