#include "skill.h"

#include "battle.h"
#include "logger.h"

const char *skill_name(SkillId skill_id)
{
    switch (skill_id)
    {
    case SKILL_IRON_SHIELD:
        return "铁壁护盾";
    case SKILL_POWER_STRIKE:
        return "破势斩";
    case SKILL_QUICK_SHOT:
        return "连珠箭";
    case SKILL_FIREBALL:
        return "火球术";
    case SKILL_NONE:
    default:
        return "无技能";
    }
}

static void heal_self(BattleUnit *unit, int amount)
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

int skill_cast(BattleContext *context, int caster_index, int target_index, FILE *log_stream)
{
    BattleUnit *caster = 0;
    BattleUnit *target = 0;
    SkillId skill_id = SKILL_NONE;

    if (context == 0 ||
        caster_index < 0 ||
        caster_index >= context->unit_count ||
        target_index < 0 ||
        target_index >= context->unit_count)
    {
        return 0;
    }

    caster = &context->units[caster_index];
    target = &context->units[target_index];

    if (!caster->is_alive || caster->current_mana < caster->max_mana || caster->skill_id == SKILL_NONE)
    {
        return 0;
    }

    skill_id = caster->skill_id;
    caster->current_mana = 0;

    if (skill_id == SKILL_IRON_SHIELD)
    {
        logger_skill(log_stream, caster->name, skill_name(skill_id), caster->name);
        heal_self(caster, 35);
        return 1;
    }

    if (!target->is_alive)
    {
        return 1;
    }

    logger_skill(log_stream, caster->name, skill_name(skill_id), target->name);

    if (skill_id == SKILL_POWER_STRIKE)
    {
        battle_apply_damage(target, caster->attack);
    }
    else if (skill_id == SKILL_QUICK_SHOT)
    {
        battle_apply_damage(target, 30);
    }
    else if (skill_id == SKILL_FIREBALL)
    {
        battle_apply_damage(target, 45);
    }

    if (!target->is_alive)
    {
        logger_defeated(log_stream, target->name);
    }

    return 1;
}
