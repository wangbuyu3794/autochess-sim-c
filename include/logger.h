#ifndef AUTOCHESS_LOGGER_H
#define AUTOCHESS_LOGGER_H

#include <stdio.h>

void logger_battle_start(FILE *stream);
void logger_round_start(FILE *stream, int round);
void logger_attack(FILE *stream, const char *attacker, const char *target, int damage, int current_hp, int max_hp);
void logger_attack_detail(FILE *stream, const char *attacker, const char *target, int damage, int current_hp, int max_hp, int is_critical);
void logger_skill(FILE *stream, const char *caster, const char *skill_name, const char *target);
void logger_skill_damage(FILE *stream, const char *skill_name, const char *target, int damage, int current_hp, int max_hp);
void logger_skill_heal(FILE *stream, const char *skill_name, const char *target, int healing, int current_hp, int max_hp);
void logger_shield_gain(FILE *stream, const char *unit_name, int amount, int shield);
void logger_shield_absorb(FILE *stream, const char *unit_name, int amount, int shield);
void logger_burn(FILE *stream, const char *unit_name, int damage, int turns_left, int current_hp, int max_hp);
void logger_stunned(FILE *stream, const char *unit_name, int turns_left);
void logger_move(FILE *stream, const char *unit_name, int from_row, int from_col, int to_row, int to_col);
void logger_wait(FILE *stream, const char *unit_name);
void logger_defeated(FILE *stream, const char *target);
void logger_battle_end(FILE *stream, const char *result_name);

#endif
