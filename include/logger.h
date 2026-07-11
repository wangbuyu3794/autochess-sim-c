#ifndef AUTOCHESS_LOGGER_H
#define AUTOCHESS_LOGGER_H

#include <stdio.h>

void logger_battle_start(FILE *stream);
void logger_round_start(FILE *stream, int round);
void logger_attack(FILE *stream, const char *attacker, const char *target, int damage, int current_hp, int max_hp);
void logger_defeated(FILE *stream, const char *target);
void logger_battle_end(FILE *stream, const char *result_name);

#endif
