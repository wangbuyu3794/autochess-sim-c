#ifndef AUTOCHESS_HERO_H
#define AUTOCHESS_HERO_H

#include <stddef.h>

typedef struct
{
    int id;
    const char *name;
    int cost;
    int base_hp;
    int base_attack;
    int attack_range;
} HeroTemplate;

const HeroTemplate *hero_get_template(int template_id);
const HeroTemplate *hero_get_templates(size_t *count);
const HeroTemplate *hero_get_first_template_by_cost(int cost);

#endif
