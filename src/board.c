#include "board.h"

#include <stdlib.h>

#include "config.h"

int board_is_position_valid(BoardPosition position)
{
    return position.row >= 0 &&
           position.row < AUTOCHESS_BOARD_ROWS &&
           position.col >= 0 &&
           position.col < AUTOCHESS_BOARD_COLS;
}

int board_is_player_deploy_position(BoardPosition position)
{
    return board_is_position_valid(position) &&
           position.row >= AUTOCHESS_BOARD_ROWS / 2;
}

int board_is_enemy_deploy_position(BoardPosition position)
{
    return board_is_position_valid(position) &&
           position.row < AUTOCHESS_BOARD_ROWS / 2;
}

int board_manhattan_distance(BoardPosition a, BoardPosition b)
{
    return abs(a.row - b.row) + abs(a.col - b.col);
}

int board_positions_equal(BoardPosition a, BoardPosition b)
{
    return a.row == b.row && a.col == b.col;
}
