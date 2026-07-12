#include "hero.h"

enum
{
    HERO_IRON_GUARD = 1,
    HERO_BLADE_RUNNER = 2,
    HERO_FOREST_ARCHER = 3,
    HERO_FIRE_SPARK = 4,
    HERO_SHADOW_BLADE = 5,
    HERO_ROCK_ARMOR = 6,
    HERO_STORM_RANGER = 7,
    HERO_RUNE_MAGE = 8,
    HERO_DAWN_GUARDIAN = 9,
    HERO_NIGHT_ASSASSIN = 10
};

static const HeroTemplate HERO_TEMPLATES[] = {
    {HERO_IRON_GUARD, "铁卫", 1, 130, 16, 18, 10, 5, 150, 1, 0, 60, SKILL_IRON_SHIELD, TRAIT_GUARDIAN, TRAIT_CITY},
    {HERO_BLADE_RUNNER, "斩锋", 1, 105, 24, 10, 8, 10, 150, 1, 0, 60, SKILL_POWER_STRIKE, TRAIT_BLADEMASTER, TRAIT_CITY},
    {HERO_FOREST_ARCHER, "林弓", 1, 90, 28, 6, 8, 15, 150, 3, 0, 60, SKILL_QUICK_SHOT, TRAIT_RANGER, TRAIT_FOREST},
    {HERO_FIRE_SPARK, "火苗", 1, 80, 30, 5, 12, 5, 150, 3, 0, 60, SKILL_FIREBALL, TRAIT_MAGE, TRAIT_ELEMENT},
    {HERO_SHADOW_BLADE, "影刃", 2, 95, 27, 9, 8, 20, 160, 1, 0, 60, SKILL_POWER_STRIKE, TRAIT_BLADEMASTER, TRAIT_SHADOW},
    {HERO_ROCK_ARMOR, "岩甲", 2, 140, 15, 24, 14, 5, 150, 1, 0, 60, SKILL_IRON_SHIELD, TRAIT_GUARDIAN, TRAIT_ELEMENT},
    {HERO_STORM_RANGER, "岚弩", 3, 115, 36, 8, 12, 18, 160, 3, 10, 70, SKILL_QUICK_SHOT, TRAIT_RANGER, TRAIT_ELEMENT},
    {HERO_RUNE_MAGE, "符法", 3, 105, 40, 6, 18, 8, 150, 3, 20, 80, SKILL_ARCANE_BOLT, TRAIT_MAGE, TRAIT_CITY},
    {HERO_DAWN_GUARDIAN, "曦盾", 4, 190, 28, 28, 18, 5, 150, 1, 20, 90, SKILL_HOLY_GUARD, TRAIT_GUARDIAN, TRAIT_FOREST},
    {HERO_NIGHT_ASSASSIN, "夜刺", 4, 150, 48, 12, 10, 25, 170, 1, 30, 80, SKILL_EXECUTE_STRIKE, TRAIT_BLADEMASTER, TRAIT_SHADOW},
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
