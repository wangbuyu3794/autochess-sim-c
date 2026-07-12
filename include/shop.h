#ifndef AUTOCHESS_SHOP_H
#define AUTOCHESS_SHOP_H

#include "config.h"
#include "player.h"

typedef enum
{
    SHOP_OK = 0,
    SHOP_ERROR_INVALID_ARGUMENT,
    SHOP_ERROR_INVALID_SLOT,
    SHOP_ERROR_EMPTY_SLOT,
    SHOP_ERROR_NOT_ENOUGH_GOLD,
    SHOP_ERROR_BENCH_FULL,
    SHOP_ERROR_NO_TEMPLATE
} ShopResult;

typedef struct
{
    int template_id;
    int is_available;
} ShopSlot;

typedef struct
{
    ShopSlot slots[AUTOCHESS_SHOP_SIZE];
    unsigned int rng_state;
    int is_locked;
} Shop;

int shop_get_cost_probability(int player_level, int cost);
void shop_init(Shop *shop, unsigned int seed);
void shop_refresh(Shop *shop, int player_level);
int shop_prepare_round(Shop *shop, int player_level);
void shop_set_locked(Shop *shop, int is_locked);
ShopResult shop_refresh_for_player(Shop *shop, Player *player);
ShopResult shop_buy_slot(Shop *shop, Player *player, int slot_index, int instance_id);
const char *shop_result_name(ShopResult result);

#endif
