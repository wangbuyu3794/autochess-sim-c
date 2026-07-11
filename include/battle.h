#ifndef AUTOCHESS_BATTLE_H
#define AUTOCHESS_BATTLE_H

#include <stdio.h>

#include "config.h"
#include "hero.h"

typedef enum
{
    BATTLE_SIDE_PLAYER = 0,
    BATTLE_SIDE_ENEMY = 1
} BattleSide;

typedef enum
{
    BATTLE_RESULT_ONGOING = 0,
    BATTLE_RESULT_PLAYER_WIN,
    BATTLE_RESULT_ENEMY_WIN,
    BATTLE_RESULT_DRAW
} BattleResult;

typedef struct
{
    int instance_id;
    int template_id;
    BattleSide side;
    const char *name;
    int max_hp;
    int current_hp;
    int attack;
    int is_alive;
} BattleUnit;

typedef struct
{
    BattleUnit units[AUTOCHESS_MAX_BATTLE_UNITS];
    int unit_count;
    int current_round;
} BattleContext;

BattleUnit battle_create_unit(int instance_id, const HeroTemplate *hero, BattleSide side);
void battle_add_unit(BattleContext *context, BattleUnit unit);
void battle_apply_damage(BattleUnit *target, int damage);
int battle_select_target_lowest_hp(const BattleContext *context, BattleSide attacker_side);
BattleResult battle_check_result(const BattleContext *context);
BattleResult battle_run(BattleContext *context, FILE *log_stream);
BattleContext battle_create_demo_context(void);
const char *battle_result_name(BattleResult result);

#endif
