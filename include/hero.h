#ifndef AUTOCHESS_HERO_H
#define AUTOCHESS_HERO_H

#include <stddef.h>

#include "skill.h"
#include "trait.h"

typedef struct
{
    int id;
    const char *name;
    int cost;
    int base_hp;
    int base_attack;
    int armor;
    int magic_resist;
    int crit_chance;
    int crit_damage;
    int attack_range;
    int initial_mana;
    int max_mana;
    SkillId skill_id;
    TraitId class_trait;
    TraitId origin_trait;
} HeroTemplate;

const HeroTemplate *hero_get_template(int template_id);
const HeroTemplate *hero_get_templates(size_t *count);
const HeroTemplate *hero_get_first_template_by_cost(int cost);

#endif
