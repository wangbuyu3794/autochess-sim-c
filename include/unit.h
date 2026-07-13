#ifndef AUTOCHESS_UNIT_H
#define AUTOCHESS_UNIT_H

#include "board.h"
#include "equipment.h"

typedef enum
{
    UNIT_LOCATION_NONE = 0,
    UNIT_LOCATION_BENCH,
    UNIT_LOCATION_BOARD
} UnitLocation;

typedef struct
{
    int instance_id;
    int template_id;
    int star;
    UnitLocation location;
    int bench_index;
    BoardPosition position;
    EquipmentId equipment_id;
    int is_active;
} Unit;

Unit unit_create(int instance_id, int template_id);
void unit_place_on_bench(Unit *unit, int bench_index);
void unit_deploy_to_board(Unit *unit, BoardPosition position);
int unit_is_on_bench(const Unit *unit);
int unit_is_deployed(const Unit *unit);

#endif
