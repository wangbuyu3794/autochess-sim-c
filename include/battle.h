#ifndef AUTOCHESS_BATTLE_H
#define AUTOCHESS_BATTLE_H

#include <stdio.h>

#include "config.h"
#include "board.h"
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

typedef struct BattleUnit
{
    int instance_id;
    int template_id;
    BattleSide side;
    const char *name;
    int max_hp;
    int current_hp;
    int attack;
    int armor;
    int magic_resist;
    int crit_chance;
    int crit_damage;
    int attack_range;
    int current_mana;
    int max_mana;
    int shield;
    int burn_damage;
    int burn_turns;
    int stun_turns;
    SkillId skill_id;
    BoardPosition position;
    int is_alive;
    TraitId class_trait;
    TraitId origin_trait;
} BattleUnit;

typedef struct BattleContext
{
    BattleUnit units[AUTOCHESS_MAX_BATTLE_UNITS];
    int unit_count;
    int current_round;
} BattleContext;

BattleUnit battle_create_unit(int instance_id, const HeroTemplate *hero, BattleSide side);
BattleUnit battle_create_unit_at(int instance_id, const HeroTemplate *hero, BattleSide side, BoardPosition position);
BattleUnit battle_create_unit_at_star(int instance_id, const HeroTemplate *hero, BattleSide side, BoardPosition position, int star);
void battle_add_unit(BattleContext *context, BattleUnit unit);
void battle_apply_trait_summary(BattleContext *context);
void battle_apply_damage(BattleUnit *target, int damage);
void battle_add_shield(BattleUnit *unit, int amount);
void battle_apply_burn(BattleUnit *unit, int damage, int turns);
void battle_apply_stun(BattleUnit *unit, int turns);
int battle_process_status_start(BattleUnit *unit, FILE *log_stream);
int battle_calculate_mitigated_damage(int raw_damage, int resistance);
int battle_is_critical_hit(const BattleUnit *attacker, const BattleUnit *target, int round);
int battle_calculate_attack_damage(const BattleUnit *attacker, const BattleUnit *target, int round);
int battle_calculate_spell_damage(int raw_damage, const BattleUnit *target);
void battle_gain_mana(BattleUnit *unit, int amount);
int battle_select_target_nearest(const BattleContext *context, int attacker_index);
int battle_is_position_occupied(const BattleContext *context, BoardPosition position);
int battle_is_target_in_range(const BattleUnit *attacker, const BattleUnit *target);
int battle_try_move_toward(BattleContext *context, int mover_index, BoardPosition target_position);
BattleResult battle_check_result(const BattleContext *context);
BattleResult battle_run(BattleContext *context, FILE *log_stream);
BattleContext battle_create_demo_context(void);
const char *battle_result_name(BattleResult result);

#endif
