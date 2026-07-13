#ifndef AUTOCHESS_EQUIPMENT_H
#define AUTOCHESS_EQUIPMENT_H

#include <stddef.h>

typedef enum
{
    EQUIPMENT_NONE = 0,
    EQUIPMENT_BROADSWORD,
    EQUIPMENT_GUARDIAN_VEST,
    EQUIPMENT_MANA_GEM,
    EQUIPMENT_CRIT_GLOVES
} EquipmentId;

typedef struct
{
    EquipmentId id;
    const char *name;
    int bonus_hp;
    int bonus_attack;
    int bonus_armor;
    int bonus_magic_resist;
    int bonus_initial_mana;
    int bonus_crit_chance;
    int bonus_crit_damage;
} EquipmentTemplate;

const EquipmentTemplate *equipment_get_template(EquipmentId id);
const EquipmentTemplate *equipment_get_templates(size_t *count);
const char *equipment_name(EquipmentId id);

#endif
