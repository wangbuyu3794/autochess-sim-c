#include "equipment.h"

static const EquipmentTemplate EQUIPMENT_TEMPLATES[] = {
    {EQUIPMENT_BROADSWORD, "长剑", 0, 10, 0, 0, 0, 0, 0},
    {EQUIPMENT_GUARDIAN_VEST, "守护甲", 50, 0, 15, 0, 0, 0, 0},
    {EQUIPMENT_MANA_GEM, "法力宝石", 0, 0, 0, 0, 20, 0, 0},
    {EQUIPMENT_CRIT_GLOVES, "暴击手套", 0, 0, 0, 0, 0, 15, 25},
};

const EquipmentTemplate *equipment_get_template(EquipmentId id)
{
    size_t count = 0;
    const EquipmentTemplate *templates = equipment_get_templates(&count);

    for (size_t i = 0; i < count; ++i)
    {
        if (templates[i].id == id)
        {
            return &templates[i];
        }
    }

    return 0;
}

const EquipmentTemplate *equipment_get_templates(size_t *count)
{
    if (count != 0)
    {
        *count = sizeof(EQUIPMENT_TEMPLATES) / sizeof(EQUIPMENT_TEMPLATES[0]);
    }

    return EQUIPMENT_TEMPLATES;
}

const char *equipment_name(EquipmentId id)
{
    const EquipmentTemplate *equipment = equipment_get_template(id);
    return equipment != 0 ? equipment->name : "无";
}
