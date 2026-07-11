#ifndef AUTOCHESS_SKILL_H
#define AUTOCHESS_SKILL_H

#include <stdio.h>

typedef struct BattleContext BattleContext;

typedef enum
{
    SKILL_NONE = 0,
    SKILL_IRON_SHIELD = 1,
    SKILL_POWER_STRIKE = 2,
    SKILL_QUICK_SHOT = 3,
    SKILL_FIREBALL = 4
} SkillId;

const char *skill_name(SkillId skill_id);
int skill_cast(BattleContext *context, int caster_index, int target_index, FILE *log_stream);

#endif
