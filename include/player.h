#ifndef AUTOCHESS_PLAYER_H
#define AUTOCHESS_PLAYER_H

#include "battle.h"
#include "config.h"
#include "unit.h"

typedef enum
{
    PLAYER_OK = 0,
    PLAYER_ERROR_INVALID_ARGUMENT,
    PLAYER_ERROR_BENCH_FULL,
    PLAYER_ERROR_INVALID_BENCH_SLOT,
    PLAYER_ERROR_EMPTY_BENCH_SLOT,
    PLAYER_ERROR_INVALID_DEPLOY_POSITION,
    PLAYER_ERROR_POSITION_OCCUPIED,
    PLAYER_ERROR_DEPLOY_LIMIT_REACHED,
    PLAYER_ERROR_UNIT_NOT_FOUND,
    PLAYER_ERROR_INVALID_EQUIPMENT,
    PLAYER_ERROR_EQUIPMENT_UNAVAILABLE,
    PLAYER_ERROR_UNIT_HAS_NO_EQUIPMENT
} PlayerResult;

typedef struct
{
    int id;
    BattleSide side;
    int health;
    int gold;
    int level;
    int experience;
    Unit units[AUTOCHESS_MAX_PLAYER_UNITS];
    int unit_count;
    int bench_slots[AUTOCHESS_BENCH_SIZE];
    int equipment_counts[AUTOCHESS_MAX_EQUIPMENT_ID];
} Player;

void player_init(Player *player, int id, BattleSide side);
PlayerResult player_add_unit_to_bench(Player *player, Unit unit);
PlayerResult player_deploy_from_bench(Player *player, int bench_index, BoardPosition position);
PlayerResult player_move_deployed_unit(Player *player, BoardPosition from, BoardPosition to);
PlayerResult player_return_unit_to_bench(Player *player, BoardPosition position);
PlayerResult player_sell_unit(Player *player, int unit_index, int *refund);
PlayerResult player_add_equipment(Player *player, EquipmentId equipment_id, int count);
PlayerResult player_equip_unit(Player *player, int unit_index, EquipmentId equipment_id);
PlayerResult player_unequip_unit(Player *player, int unit_index);
int player_try_merge_units(Player *player);
int player_count_equipment(const Player *player, EquipmentId equipment_id);
int player_count_deployed_units(const Player *player);
int player_count_active_units(const Player *player);
int player_count_units_by_template_and_star(const Player *player, int template_id, int star);
int player_is_position_occupied(const Player *player, BoardPosition position);
int player_find_deployed_unit_index(const Player *player, BoardPosition position);
void player_add_deployed_units_to_battle(const Player *player, BattleContext *context);
const char *player_result_name(PlayerResult result);

#endif
