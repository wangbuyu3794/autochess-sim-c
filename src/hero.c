#include "hero.h"

enum
{
    HERO_IRON_GUARD = 1,
    HERO_BLADE_RUNNER = 2,
    HERO_FOREST_ARCHER = 3,
    HERO_FIRE_SPARK = 4,
    HERO_SHADOW_BLADE = 5,
    HERO_ROCK_ARMOR = 6
};

static const HeroTemplate HERO_TEMPLATES[] = {
    {HERO_IRON_GUARD, "铁卫", 1, 130, 16, 1, TRAIT_GUARDIAN, TRAIT_CITY},
    {HERO_BLADE_RUNNER, "斩锋", 1, 105, 24, 1, TRAIT_BLADEMASTER, TRAIT_CITY},
    {HERO_FOREST_ARCHER, "林弓", 1, 90, 28, 3, TRAIT_RANGER, TRAIT_FOREST},
    {HERO_FIRE_SPARK, "火苗", 1, 80, 30, 3, TRAIT_MAGE, TRAIT_ELEMENT},
    {HERO_SHADOW_BLADE, "影刃", 2, 95, 27, 1, TRAIT_BLADEMASTER, TRAIT_SHADOW},
    {HERO_ROCK_ARMOR, "岩甲", 2, 140, 15, 1, TRAIT_GUARDIAN, TRAIT_ELEMENT},
};

const HeroTemplate *hero_get_template(int template_id)
{
    size_t count = 0;
    const HeroTemplate *templates = hero_get_templates(&count);

    for (size_t i = 0; i < count; ++i)
    {
        if (templates[i].id == template_id)
        {
            return &templates[i];
        }
    }

    return NULL;
}

const HeroTemplate *hero_get_templates(size_t *count)
{
    if (count != NULL)
    {
        *count = sizeof(HERO_TEMPLATES) / sizeof(HERO_TEMPLATES[0]);
    }

    return HERO_TEMPLATES;
}

const HeroTemplate *hero_get_first_template_by_cost(int cost)
{
    size_t count = 0;
    const HeroTemplate *templates = hero_get_templates(&count);

    for (size_t i = 0; i < count; ++i)
    {
        if (templates[i].cost == cost)
        {
            return &templates[i];
        }
    }

    return NULL;
}
