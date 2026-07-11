#ifndef AUTOCHESS_ECONOMY_H
#define AUTOCHESS_ECONOMY_H

#include "player.h"

int economy_calculate_interest(int gold);
int economy_calculate_round_income(int gold);
void economy_apply_round_income(Player *player);
int economy_can_afford(const Player *player, int cost);
int economy_spend_gold(Player *player, int cost);
void economy_add_experience(Player *player, int amount);
int economy_buy_experience(Player *player);

#endif
