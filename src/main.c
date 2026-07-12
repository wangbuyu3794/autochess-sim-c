#include <stdio.h>

#include "game.h"

int main(void)
{
    GameContext game;
    GameResult result = GAME_RESULT_ONGOING;

    game_init(&game, 20260711u, 20260712u);
    game_seed_player_demo_units(&game);
    result = game_run_until_over(&game, stdout);

    return result == GAME_RESULT_DRAW ? 1 : 0;
}
