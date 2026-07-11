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
    {HERO_IRON_GUARD, "铁卫", 130, 16, 1},
    {HERO_BLADE_RUNNER, "斩锋", 105, 24, 1},
    {HERO_FOREST_ARCHER, "林弓", 90, 28, 3},
    {HERO_FIRE_SPARK, "火苗", 80, 30, 3},
    {HERO_SHADOW_BLADE, "影刃", 95, 27, 1},
    {HERO_ROCK_ARMOR, "岩甲", 140, 15, 1},
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
