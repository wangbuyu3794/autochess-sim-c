#ifndef AUTOCHESS_SKILL_H
#define AUTOCHESS_SKILL_H

#include <stdio.h>

typedef struct BattleContext BattleContext;
typedef struct BattleUnit BattleUnit;

typedef enum
{
    SKILL_NONE = 0,
    SKILL_IRON_SHIELD = 1,
    SKILL_POWER_STRIKE = 2,
    SKILL_QUICK_SHOT = 3,
    SKILL_FIREBALL = 4,
    SKILL_ARCANE_BOLT = 5,
    SKILL_HOLY_GUARD = 6,
    SKILL_EXECUTE_STRIKE = 7
} SkillId;

typedef enum
{
    SKILL_DAMAGE_NONE = 0,
    SKILL_DAMAGE_PHYSICAL = 1,
    SKILL_DAMAGE_MAGICAL = 2
} SkillDamageType;

typedef enum
{
    SKILL_TARGET_NONE = 0,
    SKILL_TARGET_SELF = 1,
    SKILL_TARGET_ENEMY = 2
} SkillTargetType;

typedef struct
{
    SkillId id;
    const char *name;
    SkillDamageType damage_type;
    SkillTargetType target_type;
    int base_damage;
    int attack_percent;
    int healing;
    int execute_threshold_percent;
    int execute_bonus_damage;
    int shield;
    int burn_damage;
    int burn_turns;
    int stun_turns;
} SkillDefinition;

const char *skill_name(SkillId skill_id);
const SkillDefinition *skill_get_definition(SkillId skill_id);
int skill_calculate_damage(const SkillDefinition *definition, const BattleUnit *caster, const BattleUnit *target);
int skill_calculate_healing(const SkillDefinition *definition);
int skill_cast(BattleContext *context, int caster_index, int target_index, FILE *log_stream);

#endif
