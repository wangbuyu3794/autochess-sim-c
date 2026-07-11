#include "unit.h"

Unit unit_create(int instance_id, int template_id)
{
    BoardPosition default_position = {-1, -1};
    Unit unit;

    unit.instance_id = instance_id;
    unit.template_id = template_id;
    unit.star = 1;
    unit.location = UNIT_LOCATION_NONE;
    unit.bench_index = -1;
    unit.position = default_position;
    unit.is_active = 1;

    return unit;
}

void unit_place_on_bench(Unit *unit, int bench_index)
{
    BoardPosition no_position = {-1, -1};

    if (unit == 0)
    {
        return;
    }

    unit->location = UNIT_LOCATION_BENCH;
    unit->bench_index = bench_index;
    unit->position = no_position;
}

void unit_deploy_to_board(Unit *unit, BoardPosition position)
{
    if (unit == 0)
    {
        return;
    }

    unit->location = UNIT_LOCATION_BOARD;
    unit->bench_index = -1;
    unit->position = position;
}

int unit_is_on_bench(const Unit *unit)
{
    return unit != 0 && unit->is_active && unit->location == UNIT_LOCATION_BENCH;
}

int unit_is_deployed(const Unit *unit)
{
    return unit != 0 && unit->is_active && unit->location == UNIT_LOCATION_BOARD;
}
