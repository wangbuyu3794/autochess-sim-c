#include "trait.h"

#include "config.h"

const char *trait_name(TraitId trait)
{
    switch (trait)
    {
    case TRAIT_GUARDIAN:
        return "守卫";
    case TRAIT_BLADEMASTER:
        return "剑士";
    case TRAIT_RANGER:
        return "游侠";
    case TRAIT_MAGE:
        return "法师";
    case TRAIT_CITY:
        return "城邦";
    case TRAIT_FOREST:
        return "森林";
    case TRAIT_ELEMENT:
        return "元素";
    case TRAIT_SHADOW:
        return "暗影";
    case TRAIT_NONE:
    default:
        return "无";
    }
}

void trait_summary_init(TraitSummary *summary)
{
    if (summary == 0)
    {
        return;
    }

    for (int i = 0; i < AUTOCHESS_MAX_TRAIT_ID; ++i)
    {
        summary->counts[i] = 0;
    }
}

void trait_summary_add(TraitSummary *summary, TraitId trait)
{
    if (summary == 0 || trait <= TRAIT_NONE || trait >= AUTOCHESS_MAX_TRAIT_ID)
    {
        return;
    }

    summary->counts[trait] += 1;
}

int trait_summary_get_count(const TraitSummary *summary, TraitId trait)
{
    if (summary == 0 || trait <= TRAIT_NONE || trait >= AUTOCHESS_MAX_TRAIT_ID)
    {
        return 0;
    }

    return summary->counts[trait];
}

int trait_current_threshold(int count)
{
    if (count >= 4)
    {
        return 4;
    }

    if (count >= 2)
    {
        return 2;
    }

    return 0;
}

int trait_next_threshold(int count)
{
    if (count < 2)
    {
        return 2;
    }

    if (count < 4)
    {
        return 4;
    }

    return 0;
}

int trait_guardian_bonus_hp(const TraitSummary *summary)
{
    int count = trait_summary_get_count(summary, TRAIT_GUARDIAN);

    if (count >= 4)
    {
        return 60;
    }

    if (count >= 2)
    {
        return 25;
    }

    return 0;
}

int trait_blademaster_attack_percent(const TraitSummary *summary)
{
    int count = trait_summary_get_count(summary, TRAIT_BLADEMASTER);

    if (count >= 4)
    {
        return 35;
    }

    if (count >= 2)
    {
        return 15;
    }

    return 0;
}

int trait_ranger_bonus_range(const TraitSummary *summary)
{
    int count = trait_summary_get_count(summary, TRAIT_RANGER);

    if (count >= 4)
    {
        return 2;
    }

    if (count >= 2)
    {
        return 1;
    }

    return 0;
}

int trait_mage_bonus_initial_mana(const TraitSummary *summary)
{
    int count = trait_summary_get_count(summary, TRAIT_MAGE);

    if (count >= 4)
    {
        return 40;
    }

    if (count >= 2)
    {
        return 20;
    }

    return 0;
}

int trait_city_bonus_initial_mana(const TraitSummary *summary)
{
    int count = trait_summary_get_count(summary, TRAIT_CITY);

    if (count >= 4)
    {
        return 30;
    }

    if (count >= 2)
    {
        return 15;
    }

    return 0;
}

int trait_forest_bonus_hp(const TraitSummary *summary)
{
    int count = trait_summary_get_count(summary, TRAIT_FOREST);

    if (count >= 4)
    {
        return 45;
    }

    if (count >= 2)
    {
        return 20;
    }

    return 0;
}

int trait_element_bonus_hp(const TraitSummary *summary)
{
    int count = trait_summary_get_count(summary, TRAIT_ELEMENT);

    if (count >= 4)
    {
        return 50;
    }

    if (count >= 2)
    {
        return 20;
    }

    return 0;
}

int trait_shadow_attack_percent(const TraitSummary *summary)
{
    int count = trait_summary_get_count(summary, TRAIT_SHADOW);

    if (count >= 4)
    {
        return 35;
    }

    if (count >= 2)
    {
        return 15;
    }

    return 0;
}
