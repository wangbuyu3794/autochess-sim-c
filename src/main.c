#include <stdio.h>

#include "ai.h"
#include "battle.h"
#include "economy.h"
#include "player.h"
#include "shop.h"

int main(void)
{
    Player player;
    Player enemy;
    Shop player_shop;
    Shop enemy_shop;
    int enemy_next_instance_id = 101;
    BattleContext context = {0};
    BoardPosition player_front = {6, 3};
    BoardPosition player_left = {7, 2};
    BoardPosition player_back = {7, 4};

    player_init(&player, 1, BATTLE_SIDE_PLAYER);
    player_init(&enemy, 2, BATTLE_SIDE_ENEMY);
    shop_init(&player_shop, 20260711u);
    shop_init(&enemy_shop, 20260712u);

    economy_apply_round_income(&player);
    shop_refresh(&player_shop, player.level);
    shop_buy_slot(&player_shop, &player, 0, 1);
    shop_buy_slot(&player_shop, &player, 1, 2);
    shop_buy_slot(&player_shop, &player, 2, 3);
    player_deploy_from_bench(&player, 0, player_front);
    player_deploy_from_bench(&player, 1, player_left);
    player_deploy_from_bench(&player, 2, player_back);

    ai_run_preparation(&enemy, &enemy_shop, &enemy_next_instance_id);

    player_add_deployed_units_to_battle(&player, &context);
    player_add_deployed_units_to_battle(&enemy, &context);
    battle_apply_trait_summary(&context);

    BattleResult result = battle_run(&context, stdout);

    return result == BATTLE_RESULT_DRAW ? 1 : 0;
}
