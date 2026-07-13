#include "skill.h"

#include "battle.h"
#include "logger.h"

static const SkillDefinition SKILL_DEFINITIONS[] = {
    {SKILL_IRON_SHIELD, "铁壁护盾", SKILL_DAMAGE_NONE, SKILL_TARGET_SELF, 0, 0, 35, 0, 0, 20, 0, 0, 0, 0},
    {SKILL_POWER_STRIKE, "破势斩", SKILL_DAMAGE_PHYSICAL, SKILL_TARGET_ENEMY, 0, 100, 0, 0, 0, 0, 0, 0, 1, 0},
    {SKILL_QUICK_SHOT, "连珠箭", SKILL_DAMAGE_PHYSICAL, SKILL_TARGET_ENEMY, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {SKILL_FIREBALL, "火球术", SKILL_DAMAGE_MAGICAL, SKILL_TARGET_ENEMY, 45, 0, 0, 0, 0, 0, 6, 2, 0, 50},
    {SKILL_ARCANE_BOLT, "秘法箭", SKILL_DAMAGE_MAGICAL, SKILL_TARGET_ENEMY, 25, 60, 0, 0, 0, 0, 8, 2, 0, 0},
    {SKILL_HOLY_GUARD, "圣光守护", SKILL_DAMAGE_NONE, SKILL_TARGET_LOWEST_HP_ALLY, 0, 0, 55, 0, 0, 35, 0, 0, 0, 0},
    {SKILL_EXECUTE_STRIKE, "终结斩", SKILL_DAMAGE_PHYSICAL, SKILL_TARGET_ENEMY, 10, 100, 0, 40, 25, 0, 0, 0, 1, 0},
};

const char *skill_name(SkillId skill_id)
{
    const SkillDefinition *definition = skill_get_definition(skill_id);

    return definition != 0 ? definition->name : "无技能";
}

const SkillDefinition *skill_get_definition(SkillId skill_id)
{
    int count = (int)(sizeof(SKILL_DEFINITIONS) / sizeof(SKILL_DEFINITIONS[0]));

    for (int i = 0; i < count; ++i)
    {
        if (SKILL_DEFINITIONS[i].id == skill_id)
        {
            return &SKILL_DEFINITIONS[i];
        }
    }

    return 0;
}

int skill_calculate_damage(const SkillDefinition *definition, const BattleUnit *caster, const BattleUnit *target)
{
    int raw_damage = 0;

    if (definition == 0 || caster == 0 || target == 0)
    {
        return 0;
    }

    raw_damage = definition->base_damage + (caster->attack * definition->attack_percent) / 100;
    if (definition->execute_threshold_percent > 0 &&
        target->max_hp > 0 &&
        target->current_hp * 100 <= target->max_hp * definition->execute_threshold_percent)
    {
        raw_damage += definition->execute_bonus_damage;
    }

    if (raw_damage <= 0)
    {
        return 0;
    }

    if (definition->damage_type == SKILL_DAMAGE_PHYSICAL)
    {
        return battle_calculate_mitigated_damage(raw_damage, target->armor);
    }

    if (definition->damage_type == SKILL_DAMAGE_MAGICAL)
    {
        return battle_calculate_spell_damage(raw_damage, target);
    }

    return 0;
}

static int skill_calculate_scaled_damage(const SkillDefinition *definition, const BattleUnit *caster, const BattleUnit *target, int percent)
{
    SkillDefinition scaled;

    if (definition == 0 || percent <= 0)
    {
        return 0;
    }

    scaled = *definition;
    scaled.base_damage = (scaled.base_damage * percent) / 100;
    scaled.attack_percent = (scaled.attack_percent * percent) / 100;
    scaled.execute_bonus_damage = (scaled.execute_bonus_damage * percent) / 100;

    return skill_calculate_damage(&scaled, caster, target);
}

int skill_calculate_healing(const SkillDefinition *definition)
{
    return definition != 0 && definition->healing > 0 ? definition->healing : 0;
}

static void heal_unit(BattleUnit *unit, int amount)
{
    if (unit == 0 || !unit->is_alive || amount <= 0)
    {
        return;
    }

    unit->current_hp += amount;
    if (unit->current_hp > unit->max_hp)
    {
        unit->current_hp = unit->max_hp;
    }
}

static int select_lowest_hp_ally(const BattleContext *context, int caster_index)
{
    int best_index = -1;

    if (context == 0 || caster_index < 0 || caster_index >= context->unit_count)
    {
        return -1;
    }

    for (int i = 0; i < context->unit_count; ++i)
    {
        const BattleUnit *candidate = &context->units[i];
        const BattleUnit *caster = &context->units[caster_index];

        if (candidate->side != caster->side || candidate->is_alive == 0)
        {
            continue;
        }

        if (best_index < 0 ||
            candidate->current_hp * context->units[best_index].max_hp < context->units[best_index].current_hp * candidate->max_hp ||
            (candidate->current_hp * context->units[best_index].max_hp == context->units[best_index].current_hp * candidate->max_hp &&
             candidate->instance_id < context->units[best_index].instance_id))
        {
            best_index = i;
        }
    }

    return best_index;
}

static void apply_splash_damage(BattleContext *context, int caster_index, int primary_target_index, const SkillDefinition *definition, FILE *log_stream)
{
    BattleUnit *caster = 0;
    BattleUnit *primary = 0;

    if (context == 0 || definition == 0 || definition->splash_percent <= 0 ||
        caster_index < 0 || caster_index >= context->unit_count ||
        primary_target_index < 0 || primary_target_index >= context->unit_count)
    {
        return;
    }

    caster = &context->units[caster_index];
    primary = &context->units[primary_target_index];

    for (int i = 0; i < context->unit_count; ++i)
    {
        BattleUnit *candidate = &context->units[i];
        int damage = 0;

        if (i == primary_target_index ||
            candidate->side == caster->side ||
            candidate->is_alive == 0 ||
            board_manhattan_distance(candidate->position, primary->position) > 1)
        {
            continue;
        }

        damage = skill_calculate_scaled_damage(definition, caster, candidate, definition->splash_percent);
        battle_apply_damage(candidate, damage);
        logger_skill_damage(log_stream, definition->name, candidate->name, damage, candidate->current_hp, candidate->max_hp);
        if (!candidate->is_alive)
        {
            logger_defeated(log_stream, candidate->name);
        }
    }
}

int skill_cast(BattleContext *context, int caster_index, int target_index, FILE *log_stream)
{
    BattleUnit *caster = 0;
    BattleUnit *target = 0;
    const SkillDefinition *definition = 0;

    if (context == 0 ||
        caster_index < 0 ||
        caster_index >= context->unit_count ||
        target_index < 0 ||
        target_index >= context->unit_count)
    {
        return 0;
    }

    caster = &context->units[caster_index];

    if (!caster->is_alive || caster->current_mana < caster->max_mana || caster->skill_id == SKILL_NONE)
    {
        return 0;
    }

    definition = skill_get_definition(caster->skill_id);
    if (definition == 0)
    {
        return 0;
    }

    caster->current_mana = 0;

    if (definition->target_type == SKILL_TARGET_SELF || definition->target_type == SKILL_TARGET_LOWEST_HP_ALLY)
    {
        int ally_index = definition->target_type == SKILL_TARGET_LOWEST_HP_ALLY ? select_lowest_hp_ally(context, caster_index) : caster_index;
        BattleUnit *ally = ally_index >= 0 ? &context->units[ally_index] : caster;
        int healing = skill_calculate_healing(definition);
        logger_skill(log_stream, caster->name, definition->name, ally->name);
        heal_unit(ally, healing);
        logger_skill_heal(log_stream, definition->name, ally->name, healing, ally->current_hp, ally->max_hp);
        if (definition->shield > 0)
        {
            battle_add_shield(ally, definition->shield);
            logger_shield_gain(log_stream, ally->name, definition->shield, ally->shield);
        }
        return 1;
    }

    target = &context->units[target_index];
    if (definition->target_type != SKILL_TARGET_ENEMY || !target->is_alive)
    {
        return 1;
    }

    logger_skill(log_stream, caster->name, definition->name, target->name);
    int damage = skill_calculate_damage(definition, caster, target);
    battle_apply_damage(target, damage);
    logger_skill_damage(log_stream, definition->name, target->name, damage, target->current_hp, target->max_hp);
    apply_splash_damage(context, caster_index, target_index, definition, log_stream);
    if (definition->burn_damage > 0 && definition->burn_turns > 0)
    {
        battle_apply_burn(target, definition->burn_damage, definition->burn_turns);
    }
    if (definition->stun_turns > 0)
    {
        battle_apply_stun(target, definition->stun_turns);
    }

    if (!target->is_alive)
    {
        logger_defeated(log_stream, target->name);
    }

    return 1;
}
