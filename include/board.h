#ifndef AUTOCHESS_BOARD_H
#define AUTOCHESS_BOARD_H

typedef struct
{
    int row;
    int col;
} BoardPosition;

int board_is_position_valid(BoardPosition position);
int board_is_player_deploy_position(BoardPosition position);
int board_is_enemy_deploy_position(BoardPosition position);
int board_manhattan_distance(BoardPosition a, BoardPosition b);
int board_positions_equal(BoardPosition a, BoardPosition b);

#endif
