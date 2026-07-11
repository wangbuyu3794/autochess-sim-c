#ifndef AUTOCHESS_TRAIT_H
#define AUTOCHESS_TRAIT_H

#include "config.h"

typedef enum
{
    TRAIT_NONE = 0,
    TRAIT_GUARDIAN = 1,
    TRAIT_BLADEMASTER = 2,
    TRAIT_RANGER = 3,
    TRAIT_MAGE = 4,
    TRAIT_CITY = 5,
    TRAIT_FOREST = 6,
    TRAIT_ELEMENT = 7,
    TRAIT_SHADOW = 8
} TraitId;

typedef struct
{
    int counts[AUTOCHESS_MAX_TRAIT_ID];
} TraitSummary;

const char *trait_name(TraitId trait);
void trait_summary_init(TraitSummary *summary);
void trait_summary_add(TraitSummary *summary, TraitId trait);
int trait_summary_get_count(const TraitSummary *summary, TraitId trait);
int trait_guardian_bonus_hp(const TraitSummary *summary);
int trait_blademaster_attack_percent(const TraitSummary *summary);

#endif
