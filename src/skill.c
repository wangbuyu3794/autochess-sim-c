#include "skill.h"

#include "battle.h"
#include "logger.h"

static const SkillDefinition SKILL_DEFINITIONS[] = {
    {SKILL_IRON_SHIELD, "铁壁护盾", SKILL_DAMAGE_NONE, SKILL_TARGET_SELF, 0, 0, 35, 0, 0},
    {SKILL_POWER_STRIKE, "破势斩", SKILL_DAMAGE_PHYSICAL, SKILL_TARGET_ENEMY, 0, 100, 0, 0, 0},
    {SKILL_QUICK_SHOT, "连珠箭", SKILL_DAMAGE_PHYSICAL, SKILL_TARGET_ENEMY, 30, 0, 0, 0, 0},
    {SKILL_FIREBALL, "火球术", SKILL_DAMAGE_MAGICAL, SKILL_TARGET_ENEMY, 45, 0, 0, 0, 0},
    {SKILL_ARCANE_BOLT, "秘法箭", SKILL_DAMAGE_MAGICAL, SKILL_TARGET_ENEMY, 25, 60, 0, 0, 0},
    {SKILL_HOLY_GUARD, "圣光守护", SKILL_DAMAGE_NONE, SKILL_TARGET_SELF, 0, 0, 55, 0, 0},
    {SKILL_EXECUTE_STRIKE, "终结斩", SKILL_DAMAGE_PHYSICAL, SKILL_TARGET_ENEMY, 10, 100, 0, 40, 25},
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

int skill_calculate_healing(const SkillDefinition *definition)
{
    return definition != 0 && definition->healing > 0 ? definition->healing : 0;
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
    target = &context->units[target_index];

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

    if (definition->target_type == SKILL_TARGET_SELF)
    {
        logger_skill(log_stream, caster->name, definition->name, caster->name);
        heal_self(caster, skill_calculate_healing(definition));
        return 1;
    }

    if (definition->target_type != SKILL_TARGET_ENEMY || !target->is_alive)
    {
        return 1;
    }

    logger_skill(log_stream, caster->name, definition->name, target->name);
    battle_apply_damage(target, skill_calculate_damage(definition, caster, target));

    if (!target->is_alive)
    {
        logger_defeated(log_stream, target->name);
    }

    return 1;
}
