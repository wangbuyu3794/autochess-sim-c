#ifndef AUTOCHESS_AI_H
#define AUTOCHESS_AI_H

#include "hero.h"
#include "player.h"
#include "shop.h"
#include "unit.h"

int ai_score_hero_template(const HeroTemplate *hero);
int ai_score_unit(const Unit *unit);
int ai_choose_shop_slot(const Shop *shop, const Player *player);
int ai_buy_best_affordable_unit(Player *player, Shop *shop, int *next_instance_id);
void ai_deploy_best_units(Player *player);
void ai_run_preparation(Player *player, Shop *shop, int *next_instance_id);

#endif
