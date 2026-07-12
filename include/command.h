#ifndef AUTOCHESS_COMMAND_H
#define AUTOCHESS_COMMAND_H

#include <stdio.h>

#include "game.h"

typedef enum
{
    COMMAND_RESULT_CONTINUE = 0,
    COMMAND_RESULT_READY,
    COMMAND_RESULT_QUIT,
    COMMAND_RESULT_ERROR
} CommandResult;

void command_print_help(FILE *output);
void command_print_shop(const GameContext *game, FILE *output);
void command_print_roster(const GameContext *game, FILE *output);
CommandResult command_execute_preparation(GameContext *game, const char *line, FILE *output);
CommandResult command_run_preparation_loop(GameContext *game, FILE *input, FILE *output);
const char *command_result_name(CommandResult result);

#endif
