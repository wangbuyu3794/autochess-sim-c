#ifndef AUTOCHESS_GAME_H
#define AUTOCHESS_GAME_H

#include <stdio.h>

#include "battle.h"
#include "player.h"
#include "shop.h"

typedef enum
{
    GAME_RESULT_ONGOING = 0,
    GAME_RESULT_PLAYER_WIN,
    GAME_RESULT_ENEMY_WIN,
    GAME_RESULT_DRAW
} GameResult;

typedef struct
{
    Player player;
    Player enemy;
    Shop player_shop;
    Shop enemy_shop;
    int player_next_instance_id;
    int enemy_next_instance_id;
    int current_round;
    GameResult result;
} GameContext;

void game_init(GameContext *game, unsigned int player_seed, unsigned int enemy_seed);
void game_seed_player_demo_units(GameContext *game);
int game_calculate_damage(const BattleContext *battle_context, BattleSide winner);
GameResult game_settle_battle(GameContext *game, const BattleContext *battle_context, BattleResult battle_result);
BattleResult game_run_round(GameContext *game, FILE *log_stream);
GameResult game_run_until_over(GameContext *game, FILE *log_stream);
GameResult game_run_until_over_with_limit(GameContext *game, int max_rounds, FILE *log_stream);
const char *game_result_name(GameResult result);

#endif
